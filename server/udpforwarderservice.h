#ifndef UDPFORWARDERSERVICE_H
#define UDPFORWARDERSERVICE_H

#include <QtService/Service>
#include <QSettings>

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
	static const QString KeyCacheSize;
	static const QString KeyBindAddress;
	static const QString KeyBindPort;

	QSettings *_settings = nullptr;
	PackageForwarder *_forwarder = nullptr;
};

#undef qService
#define qService static_cast<UdpForwarderService*>(QtService::Service::instance())

#endif // UDPFORWARDERSERVICE_H
