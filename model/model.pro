CONFIG  += shared debug
TEMPLATE = lib
QT      += xml
TARGET = bqModelLibrary

HEADERS += ./inc/BookLocation.h \
           ./inc/BookInfo.h \
           ./inc/Model.h \
           ./inc/IModelBackend.h \
           ./inc/DeleteLaterFile.h \
           ./inc/ModelBackendOneFile.h \
           ../metadata-extractor/inc/MetaDataExtractor.h \
           ../metadata-extractor/inc/PdfMetaDataExtractor.h \
           ../metadata-extractor/inc/EpubMetaDataExtractor.h \
           ../metadata-extractor/inc/Fb2MetaDataExtractor.h \
           ../metadata-extractor/inc/MobiMetaDataExtractor.h

SOURCES +=  ./src/BookLocation.cpp \
            ./src/BookInfo.cpp \
            ./src/ModelBackendOneFile.cpp \
            ./src/Model.cpp \
            ./src/DeleteLaterFile.cpp \
            ../metadata-extractor/src/MetaDataExtractor.cpp \
            ../metadata-extractor/src/PdfMetaDataExtractor.cpp \
            ../metadata-extractor/src/EpubMetaDataExtractor.cpp \
            ../metadata-extractor/src/Fb2MetaDataExtractor.cpp \
            ../metadata-extractor/src/MobiMetaDataExtractor.cpp

INCLUDEPATH += ./src ./inc ../metadata-extractor/src ../metadata-extractor/inc ../bqUtils/inc
linux-arm-g++ {
        include ( main.conf )
        DESTDIR = lib
        LIBS += -L../bqUtils/lib -lbqUtils -lepub -lzip -lcrengine
} else {
        INCLUDEPATH +=$$(ROOTFS)/usr/include
        DESTDIR = x86-lib
        LIBS += -L../bqUtils/x86-lib -lbqUtils -lzip -L$$(ROOTFS)/usr/lib -lepub
}

QMAKE_CLEAN = libbqModelLibrary.so*
