include ( ../main.conf )


QT += network dbus
CONFIG += debug


TEMPLATE = lib

TARGET = ConnectionManager

INCLUDEPATH += ../../bqHAL/Wifi/inc
SOURCES = Clock.cpp ConnectionManager.cpp Manager.cpp Network.cpp Service.cpp TechnologiesList.cpp Technology.cpp NetworkService.cpp WpaInterface.cpp Supplicant.cpp HiddenQuery.cpp
HEADERS = Clock.h ConnectionManager.h Manager.h Network.h Service.h TechnologiesList.h Technology.h NetworkService.h WpaInterface.h Supplicant.h HiddenQuery.h

linux-arm-g++ {
        DESTDIR = lib
} else {
        DESTDIR = x86-lib
    	FAKE_WIFI = $$(FAKE_WIFI)
    	!isEmpty(FAKE_WIFI) {
        	DEFINES += "FAKE_WIFI=\\\"1\\\""
    	}

}


QMAKE_CLEAN = libConnectionManager.so*

