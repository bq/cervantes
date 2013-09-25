
QT += network
CONFIG += debug

include ( ../main.conf )

TARGET = test-services

TEMPLATE = app

SOURCES += main.cpp  Test.cpp
HEADERS += Test.h

INCLUDEPATH += ../lib

LIBS += -lreadline -lncurses

linux-arm-g++ {
        DESTDIR = bin
} else {
        DESTDIR = x86-bin
}


QMAKE_CLEAN = test-services
