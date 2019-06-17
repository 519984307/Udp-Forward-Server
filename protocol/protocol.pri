HEADERS += \
	$$PWD/announcepeermessage.h \
	$$PWD/dismisspeermessage.h \
	$$PWD/errormessage.h \
	$$PWD/message.h \
	$$PWD/message_p.h \
	$$PWD/payloadmessagebase.h \
	$$PWD/tunnelinmessage.h \
	$$PWD/tunneloutmessage.h \
	$$PWD/udpfwdproto.h \
	$$PWD/udpfwdproto_p.h

SOURCES += \
	$$PWD/announcepeermessage.cpp \
	$$PWD/dismisspeermessage.cpp \
	$$PWD/errormessage.cpp \
	$$PWD/message.cpp \
	$$PWD/payloadmessagebase.cpp \
	$$PWD/tunnelinmessage.cpp \
	$$PWD/tunneloutmessage.cpp \
	$$PWD/udpfwdproto.cpp

INCLUDEPATH += $$PWD

QDEP_DEPENDS *= \
	Skycoder42/CryptoQQ

QDEP_PACKAGE_EXPORTS += Q_UDP_FWD_PROTOCOL_EXPORT
!qdep_build: DEFINES += "Q_UDP_FWD_PROTOCOL_EXPORT="

QDEP_HOOK_FNS += qdep_udp_fwd_protocol_init
