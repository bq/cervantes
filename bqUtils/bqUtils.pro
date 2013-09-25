CONFIG  += shared
TEMPLATE = lib

TARGET = bqUtils


HEADERS += ./inc/bqUtils.h \
           ./inc/json.h \
           ./inc/BQDeviceInfoUtils.h \
           ./inc/bqLibepubUtils.h

SOURCES += ./src/bqUtils.cpp \
           ./src/json.cpp \
           ./src/BQDeviceInfoUtils.cpp \
           ./src/bqLibepubUtils.cpp

INCLUDEPATH += ./src ./inc

linux-arm-g++ {
        include ( main.conf )
        DESTDIR = lib
} else {
        DESTDIR = x86-lib
}


QMAKE_CLEAN = libbqUtils.so*

