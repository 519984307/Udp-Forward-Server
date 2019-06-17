#ifndef UDPFWDCLIENT_H
#define UDPFWDCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QUuid>
#include <QCache>

#include <cryptopp/osrng.h>

#include <udpfwdproto.h>
#include <tunneloutmessage.h>
#include <errormessage.h>

class Q_UDP_FWD_SERVER_EXPORT UdpFwdClient : public QObject
{
	Q_OBJECT

public:
	explicit UdpFwdClient(QObject *parent = nullptr);
	explicit UdpFwdClient(int replyCacheSize, QObject *parent = nullptr);

	bool setup(UdpFwdProto::PrivateKey key,
			   QHostAddress fwdSvcAddress,
			   quint16 port = UdpFwdProto::DefaultPort);

	const UdpFwdProto::PrivateKey &key() const;

	QString errorString() const;

public slots:
	void send(const UdpFwdProto::PublicKey &peer, const QByteArray &data, bool allowReply = true);
	bool reply(const QByteArray &peer, const QByteArray &data, bool allowReply = true);

signals:
	void messageReceived(const QByteArray &data, const QByteArray &replyPeer = {});
	void error();

protected:
	CryptoPP::RandomNumberGenerator &rng();
	template <typename TMessage>
	void sendImpl(const TMessage &message);

private slots:
	void socketError();
	void readyRead();

private:
	CryptoPP::AutoSeededRandomPool _rng;
	QUdpSocket *_socket;

	UdpFwdProto::PrivateKey _key;
	QHostAddress _peerHost;
	quint16 _peerPort = 0;

	QCache<QByteArray, UdpFwdProto::PublicKey> _replyCache;
	QString _lastError;

	struct MsgHandler {
		UdpFwdClient *self;
		QHostAddress host;
		quint16 port;

		void operator()(UdpFwdProto::TunnelOutMessage &&message);
		void operator()(UdpFwdProto::ErrorMessage &&message);
	};
};

template<typename TMessage>
void UdpFwdClient::sendImpl(const TMessage &message)
{
	_socket->writeDatagram(UdpFwdProto::Message::serialize(message),
						   _peerHost, _peerPort);
}

#endif // UDPFWDCLIENT_H
