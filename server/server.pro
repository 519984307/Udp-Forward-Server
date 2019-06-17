TEMPLATE = app

QT = core network service
CONFIG += console warning_clean exceptions c++17
CONFIG -= app_bundle
DEFINES += QT_DEPRECATED_WARNINGS QT_ASCII_CAST_WARNINGS QT_USE_QSTRINGBUILDER

TARGET_BASE = udpforwarder
win32: TARGET = $${TARGET_BASE}svc
else:unix: TARGET = $${TARGET_BASE}d
unix: TARGET = $$TARGET_BASE

VERSION = 1.0.0

QMAKE_TARGET_PRODUCT = "Udp-Forward-Server"
QMAKE_TARGET_DESCRIPTION = "The Udp-Forward-Server Service"
QMAKE_TARGET_COMPANY = "Skycoder42"
QMAKE_TARGET_COPYRIGHT = "Felix Barz"

DEFINES += "TARGET_BASE=\\\"$$TARGET_BASE\\\""
DEFINES += "TARGET=\\\"$$TARGET\\\""
DEFINES += "VERSION=\\\"$$VERSION\\\""
DEFINES += "COMPANY=\"\\\"$$QMAKE_TARGET_COMPANY\\\"\""
DEFINES += "BUNDLE=\"\\\"de.skycoder42\\\"\""

HEADERS += \
	packageforwarder.h \
	udpforwarderservice.h

SOURCES += \
	main.cpp \
	packageforwarder.cpp \
	udpforwarderservice.cpp

!isEmpty(UDP_FWD_SVC_CRYPTOPP_PKG):packagesExist($$UDP_FWD_SVC_CRYPTOPP_PKG) {
	CONFIG += link_pkgconfig
	PKGCONFIG += $$UDP_FWD_SVC_CRYPTOPP_PKG
} else {
	isEmpty(UDP_FWD_SVC_QDEP_PROJECT_ROOT): UDP_FWD_SVC_QDEP_PROJECT_ROOT = ..
	QDEP_PROJECT_ROOT = $$UDP_FWD_SVC_QDEP_PROJECT_ROOT
	QDEP_PROJECT_LINK_DEPENDS += Skycoder42/cryptopp-qdep
}

QDEP_DEPENDS += \
	Skycoder42/Udp-Forward-Server@0.1.1/protocol/protocol.pri

isEmpty(UDP_FWD_SVC_INSTALL_BINS): UDP_FWD_SVC_INSTALL_BINS = $$[QT_INSTALL_BINS]
isEmpty(UDP_FWD_SVC_INSTALL_LIBS): UDP_FWD_SVC_INSTALL_LIBS = $$[QT_INSTALL_LIBS]
isEmpty(UDP_FWD_SVC_INSTALL_SYSTEMD_UNITS): UDP_FWD_SVC_INSTALL_SYSTEMD_UNITS = $$UDP_FWD_SVC_INSTALL_LIBS/systemd/user/
isEmpty(UDP_FWD_SVC_INSTALL_LAUNCHD_AGENTS): UDP_FWD_SVC_INSTALL_LAUNCHD_AGENTS = /Library/LaunchAgents

target.path = $$UDP_FWD_SVC_INSTALL_BINS
INSTALLS += target

linux:!android {
	# install targets for systemd service files
	QMAKE_SUBSTITUTES += udpforwarderservice.service.in

	install_svcconf.files += $$shadowed(udpforwarderservice.service)
	install_svcconf.files += udpforwarderservice.socket
	install_svcconf.CONFIG += no_check_exist
	install_svcconf.path = $$UDP_FWD_SVC_INSTALL_SYSTEMD_UNITS
	INSTALLS += install_svcconf
}

win32 {
	# install targets for windows service files
	QMAKE_SUBSTITUTES += udpforwarderservice-install.bat.in

	install_svcconf.files += $$shadowed(udpforwarderservice-install.bat)
	install_svcconf.CONFIG += no_check_exist
	install_svcconf.path = $$UDP_FWD_SVC_INSTALL_BINS
	INSTALLS += install_svcconf
}

macos {
	# install targets for launchd service files
	QMAKE_SUBSTITUTES += de.skycoder42.udpforwarderservice.plist.in

	install_svcconf.files += $$shadowed(de.skycoder42.udpforwarderservice.plist)
	install_svcconf.CONFIG += no_check_exist
	install_svcconf.path = $$UDP_FWD_SVC_INSTALL_LAUNCHD_AGENTS
	INSTALLS += install_svcconf
}

DISTFILES += $$QMAKE_SUBSTITUTES

!load(qdep):error("Failed to load qdep feature! Run 'qdep.py prfgen --qmake $$QMAKE_QMAKE' to create it.")
