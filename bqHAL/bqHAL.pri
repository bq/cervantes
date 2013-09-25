base=bqHAL

Q_PROJECTS = \
    Power \
    Devices \
    Storage \
    Battery \     
    Wifi \     
    Screen \
    FrontLight

DEPENDPATH += $${Q_PROJECTS}
VPATH += $${Q_PROJECTS}

CONFIG(mx508) {
    DEFINES += MX508
}

# Include projects depending on platform
for(prj, Q_PROJECTS) {
    INCLUDEPATH += $${base}/$${prj}/inc
    DEPENDPATH += $${base}/$${prj}/inc
    include($${base}/$${prj}/$${prj}.pri)
}
