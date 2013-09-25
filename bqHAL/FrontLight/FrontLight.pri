base_project=$${PWD}

# Interface
HEADERS += $${base_project}/inc/FrontLight.h

SOURCES += $${base_project}/src/FrontLight.cpp

INCLUDEPATH += $${base_project}/$(PLATFORM)/inc
DEPENDPATH += $${base_project}/$(PLATFORM)/inc

include($(PLATFORM)/FrontLight_$(PLATFORM).pri)
