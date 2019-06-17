#ifndef UDPFWDSERVER_H
#define UDPFWDSERVER_H

#include "udpfwdclient.h"

class Q_UDP_FWD_SERVER_EXPORT UdpFwdServer : public UdpFwdClient
{
	Q_OBJECT

public:
	explicit UdpFwdServer(QObject *parent = nullptr);
	explicit UdpFwdServer(int replyCacheSize, QObject *parent = nullptr);
	~UdpFwdServer();

	bool wasAnnounced() const;

public slots:
	void announce(bool oneTime = false);
	void dismiss(bool clearReplies = false);

private:
	bool _wasAnnounced = false;
};

#endif // UDPFWDSERVER_H
