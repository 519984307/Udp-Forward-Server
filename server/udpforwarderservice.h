#ifndef UDPFORWARDERSERVICE_H
#define UDPFORWARDERSERVICE_H

#include <QtService/Service>

#include "packageforwarder.h"

class UdpForwarderService : public QtService::Service
{
	Q_OBJECT

public:
	explicit UdpForwarderService(int &argc, char **argv);

protected:
	CommandResult onStart() override;
	CommandResult onStop(int &exitCode) override;

private:
	PackageForwarder *_forwarder = nullptr;
};

#undef qService
#define qService static_cast<UdpForwarderService*>(QtService::Service::instance())

#endif // UDPFORWARDERSERVICE_H
