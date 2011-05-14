TEMPLATE = lib
LANGUAGE = C++

CONFIG += qt stl warn_on debug plugin

TARGET = skinstyle
DESTDIR = bin
DLLDESTDIR = tests/demo1/bin/styles

INCLUDEPATH += ../util-ng/include
INCLUDEPATH += ../qtutil/include
INCLUDEPATH += ../global/include

HEADERS += src/SkinStyle.h \
	src/SkinInfoReader.h \
	src/SkinWidget.h \
	src/SkinWindow.h \
	src/SkinButton.h

SOURCES += src/SkinStyle.cpp \
	src/SkinInfoReader.cpp \
	src/SkinWidget.cpp \
	src/SkinWindow.cpp \
	src/SkinButton.cpp

UI_DIR = bin/ui
MOC_DIR = bin/moc
OBJECTS_DIR = bin/obj

win32 {
	DEFINES += WIN32

	LIBS += ../qtutil/bin/qtutil.lib
}

macx {
	LIBS += ../../debug/libqtutil.a ../../debug/libutil-ng.a
}

