#include "packageforwarder.h"
#include <QNetworkDatagram>
#include "tunneloutmessage.h"
using namespace UdpFwdProto;

template <typename T>
inline uint qHash(const std::optional<T> &key, uint seed = 0) Q_DECL_NOEXCEPT_EXPR(noexcept(qHash(*key, seed))) {
	return key ? qHash(*key, seed) : seed;
}

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
	bool ok;
	if (socket != -1)
		ok = _socket->setSocketDescriptor(socket, QAbstractSocket::BoundState);
	else
		ok = _socket->bind(QHostAddress::Any, 13119, QAbstractSocket::DontShareAddress | QAbstractSocket::ReuseAddressHint);
	if (!ok) {
		qCritical() << "Failed create UDP-socket with error:"
					<< qUtf8Printable(_socket->errorString());
	}
	return ok;
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
		try {
			MsgHandler handler{this, {datagram.senderAddress(), datagram.senderPort()}};
			std::visit(handler, Message::deserialize<AnnouncePeerMessage, DismissPeerMessage, TunnelInMessage>(datagram.data()));
		} catch (std::bad_variant_access &) {
			qCritical() << "Received invalid message from"
						<< datagram.senderAddress() << "on port"
						<< datagram.senderPort();
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
	qDebug() << Q_FUNC_INFO << peer;
	if (!message.verifySignature()) {
		self->sendError(peer, ErrorMessage::Error::InvalidSignature);
		return;
	}

	const auto fingerprint = fingerPrint(message.key);
	if (message.oneTime)
		self->_replyCache.insert({fingerprint, std::nullopt}, new PeerInfo{std::move(peer), std::move(message.key)});
	else
		self->_peerCache.insert(fingerprint, {std::move(peer), std::move(message.key)});
}

void PackageForwarder::MsgHandler::operator()(DismissPeerMessage &&message)
{
	qDebug() << Q_FUNC_INFO << peer;
	// remove from peer cache
	const auto pIt = std::find_if(self->_peerCache.begin(), self->_peerCache.end(), [this](const PeerInfo &cPeer) {
		return peer == cPeer.first;
	});

	bool verified = false;
	if (pIt != self->_peerCache.end()) {
		if (message.verifySignature(pIt->second)) {
			verified = true;
			self->_peerCache.erase(pIt);
		} else
			return;
	}

	// remove all from reply cache
	if (message.clearReplies) {
		for (const auto &key : self->_replyCache.keys()) {
			if (self->_replyCache.object(key)->first == peer) {
				if (verified || message.verifySignature(pIt->second))
					self->_replyCache.remove(key);
				break;
			}
		}
	}
}

void PackageForwarder::MsgHandler::operator()(TunnelInMessage &&message)
{
	qDebug() << Q_FUNC_INFO << peer;
	if (!message.verifySignature()) {
		self->sendError(peer, ErrorMessage::Error::InvalidSignature);
		return;
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
	} else
		self->sendError(peer, ErrorMessage::Error::InvalidPeer);
}
