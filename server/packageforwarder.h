#ifndef PACKAGEFORWARDER_H
#define PACKAGEFORWARDER_H

#include <QObject>
#include <QUdpSocket>
#include <QCache>

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
	using PeerInfo = std::pair<Peer, UdpFwdProto::PublicKey>; // (peer, pubKey)
	using ReplyInfo = std::pair<QByteArray, std::optional<Peer>>; // (fingerprint, sender)

	friend inline QDebug operator<<(QDebug debug, const Peer &peer) {
		QDebugStateSaver state{debug};
		debug.noquote().nospace() << peer.first.toString() << ":" << peer.second;
		return debug;
	}

	QUdpSocket *_socket;
	QHash<QByteArray, PeerInfo> _peerCache;
	QCache<ReplyInfo, PeerInfo> _replyCache;

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

#endif // PACKAGEFORWARDER_H
