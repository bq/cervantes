base_project=$${PWD}

# Interface
HEADERS += $${base_project}/inc/ADConverter.h \
           $${base_project}/inc/DeviceInfo.h

SOURCES += $${base_project}/src/ADConverter.cpp \
           $${base_project}/src/DeviceInfo.cpp

INCLUDEPATH += $${base_project}/$(PLATFORM)/inc
DEPENDPATH += $${base_project}/$(PLATFORM)/inc

include($${base_project}/$(PLATFORM)/Devices_$(PLATFORM).pri)
