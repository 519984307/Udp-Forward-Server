QT += network

HEADERS += \
	$$PWD/udpfwdclient.h \
	$$PWD/udpfwdserver.h

SOURCES += \
	$$PWD/udpfwdclient.cpp \
	$$PWD/udpfwdserver.cpp

INCLUDEPATH += $$PWD

QDEP_DEPENDS += \
	Skycoder42/Udp-Forward-Server@1.1.0/protocol/protocol.pri

QDEP_PACKAGE_EXPORTS += Q_UDP_FWD_SERVER_EXPORT
!qdep_build: DEFINES += "Q_UDP_FWD_SERVER_EXPORT="
