#include "udpfwdclient.h"
#include <QNetworkDatagram>
#include <announcepeermessage.h>
#include <tunnelinmessage.h>
using namespace UdpFwdProto;

UdpFwdClient::UdpFwdClient(QObject *parent) :
	UdpFwdClient{10000, parent}
{}

UdpFwdClient::UdpFwdClient(int replyCacheSize, QObject *parent) :
	QObject{parent},
	_socket{new QUdpSocket{this}},
	_replyCache{replyCacheSize}
{
	connect(_socket, qOverload<QAbstractSocket::SocketError>(&QUdpSocket::error),
			this, &UdpFwdClient::socketError);
	connect(_socket, &QUdpSocket::readyRead,
			this, &UdpFwdClient::readyRead);
}

bool UdpFwdClient::setup(PrivateKey key, QHostAddress fwdSvcAddress, quint16 port)
{
	_key = std::move(key);
	_peerHost = std::move(fwdSvcAddress);
	_peerPort = port;

	if (_socket->isOpen())
		_socket->close();
	if (_socket->bind(QHostAddress::Any, 0, QAbstractSocket::DontShareAddress))
		return true;
	else {
		_lastError = _socket->errorString();
		emit error();
		return false;
	}
}

const PrivateKey &UdpFwdClient::key() const
{
	return _key;
}

QString UdpFwdClient::errorString() const
{
	return _lastError;
}

void UdpFwdClient::send(const PublicKey &peer, const QByteArray &data, bool allowReply)
{
	sendImpl(TunnelInMessage::createEncrypted(_rng, peer, data,
											  allowReply ? std::optional<PrivateKey>{_key} : std::nullopt));
}

bool UdpFwdClient::reply(const QByteArray &peer, const QByteArray &data, bool allowReply)
{
	auto pKey = _replyCache.take(peer);
	if (pKey) {
		send(*pKey, data, allowReply);
		delete pKey;
		return true;
	} else
		return false;
}

CryptoPP::RandomNumberGenerator &UdpFwdClient::rng()
{
	return _rng;
}

void UdpFwdClient::socketError()
{
	_lastError = _socket->errorString();
	emit error();
}

void UdpFwdClient::readyRead()
{
	while (_socket->hasPendingDatagrams()) {
		const auto datagram = _socket->receiveDatagram();
		try {
			MsgHandler handler{this, datagram.senderAddress(), static_cast<quint16>(datagram.senderPort())};
			std::visit(handler, Message::deserialize<TunnelOutMessage, ErrorMessage>(datagram.data()));
		} catch (std::bad_variant_access &) {
			qWarning() << "UdpFwdClient: Ignoring invalid message from"
					   << datagram.senderAddress() << "on port"
					   << datagram.senderPort();
		}
	}
}

void UdpFwdClient::MsgHandler::operator()(TunnelOutMessage &&message)
{
	auto data = message.decrypt(self->_rng, self->_key);
	if (message.replyKey) {
		if (message.replyKey->Validate(self->_rng, 3)) {
			const auto fp = fingerPrint(*message.replyKey);
			self->_replyCache.insert(fp, new PublicKey{*message.replyKey});
			emit self->messageReceived(data, fp);
		} else {
			self->_lastError = tr("Received message with invalid reply key - message has been dropped!");
			emit self->error();
		}
	} else
		emit self->messageReceived(data);
}

void UdpFwdClient::MsgHandler::operator()(ErrorMessage &&message)
{
	switch (message.error) {
	case ErrorMessage::Error::InvalidPeer:
		self->_lastError = tr("Unabled to deliver message - unknown peer!");
		break;
	case ErrorMessage::Error::InvalidSignature:
		self->_lastError = tr("Message was rejected by the forward server - invalid signature!");
		break;
	case ErrorMessage::Error::InvalidKey:
		self->_lastError = tr("Message was rejected by the forward server - invalid public key!");
		break;
	case ErrorMessage::Error::Unknown:
		self->_lastError = self->_socket->errorString(); // maybe info is here
		break;
	}
	emit self->error();
}
