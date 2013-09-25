base_project=$${PWD}

HEADERS += $${base_project}/inc/Battery.h

SOURCES += $${base_project}/src/Battery.cpp

INCLUDEPATH += $${base_project}/$(PLATFORM)/inc
DEPENDPATH += $${base_project}/$(PLATFORM)/inc

include($${base_project}/$(PLATFORM)/Battery_$(PLATFORM).pri)
