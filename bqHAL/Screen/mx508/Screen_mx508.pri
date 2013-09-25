base_project=$${PWD}

HEADERS += \
        $${base_project}/inc/einkfb.h \
        $${base_project}/inc/ScreenMx508.h
	

SOURCES += \
        $${base_project}/src/ScreenMx508.cpp

LIBS += $${ROOTFS}/usr/lib/libntx.so
