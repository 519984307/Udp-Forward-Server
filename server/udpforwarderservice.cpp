#include "udpforwarderservice.h"

const QString UdpForwarderService::KeyCacheSize {QStringLiteral("cache/size")};
const QString UdpForwarderService::KeyBindAddress {QStringLiteral("bind/address")};
const QString UdpForwarderService::KeyBindPort {QStringLiteral("bind/port")};

UdpForwarderService::UdpForwarderService(int &argc, char **argv) :
	Service{argc, argv}
{
	QCoreApplication::setApplicationName(QStringLiteral(TARGET));
	QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));
	QCoreApplication::setOrganizationName(QStringLiteral(COMPANY));
	QCoreApplication::setOrganizationDomain(QStringLiteral(BUNDLE));
}

QtService::Service::CommandResult UdpForwarderService::onStart()
{
	_settings = new QSettings{this};
	_settings->setFallbacksEnabled(true);

	_forwarder = new PackageForwarder{_settings->value(KeyCacheSize, 10000).toInt(), this};
	auto socket = getSocket();
	bool ok;
	if (socket != -1)
		ok = _forwarder->create(socket);
	else {
		ok = _forwarder->create(_settings->contains(KeyBindAddress) ?
									QHostAddress{_settings->value(KeyBindAddress).toString()} :
									QHostAddress{QHostAddress::Any},
								static_cast<quint16>(_settings->value(KeyBindPort, UdpFwdProto::DefaultPort).toUInt()));
	}
	if (!ok)
		return CommandResult::Failed;

	return CommandResult::Completed;
}

QtService::Service::CommandResult UdpForwarderService::onStop(int &exitCode)
{
	exitCode = EXIT_SUCCESS;
	return CommandResult::Completed;
}
