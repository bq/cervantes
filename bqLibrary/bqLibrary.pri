TRANSLATIONS = tr/bqLibrary_es.ts \
                tr/bqLibrary_ca.ts \
                tr/bqLibrary_en.ts \
                tr/bqLibrary_pt.ts \
                tr/bqLibrary_gl.ts \
                tr/bqLibrary_eu.ts \
                tr/bqLibrary_de.ts \
                tr/bqLibrary_fr.ts \
                tr/bqLibrary_it.ts


HEADERS += ./inc/Library.h \
         ./inc/LibraryBookListActions.h \
         ./inc/LibraryBooksFilterLayer.h \
         ./inc/LibraryAllBooksSubFilterLayer.h \
         ./inc/LibraryBooksSubFilterLayer.h \
         ./inc/LibrarySortBooksByLayer.h \
         ./inc/LibrarySortBrowserBooksByLayer.h \
         ./inc/LibraryBookSummary.h \
         ./inc/LibraryBreadCrumb.h \
         ./inc/LibraryGridViewer.h \
         ./inc/LibraryGridViewerItem.h \
         ./inc/LibraryIconGridViewer.h \
         ./inc/LibraryIconGridViewerItem.h \
         ./inc/LibraryLineGridViewer.h \
         ./inc/LibraryLineGridViewerItem.h \
         ./inc/LibraryLineGridCollectionsItem.h \
         ./inc/LibraryLineGridCollections.h \
         ./inc/LibraryBookToCollectionItem.h \
         ./inc/LibraryEditCollection.h \
         ./inc/LibraryPageHandler.h \
         ./inc/LibraryPageSlider.h \
         ./inc/LibraryReadingIconGridViewer.h \
         ./inc/LibraryReadingLineGridViewer.h \
         ./inc/LibraryActionsMenu.h \
         ./inc/LibraryImageFullScreen.h \
         ./inc/LibraryVerticalPager.h \
         ./inc/LibraryCollectionLayer.h \
         ./inc/LibraryCollectionItem.h

SOURCES += ./src/Library.cpp \
         ./src/LibraryBookListActions.cpp \
         ./src/LibraryBooksFilterLayer.cpp \
         ./src/LibraryAllBooksSubFilterLayer.cpp \
         ./src/LibraryBooksSubFilterLayer.cpp \
         ./src/LibrarySortBooksByLayer.cpp \
         ./src/LibrarySortBrowserBooksByLayer.cpp \
         ./src/LibraryBookSummary.cpp \
         ./src/LibraryBreadCrumb.cpp \
         ./src/LibraryGridViewer.cpp \
         ./src/LibraryGridViewerItem.cpp \
         ./src/LibraryIconGridViewer.cpp \
         ./src/LibraryIconGridViewerItem.cpp \
         ./src/LibraryLineGridViewer.cpp \
         ./src/LibraryLineGridViewerItem.cpp \
         ./src/LibraryLineGridCollectionsItem.cpp \
         ./src/LibraryLineGridCollections.cpp \
         ./src/LibraryBookToCollectionItem.cpp \
         ./src/LibraryEditCollection.cpp \
         ./src/LibraryPageHandler.cpp \
         ./src/LibraryPageSlider.cpp \
         ./src/LibraryReadingIconGridViewer.cpp \
         ./src/LibraryReadingLineGridViewer.cpp \
         ./src/LibraryImageFullScreen.cpp \
         ./src//LibraryActionsMenu.cpp \
         ./src/LibraryVerticalPager.cpp \
         ./src/LibraryCollectionLayer.cpp \
         ./src/LibraryCollectionItem.cpp

FORMS += ./ui/Library.ui \
         ./ui/LibraryBookListActions.ui \
         ./ui/LibraryBooksFilterLayer.ui \
         ./ui/LibraryAllBooksSubFilterLayer.ui \
         ./ui/LibraryBooksSubFilterLayer.ui \
         ./ui/LibrarySortBooksByLayer.ui \
         ./ui/LibrarySortBrowserBooksByLayer.ui \
         ./ui/LibraryBookSummary.ui \
         ./ui/LibraryBreadCrumb.ui \
         ./ui/LibraryIconGridViewer.ui \
         ./ui/LibraryIconGridViewerItem.ui \
         ./ui/LibraryLineGridViewer.ui \
         ./ui/LibraryLineGridViewerItem.ui \
         ./ui/LibraryLineGridCollectionsItem.ui \
         ./ui/LibraryLineGridCollections.ui \
         ./ui/LibraryBookToCollectionItem.ui \
         ./ui/LibraryEditCollection.ui \
         ./ui/LibraryPageHandler.ui \
         ./ui/LibraryReadingIconGridViewer.ui \
         ./ui/LibraryReadingLineGridViewer.ui \
         ./ui//LibraryActionsMenu.ui \
         ./ui/LibraryImageFullScreen.ui \
         ./ui/LibraryVerticalPager.ui \
         ./ui/LibraryCollectionLayer.ui \
         ./ui/LibraryCollectionItem.ui

QRC_FILES += Library800.qrc Library1024.qrc

OTHER_FILES += \
    bqLibrary/res/library_styles_generic_new.qss \
    bqLibrary/res/library_styles_generic.qss \
    bqLibrary/res/1024/library_styles.qss \
    bqLibrary/res/800/library_styles_new.qss \
    bqLibrary/res/800/library_styles.qss


