QT -= network
QT += sql
LIBS -= -lQtNetwork
LIBS += -lpoppler-qt4 -lzip -L/opt/x86-rootfs-devel/usr/lib -lepub
HEADERS += ../inc/MetaDataExtractor.h
SOURCES += MetaDataExtractor.cpp PdfMetaDataExtractor.cpp EpubMetaDataExtractor.cpp Fb2MetaDataExtractor.cpp test.cpp
INCLUDEPATH += ../inc /opt/x86-rootfs-devel/usr/include/
