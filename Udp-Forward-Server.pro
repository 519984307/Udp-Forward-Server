TEMPLATE = subdirs

SUBDIRS += \
	server \
	test

QDEP_PROJECT_SUBDIRS += \
	Skycoder42/cryptopp-qdep

server.qdep_depends += Skycoder42/cryptopp-qdep
test.qdep_depends += Skycoder42/cryptopp-qdep

!load(qdep):error("Failed to load qdep feature! Run 'qdep.py prfgen --qmake $$QMAKE_QMAKE' to create it.")
