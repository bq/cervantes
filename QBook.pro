# Set root directory
Q_ROOT = $${_PRO_FILE_PWD_}/..

ROOTFS = $$(ROOTFS)
EXTRA_LIBS = $$(EXTRA_LIBS)
HACKERS_EDITION = $$(HACKERS_EDITION)
DISABLE_ADOBE_SDK = $$(DISABLE_ADOBE_SDK)

Q_PROJECTS = \
    QBookCore \
    QDocView \
    bqSetting \
    QBookApp \
    bq \
    bqHAL \
    bqViewer \
    bqUi \
    bqLibrary \
    bqHome \
    bqSearch \
    bqBrowser \
    metadata-extractor \
    bqPublicServices

isEmpty(HACKERS_EDITION) {
    Q_PROJECTS += bqWebStore bqWizard
}


# Set platform
message($(PLATFORM) platform)
CONFIG += $$(PLATFORM)

linux-arm-g++ { # Cross compilation for arm
    message(ARM compilation)
    INCLUDEPATH += $${ROOTFS}/usr/include/
    CONFIG(mx508) {
        LIBS += -ljpeg -lexpat -lfreetype -lz -lssl -lcrypto -lrt -lntx 
    } else {
    	LIBS += $${ROOTFS}/usr/lib/libssl.a $${ROOTFS}/usr/lib/libcrypto.a -L$${ROOTFS}/usr/lib/
    }

} else { # emulation
    message(Development compilation)
    INCLUDEPATH += /usr/include $${ROOTFS}/usr/include/
    QT_DEV_PATH = $$(QT_DEV_PATH)
    LIBS += -L/usr/lib -lssl -lcrypto $${EXTRA_LIBS} -lX11
    # This is needed for fonts in the emulator
    DEFINES += "ROOTFS=\\\"$${ROOTFS}\\\""
}

message (--- QBookApp Project ---)
DEFINES += $$(DEFINES)

# TEST OPTIONS
BATTERY_TEST = $$(BATTERY_TEST)
SAMPLE_PERIOD = $$(SAMPLE_PERIOD)
START_POINT = $$(START_POINT)
!isEmpty(BATTERY_TEST) {
    isEmpty(SAMPLE_PERIOD){
      SAMPLE_PERIOD = 21600 # 6 hours
    }
    isEmpty(START_POINT){
      START_POINT = 80 # Test start at 80% by default, until then device awake
    }
    message(Test build type $$BATTERY_TEST with sample period $$SAMPLE_PERIOD and start point $$START_POINT)
    QMAKE_CXXFLAGS += -DBATTERY_TEST=$$BATTERY_TEST -DSAMPLE_PERIOD=$$SAMPLE_PERIOD -DSTART_POINT=$$START_POINT
}

#SHOWCASE BUILD, just to keep the device in a showcase with light on
SHOWCASE = $$(SHOWCASE)
!isEmpty(SHOWCASE){
    message(Special build for showcase device)
    QMAKE_CXXFLAGS += -DSHOWCASE
}

# Set application properties
TEMPLATE = app
TARGET = QBookApp
QT += network webkit gui sql xml dbus
#QTPLUGIN += qjpeg qgif qico qmng qtiff


linux-arm-g++ {
  DEFINES += QT_STATICPLUGIN
  DEFINES -= QT_NO_OPENSSL
} else {
  DEFINES += EMULATOR
}

QT_DIR = $$(QTDIR)

# Include common properties
include(QBook.conf)

# Set unix properties
CONFIG += debug
CONFIG -= release
QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-psabi 
!linux-arm-g++ {
	# Remove optimization for emulator builds
        QMAKE_CXXFLAGS_RELEASE -=-O2
}

CONFIG += $$(CPP_OPTIM)
CONFIG(CPP_OPT_O0){
  # Remove optimization for debugging
  message(C++ optimization disabled)
  QMAKE_CXXFLAGS_RELEASE -=-O2
}

# Add debug info to Release build (then we will strip it)
QMAKE_CXXFLAGS_RELEASE +=-g
QMAKE_CFLAGS_RELEASE +=-g
# FIXME: currently we are striping ONLY debug symbols. We could save 2M striping ALL symbols. We need to check if that can hurts us somehow
linux-arm-g++ { # Cross compilation for arm
	# We need to do this instead of proper CONFIG+=separate_debug_info becuase qmake
        # expands QMAKE_INSTALL_SEPARATE_DEBUG_INFO macro to usr basename from the rootfs, and that is an arm binary
	APPFILE=`/usr/bin/basename $(TARGET)`
        QMAKE_POST_LINK=cd bin; /usr/bin/arm-linux-gnueabi-objcopy --only-keep-debug $${APPFILE} $${APPFILE}.debug && /usr/bin/arm-linux-gnueabi-objcopy --strip-debug $${APPFILE} && /usr/bin/arm-linux-gnueabi-objcopy --add-gnu-debuglink=$${APPFILE}.debug $${APPFILE} && chmod -x $${APPFILE}.debug
} else {
	APPFILE=`/usr/bin/basename $(TARGET)`
        QMAKE_POST_LINK=cd x86-bin; objcopy --only-keep-debug $${APPFILE} $${APPFILE}.debug && objcopy --strip-debug $${APPFILE} && objcopy --add-gnu-debuglink=$${APPFILE}.debug $${APPFILE} && chmod -x $${APPFILE}.debug
}


QMAKE_LFLAGS_APP += -Wl,-Map=$${DESTDIR}/$${TARGET}.map

# Remove .map at cleaning
QMAKE_DISTCLEAN += $${DESTDIR}/$${TARGET}.map 

# Remove generated files at cleaning
QMAKE_DISTCLEAN += $${DESTDIR}/langs/*
QMAKE_DISTCLEAN += $${DESTDIR}/*
QMAKE_DISTCLEAN += $${OBJECTS_DIR}/*
QMAKE_DISTCLEAN += $${UI_DIR}/*
QMAKE_DISTCLEAN += $${MOC_DIR}/*

# Add project subdirs
INCLUDEPATH += ./bq
DEPENDPATH += $${Q_PROJECTS} ./bq
VPATH += $${Q_PROJECTS}

# Include projects
for(prj, Q_PROJECTS) {
    INCLUDEPATH += $${prj}/inc
    DEPENDPATH += $${prj}/inc
    INCLUDEPATH += $${prj}/model
    exists($${prj}/$${prj}.prj): include($${prj}/$${prj}.prj)
    else: include($${prj}/$${prj}.pri)

    exists($${prj}/tr/*.ts): TR_FILES += $${prj}/tr/*.ts
}


# Fix some uglyness of qt paths
linux-arm-g++ { # Cross compilation for arm
    LIBS += -L$${ROOTFS}/usr/plugins/imageformats -lqjpeg -lqgif -lqico -lqmng -lqtiff
    isEmpty(HACKERS_EDITION){
        LIBS += -LConnectionManager/lib/lib -lConnectionManager -L$(PRIVATE)/bqClientServices/lib -lServices -LbqUtils/lib -lbqUtils -Lmodel/lib -lbqModelLibrary
    } else {
        LIBS += -LConnectionManager/lib/lib -lConnectionManager -LbqUtils/lib -lbqUtils -Lmodel/lib -lbqModelLibrary
    }
} else {
   isEmpty(HACKERS_EDITION) {
       LIBS += -LConnectionManager/lib/x86-lib -lConnectionManager -L$(PRIVATE)/bqClientServices/x86-lib -lServices -LbqUtils/x86-lib -lbqUtils -Lmodel/x86-lib -lbqModelLibrary
   } else {
       LIBS += -LConnectionManager/lib/x86-lib -lConnectionManager -LbqUtils/x86-lib -lbqUtils -Lmodel/x86-lib -lbqModelLibrary
   }
}

FAKE_WIFI = $$(FAKE_WIFI)
!isEmpty(FAKE_WIFI) {
	DEFINES += "FAKE_WIFI=\\\"1\\\""
}

!isEmpty(HACKERS_EDITION) {
	DEFINES += "HACKERS_EDITION=\\\"1\\\""
}

!isEmpty(DISABLE_ADOBE_SDK) {
        DEFINES += "DISABLE_ADOBE_SDK=\\\"1\\\""
}



QRC_FILES += res/common800.qrc res/common1024.qrc res/common1448.qrc

OTHER_FILES += \
    res/common_styles_generic.qss \
    res/800/common_styles.qss \
    res/1024/common_styles.qss \
    res/1448/common_styles.qss \
    QBookApp.files \
    build.sh

