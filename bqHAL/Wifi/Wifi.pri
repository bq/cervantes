base_project=$${PWD}

HEADERS += $${base_project}/inc/Wifi.h

SOURCES += $${base_project}/src/Wifi.cpp

INCLUDEPATH += $${base_project}/$(PLATFORM)/inc
DEPENDPATH += $${base_project}/$(PLATFORM)/inc

include($${base_project}/$(PLATFORM)/Wifi_$(PLATFORM).pri)
