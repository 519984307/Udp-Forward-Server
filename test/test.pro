TEMPLATE = app

VERSION = 1.0.6

CONFIG += console warning_clean exceptions c++17

SOURCES += \
	main.cpp

QDEP_PROJECT_ROOT = ..
QDEP_PROJECT_LINK_DEPENDS += Skycoder42/cryptopp-qdep

include(../protocol/protocol.pri)
include(../client/client.pri)

QDEP_DEPENDS = Skycoder42/CryptoQQ

!load(qdep):error("Failed to load qdep feature! Run 'qdep.py prfgen --qmake $$QMAKE_QMAKE' to create it.")
