#include "udpfwdserver.h"
#include "announcepeermessage.h"
#include "dismisspeermessage.h"
using namespace UdpFwdProto;

UdpFwdServer::UdpFwdServer(QObject *parent) :
	UdpFwdClient{parent}
{}

UdpFwdServer::UdpFwdServer(int replyCacheSize, QObject *parent) :
	UdpFwdClient{replyCacheSize, parent}
{}

UdpFwdServer::~UdpFwdServer()
{
	if (_wasAnnounced)
		dismiss(true);
}

bool UdpFwdServer::wasAnnounced() const
{
	return _wasAnnounced;
}

void UdpFwdServer::announce(bool oneTime)
{
	sendImpl(AnnouncePeerMessage::createSigned(rng(), key(), oneTime));
	_wasAnnounced = true;
}

void UdpFwdServer::dismiss(bool clearReplies)
{
	sendImpl(DismissPeerMessage::createSigned(rng(), key(), clearReplies));
	_wasAnnounced = false;
}
