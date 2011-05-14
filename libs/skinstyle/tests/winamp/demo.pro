TEMPLATE = app
LANGUAGE = C++

CONFIG += qt stl designer warn_on debug console

TARGET = demo
DESTDIR = bin

INCLUDEPATH += ../../../qtutil/include
INCLUDEPATH += ../../../util-ng/include

HEADERS += SkinDemo.h

SOURCES += SkinDemo.cpp \
	main.cpp

UI_DIR = bin/ui
MOC_DIR = bin/moc
OBJECTS_DIR = bin/obj

win32 {
	DEFINES += WIN32

	LIBS += ../../../../debug/qtutil.lib
}

macx {
	LIBS += ../../../../debug/libqtutil.a
}

