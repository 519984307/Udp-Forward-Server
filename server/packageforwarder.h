#ifndef PACKAGEFORWARDER_H
#define PACKAGEFORWARDER_H

#include <QObject>
#include <QUdpSocket>
#include <QCache>

#include <cryptopp/osrng.h>

#include <announcepeermessage.h>
#include <dismisspeermessage.h>
#include <tunnelinmessage.h>
#include <errormessage.h>

class PackageForwarder : public QObject
{
	Q_OBJECT

public:
	explicit PackageForwarder(int cacheSize, QObject *parent = nullptr);

	bool create(int socket);
	bool create(const QHostAddress &address, quint16 port);

public slots:

signals:

private slots:
	void socketError();
	void readyRead();

private:
	using Peer = std::pair<QHostAddress, quint16>; // (host, port)
	using PeerInfo = std::tuple<Peer, UdpFwdProto::PublicKey, quint16>; // (peer, pubKey, limit)
	using ReplyId = std::pair<QByteArray, std::optional<Peer>>; // (fingerprint, sender)

	QUdpSocket *_socket;
	CryptoPP::AutoSeededRandomPool _rng;
	QHash<QByteArray, PeerInfo> _peerCache;
	QCache<ReplyId, PeerInfo> _replyCache;

	struct MsgHandler {
		PackageForwarder *self;
		Peer peer;

		void operator()(UdpFwdProto::AnnouncePeerMessage &&message);
		void operator()(UdpFwdProto::DismissPeerMessage &&message);
		void operator()(UdpFwdProto::TunnelInMessage &&message);
	};

	void sendError(const Peer &peer, UdpFwdProto::ErrorMessage::Error error);
};

template <typename T>
inline uint qHash(const std::optional<T> &key, uint seed = 0) Q_DECL_NOEXCEPT_EXPR(noexcept(qHash(*key, seed))) {
	return key ? qHash(*key, seed) : seed;
}

template<>
inline QDebug operator<<(QDebug debug, const std::pair<QHostAddress, quint16> &peer);

#endif // PACKAGEFORWARDER_H
