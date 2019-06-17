TEMPLATE = app

CONFIG += console warning_clean exceptions c++17

SOURCES += \
	main.cpp

include(../client/client.pri)

QDEP_PROJECT_ROOT = ..
QDEP_PROJECT_LINK_DEPENDS += Skycoder42/cryptopp-qdep

QDEP_DEPENDS += \
	Skycoder42/Udp-Forward-Server@0.1.1/client/client.pri

!load(qdep):error("Failed to load qdep feature! Run 'qdep.py prfgen --qmake $$QMAKE_QMAKE' to create it.")
