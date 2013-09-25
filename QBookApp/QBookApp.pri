HEADERS += ./inc/QBookApp.h \
    ./inc/QBookEmu.h \
    ./inc/QBookScreenSaver.h \
    ./inc/QBookDevel.h \
    ./inc/PowerManager.h \
    ./inc/PowerManagerLock.h \
    ./inc/PowerManagerDefs.h \
    ./inc/ScreenImageManager.h \
    ./inc/Agent.h \
    ./inc/MyAgent.h \
    ./inc/PasswordRequester.h \
    ./inc/SyncHelper.h \
    ./inc/QFwUpgradeOTA.h \
    ./inc/DeviceInfoDialog.h \
    ./inc/QSettingLinkPc.h \
    ./inc/SleeperThread.h \
    ./inc/AfterUpdateWorker.h

SOURCES += ./src/main.cpp \
    ./src/QBookApp.cpp \
    ./src/QBookEmu.cpp \
    ./src/QBookScreenSaver.cpp \
    ./src/QBookDevel.cpp \
    ./src/PowerManager.cpp \
    ./src/PowerManagerLock.cpp \
    ./src/ScreenImageManager.cpp \
    ./src/Agent.cpp \
    ./src/MyAgent.cpp \
    ./src/PasswordRequester.cpp \
    ./src/SyncHelper.cpp \
    ./src/QFwUpgradeOTA.cpp \
    ./src/DeviceInfoDialog.cpp \
    ./src/QSettingLinkPc.cpp \
    ./src/SleeperThread.cpp \
    ./src/AfterUpdateWorker.cpp


FORMS += ./ui/QBookApp.ui \
         ./ui/QBookEmu.ui \
         ./ui/DeviceInfoDialog.ui

isEmpty(HACKERS_EDITION) {
    HEADERS += $$(PRIVATE)/QBookApp/inc/QBookDebugDialog.h
    SOURCES += $$(PRIVATE)/QBookApp/src/QBookDebugDialog.cpp
    FORMS += $$(PRIVATE)/QBookApp/ui/QBookDebugDialog.ui
    INCLUDEPATH += $$(PRIVATE)/QBookApp/inc $$(PRIVATE)/bqClientServices/inc
}

TRANSLATIONS += ./tr/qbookapp_es.ts \
    ./tr/qbookapp_en.ts \
    ./tr/qbookapp_pt.ts \
    ./tr/qbookapp_gl.ts \
    ./tr/qbookapp_ca.ts \
    ./tr/qbookapp_eu.ts

INCLUDEPATH += ./ConnectionManager/lib ./bqUtils/inc ./model/inc

QRC_FILES += QBookApp800.qrc QBookApp1024.qrc
