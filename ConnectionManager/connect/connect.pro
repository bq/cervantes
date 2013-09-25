
QT += core dbus network

TARGET = Connect
CONFIG += debug

include ( ../main.conf )

TEMPLATE = app

SOURCES += Connect.cpp main.cpp
HEADERS += Connect.h

INCLUDEPATH += ../lib

linux-arm-g++ {
        LIBS += -L../lib/lib/ -lConnectionManager -lssl -lcrypto -lz
	DESTDIR = bin
} else {
        LIBS += -L../lib/x86-lib/ -lConnectionManager
	DESTDIR = x86-bin
}


QMAKE_CLEAN = Connect
