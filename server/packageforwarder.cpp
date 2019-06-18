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
		qDebug() << peer << datagram.data();
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
		self->_replyCache.insert({fingerprint, std::nullopt}, new PeerInfo{std::move(peer), std::move(message.key)});
	else
		self->_peerCache.insert(fingerprint, {std::move(peer), std::move(message.key)});
}

void PackageForwarder::MsgHandler::operator()(DismissPeerMessage &&message)
{
	// remove from peer cache
	const auto pIt = std::find_if(self->_peerCache.begin(), self->_peerCache.end(), [this](const PeerInfo &cPeer) {
		return peer == cPeer.first;
	});

	bool verified = false;
	if (pIt != self->_peerCache.end()) {
		if (message.verifySignature(pIt->second)) {
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
			if (self->_replyCache.object(key)->first == peer) {
				if (!verified && !message.verifySignature(pIt->second)) {
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
	if (message.replyKey) {
		if (!message.replyKey->Validate(self->_rng, 3)) {
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
	std::optional<PeerInfo> fwdPeer;
	auto target = self->_replyCache.take({message.peer, peer}); // find a reply slot for this concrete sender
	if (!target)
		target = self->_replyCache.take({message.peer, std::nullopt}); // find a generic reply slot
	if (target) {
		fwdPeer = *target;
		delete target;
	} else
		fwdPeer = self->_peerCache.value(message.peer);

	// send the message and prepare a possible reply
	if (fwdPeer) {
		TunnelOutMessage reply;
		static_cast<PayloadMessageBase&>(reply) = std::move(message); // move assign the payload part
		if (reply.replyKey) {
			const auto fingerprint = fingerPrint(*reply.replyKey);
			self->_replyCache.insert({fingerprint, fwdPeer->first}, new PeerInfo{std::move(peer), *std::move(reply.replyKey)});
		}
		self->_socket->writeDatagram(Message::serialize(reply), fwdPeer->first.first, fwdPeer->first.second);
		qDebug().noquote() << "Forwarded TunnelInMessage from" << peer
						   << "to" << fwdPeer->first
						   << (reply.replyKey ? "(replying allowed)" : "(replying denied)");
	} else {
		qDebug().noquote() << "Received TunnelInMessage from" << peer
						   << "but was not able to find target identity"
						   << message.peer.toHex(':');
		self->sendError(peer, ErrorMessage::Error::InvalidPeer);
	}
}
