base_project=$${PWD}

SOURCES += $${base_project}/src/StoragePartition.cpp  \
        $${base_project}/src/Storage.cpp \
	$${base_project}/src/StorageDevice.cpp

HEADERS += $${base_project}/inc/Storage.h \
	$${base_project}/inc/StoragePartition.h \
        $${base_project}/inc/StorageDevice.h

INCLUDEPATH += $${base_project}/$(PLATFORM)/inc
DEPENDPATH += $${base_project}/$(PLATFORM)/inc

include($${base_project}/$(PLATFORM)/Storage_$(PLATFORM).pri)
