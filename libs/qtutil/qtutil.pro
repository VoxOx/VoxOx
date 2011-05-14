TEMPLATE = lib
LANGUAGE = C++

CONFIG += staticlib qt stl warn_on console debug

TARGET = qtutil
DESTDIR = bin

INCLUDEPATH += include
INCLUDEPATH += ../util/include

HEADERS += include/EventFilter.h \
	include/MouseEventFilter.h \
	include/PaintEventFilter.h \
	include/EnabledChangeEventFilter.h \
	include/PostEvent.h \
	include/WidgetFactory.h \
	include/Object.h \
	include/QObjectThreadSafe.h \
	include/ResizeEventFilter.h \
	include/NonResizeableWidget.h

SOURCES += src/EventFilter.cpp \
	src/MouseEventFilter.cpp \
	src/PaintEventFilter.cpp \
	src/EnabledChangeEventFilter.cpp \
	src/WidgetFactory.cpp \
	src/Object.cpp \
	src/QObjectThreadSafe.cpp \
	src/ResizeEventFilter.cpp \
	src/NonResizeableWidget.cpp

UI_DIR = bin/ui
MOC_DIR = bin/moc
OBJECTS_DIR = bin/obj

unix {
	QMAKE_CXXFLAGS += -Wall -ansi
	QMAKE_CFLAGS += -Wall -ansi

	LIBS += 
}

win32 {
	DEFINES += WIN32

	LIBS += ../util/util.lib
}
