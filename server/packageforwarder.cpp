#include "packageforwarder.h"
#include <QNetworkDatagram>
#include "tunneloutmessage.h"
using namespace UdpFwdProto;

PackageForwarder::PackageForwarder(int cacheSize, QObject *parent) :
	QObject{parent},
	_socket{new QUdpSocket{this}},
	_replyCache{cacheSize}
{
	connect(_socket, qOverload<QAbstractSocket::SocketError>(&QUdpSocket::error),
			this, &PackageForwarder::socketError);
	connect(_socket, &QUdpSocket::readyRead,
			this, &PackageForwarder::readyRead);
}

bool PackageForwarder::create(int socket)
{
	if (!_socket->setSocketDescriptor(socket, QAbstractSocket::BoundState)) {
		qCritical() << "Failed to attach UDP-socket with error:"
					<< qUtf8Printable(_socket->errorString());
		return false;
	} else
		return true;
}

bool PackageForwarder::create(const QHostAddress &address, quint16 port)
{
	if (!_socket->bind(address, port, QAbstractSocket::DontShareAddress | QAbstractSocket::ReuseAddressHint)) {
		qCritical() << "Failed to create UDP-socket with error:"
					<< qUtf8Printable(_socket->errorString());
		return false;
	} else
		return true;
}

void PackageForwarder::socketError()
{
	qCritical() << "Network operation failed with error:"
				<< qUtf8Printable(_socket->errorString());
}

void PackageForwarder::readyRead()
{
	while (_socket->hasPendingDatagrams()) {
		const auto datagram = _socket->receiveDatagram();
		const Peer peer {datagram.senderAddress(), datagram.senderPort()};
		try {
			MsgHandler handler{this, peer};
			std::visit(handler, Message::deserialize<AnnouncePeerMessage, DismissPeerMessage, TunnelInMessage>(datagram.data()));
		} catch (std::bad_variant_access &) {
			qWarning() << "Received invalid datagram from" << peer;
		}
	}
}

void PackageForwarder::sendError(const Peer &peer, ErrorMessage::Error error)
{
	ErrorMessage reply;
	reply.error = error;
	_socket->writeDatagram(Message::serialize(reply), peer.first, peer.second);
}

void PackageForwarder::MsgHandler::operator ()(AnnouncePeerMessage &&message)
{
	if (!message.key.Validate(self->_rng, 3)) {
		qWarning() << "Received AnnouncePeerMessage from" << peer
				   << "with invalid key";
		self->sendError(peer, ErrorMessage::Error::InvalidKey);
		return;
	}
	if (!message.verifySignature()) {
		qWarning() << "Received AnnouncePeerMessage from" << peer
				   << "with invalid signature";
		self->sendError(peer, ErrorMessage::Error::InvalidSignature);
		return;
	}

	const auto fingerprint = fingerPrint(message.key);
	qDebug().noquote() << "Peer" << peer
					   << "announced itself for" << fingerprint.toHex(':')
					   << (message.oneTime ? "(as one-time)" : "");
	if (message.oneTime)
		self->_replyCache.insert({fingerprint, std::nullopt}, new PeerInfo{std::move(peer), std::move(message.key), 0});
	else
		self->_peerCache.insert(fingerprint, {std::move(peer), std::move(message.key), 1});
}

void PackageForwarder::MsgHandler::operator()(DismissPeerMessage &&message)
{
	// remove from peer cache
	const auto pIt = std::find_if(self->_peerCache.begin(), self->_peerCache.end(), [this](const PeerInfo &cPeer) {
		return peer == std::get<0>(cPeer);
	});

	bool verified = false;
	if (pIt != self->_peerCache.end()) {
		if (message.verifySignature(std::get<1>(*pIt))) {
			verified = true;
			self->_peerCache.erase(pIt);
		} else {
			qWarning() << "Received DismissPeerMessage from" << peer
					   << "with invalid signature";
			return;
		}
	}

	// remove all from reply cache
	if (message.clearReplies) {
		for (const auto &key : self->_replyCache.keys()) {
			auto peerInfo = self->_replyCache.object(key);
			if (std::get<0>(*peerInfo) == peer) {
				if (!verified && !message.verifySignature(std::get<1>(*peerInfo))) {
					qWarning() << "Received DismissPeerMessage from" << peer
							   << "with invalid signature";
					return;
				}
				self->_replyCache.remove(key);
				break;
			}
		}
	}

	qDebug().noquote() << "Peer" << peer << "was dismissed";
}

void PackageForwarder::MsgHandler::operator()(TunnelInMessage &&message)
{
	if (message.replyInfo) {
		if (!message.replyInfo.key.Validate(self->_rng, 3)) {
			qWarning() << "Received TunnelInMessage from" << peer
					   << "with invalid replyKey";
			self->sendError(peer, ErrorMessage::Error::InvalidKey);
			return;
		}
		if (!message.verifySignature()) {
			qWarning() << "Received TunnelInMessage from" << peer
					   << "with invalid signature";
			self->sendError(peer, ErrorMessage::Error::InvalidSignature);
			return;
		}
	}

	// find the target to forward the message to
	// first, search for an explicit reply
	std::optional<ReplyId> cleanupId {{message.peer, peer}};
	auto target = self->_replyCache.object(*cleanupId); // find a reply slot for this concrete sender
	// not found -> search for an implicit reply
	if (!target) {
		cleanupId = {message.peer, std::nullopt};
		target = self->_replyCache.object(*cleanupId);
	}
	// still not found -> search for an announced peer
	if (!target && self->_peerCache.contains(message.peer)) {
		cleanupId = std::nullopt;
		target = &self->_peerCache[message.peer];
	}

	// send the message and prepare a possible reply
	if (target) {
		TunnelOutMessage reply;
		static_cast<PayloadMessageBase&>(reply) = std::move(message); // move assign the payload part
		if (reply.replyInfo) {
			const auto fingerprint = fingerPrint(reply.replyInfo.key);
			self->_replyCache.insert({fingerprint, std::get<0>(*target)}, new PeerInfo {
										 std::move(peer),
										 std::move(reply.replyInfo.key),
										 reply.replyInfo.limit
									 });
		}
		self->_socket->writeDatagram(Message::serialize(reply), std::get<0>(*target).first, std::get<0>(*target).second);
		qDebug().noquote() << "Forwarded TunnelInMessage from" << peer
						   << "to" << std::get<0>(*target)
						   << (reply.replyInfo ? "(replying allowed)" : "(replying denied)");
		if (cleanupId && --std::get<2>(*target) == 0)
			self->_replyCache.remove(*cleanupId);
	} else {
		qDebug().noquote() << "Received TunnelInMessage from" << peer
						   << "but was not able to find target identity"
						   << message.peer.toHex(':');
		self->sendError(peer, ErrorMessage::Error::InvalidPeer);
	}
}

template<>
inline QDebug operator<<(QDebug debug, const std::pair<QHostAddress, quint16> &peer) {
	QDebugStateSaver state{debug};
	debug.noquote().nospace() << peer.first.toString() << ":" << peer.second;
	return debug;
}
