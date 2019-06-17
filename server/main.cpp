#include "udpforwarderservice.h"

int main(int argc, char *argv[])
{
	UdpForwarderService svc{argc, argv};
	return svc.exec();
}
