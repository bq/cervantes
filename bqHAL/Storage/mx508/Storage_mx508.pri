base_project=$${PWD}

# Interface
HEADERS += $${base_project}/inc/StorageMx508.h

SOURCES += $${base_project}/src/StorageMx508.cpp

LIBS += $${ROOTFS}/usr/lib/libudev.so

