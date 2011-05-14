TEMPLATE = app
LANGUAGE = C++

CONFIG += warn_on debug console

TARGET = playsound

INCLUDEPATH += ../../../util

HEADERS += PlaySoundFile.h SoundFile.h
SOURCES += PlaySoundFile.cpp SoundFile.cpp main.cpp

UI_DIR = tmp/ui
MOC_DIR = tmp/moc
OBJECTS_DIR = tmp/obj
