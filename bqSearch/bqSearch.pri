#Header files
HEADERS +=  ./inc/Search.h \
            ./inc/SearchItem.h \
            ./inc/SearchPagerCont.h \
            ./inc/SearchItemShop.h

#Source files
SOURCES +=  ./src/Search.cpp \
            ./src/SearchItem.cpp \
            ./src/SearchPagerCont.cpp \
            ./src/SearchItemShop.cpp

#Forms
FORMS += ./ui/Search.ui \
         ./ui/SearchItem.ui \
         ./ui/SearchPagerCont.ui \
         ./ui/SearchItemShop.ui

#Translations
TRANSLATIONS =  ./tr/bqSearch_es.ts \
                ./tr/bqSearch_ca.ts \
                ./tr/bqSearch_en.ts \
                ./tr/bqSearch_pt.ts \
                ./tr/bqSearch_gl.ts \
                ./tr/bqSearch_eu.ts

QRC_FILES += Search800.qrc Search1024.qrc
