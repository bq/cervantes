base_project=$${PWD}

# Interface
HEADERS += $${base_project}/inc/Power.h \
           $${base_project}/inc/RTCManager.h

SOURCES += $${base_project}/src/Power.cpp \
           $${base_project}/src/RTCManager.cpp

INCLUDEPATH += $${base_project}/$(PLATFORM)/inc
DEPENDPATH += $${base_project}/$(PLATFORM)/inc

include($${base_project}/$(PLATFORM)/Power_$(PLATFORM).pri)
