base_project=$${PWD}

HEADERS += \
        $${base_project}/inc/Screen.h


SOURCES += \
        $${base_project}/src/Screen.cpp

INCLUDEPATH += $${base_project}/$(PLATFORM)/inc
DEPENDPATH += $${base_project}/$(PLATFORM)/inc

include($${base_project}/$(PLATFORM)/Screen_$(PLATFORM).pri)
