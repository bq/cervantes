HEADERS += ./inc/bqPublicServicesClientRequester.h \
           ./inc/bqPublicServicesClient.h \
           ./inc/bqPublicServices.h

SOURCES +=  ./src/bqPublicServicesClientRequester.cpp \
            ./src/bqPublicServicesClient.cpp \
            ./src/bqPublicServices.cpp

INCLUDEPATH += ./src ./inc

TRANSLATIONS = tr/bqPublicServices_es.ts \
                tr/bqPublicServices_ca.ts \
                tr/bqPublicServices_en.ts \
                tr/bqPublicServices_pt.ts \
                tr/bqPublicServices_gl.ts \
                tr/bqPublicServices_eu.ts

