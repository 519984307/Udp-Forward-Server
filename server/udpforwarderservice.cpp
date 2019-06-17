#include "udpforwarderservice.h"

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
	_forwarder = new PackageForwarder{10000, this}; // TODO read from settings
	if (!_forwarder->create(getSocket()))
		return CommandResult::Failed;

	return CommandResult::Completed;
}

QtService::Service::CommandResult UdpForwarderService::onStop(int &exitCode)
{
	exitCode = EXIT_SUCCESS;
	return CommandResult::Completed;
}
