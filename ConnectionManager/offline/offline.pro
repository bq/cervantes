
QT += core dbus network

TARGET = Offline
CONFIG += debug

include ( ../main.conf )

TEMPLATE = app

SOURCES += Offline.cpp main.cpp
HEADERS += Offline.h

INCLUDEPATH += ../lib

linux-arm-g++ {
        LIBS += -L../lib/lib/ -lConnectionManager -lssl -lcrypto -lz
	DESTDIR = bin
} else {
        LIBS += -L../lib/x86-lib/ -lConnectionManager
	DESTDIR = x86-bin
}


QMAKE_CLEAN = Offline
