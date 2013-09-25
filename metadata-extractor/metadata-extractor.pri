HEADERS += ./inc/EpubMetaDataExtractor.h \
           ./inc/MetaDataExtractor.h \
           ./inc/PdfMetaDataExtractor.h \
           ./inc/Fb2MetaDataExtractor.h \

SOURCES += ./src/EpubMetaDataExtractor.cpp \
           ./src/MetaDataExtractor.cpp \
           ./src/PdfMetaDataExtractor.cpp \
           ./src/Fb2MetaDataExtractor.cpp \

linux-arm-g++ { # Cross compilation for arm
	LIBS += -lepub -lzip -lxml2
} else {
        LIBS += -Wl,-rpath,-L$${ROOTFS}/usr/lib/ -L$${ROOTFS}/usr/lib/ -lepub -lzip
}
