TEMPLATE = app
LANGUAGE = C++
CONFIG += qt warn_on release console

TARGET = sound

INCLUDEPATH += .. ../../util
SOURCES += main.cpp

unix {
	QMAKE_CXXFLAGS += -Wall -ansi
	QMAKE_CFLAGS += -Wall -ansi

	LIBS += -L../lib -lsound \
		-L../../util/lib -lutil
}

win32 {
	LIBS += ../lib/sound.lib \
		../../util/lib/util.lib
}

UI_DIR = tmp/ui
MOC_DIR = tmp/moc
OBJECTS_DIR = tmp/obj
