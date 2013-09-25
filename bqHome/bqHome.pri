TRANSLATIONS = tr/bqHome_es.ts \
                tr/bqHome_ca.ts \
                tr/bqHome_en.ts \
                tr/bqHome_pt.ts \
                tr/bqHome_gl.ts \
                tr/bqHome_eu.ts

HEADERS += ./inc/Home.h \
           ./inc/HomeBooksList.h \
           ./inc/HomeBooksListItem.h \
           ./inc/HomeBooksListOptions.h

SOURCES += ./src/Home.cpp \
           ./src/HomeBooksList.cpp \
           ./src/HomeBooksListItem.cpp \
           ./src/HomeBooksListOptions.cpp

FORMS += ./ui/Home.ui \
         ./ui/HomeBooksList.ui \
         ./ui/HomeBooksListItem.ui \
         ./ui/HomeBooksListOptions.ui



QRC_FILES += Home800.qrc Home1024.qrc

OTHER_FILES += \
    bqHome/res/1024/home_styles.qss \
    bqHome/res/home_styles_generic.qss \
    bqHome/res/800/home_styles.qss
