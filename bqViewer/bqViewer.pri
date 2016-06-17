# Input
HEADERS += ./inc/Viewer.h \
           ./inc/ViewerMenu.h \
           ./inc/ViewerMenuPopUp.h \
           ./inc/ViewerAppearancePopup.h \
           ./inc/ViewerGotoPopup.h \
           ./inc/ViewerBookSummary.h \
           ./inc/ViewerContentsItem.h \
           ./inc/ViewerContentsList.h \
           ./inc/ViewerContentsPopup.h \
           ./inc/ViewerContentsPopupWidget.h \
           ./inc/ViewerVerticalPagerPopup.h \
           ./inc/ViewerTextActionsPopup.h \
           ./inc/ViewerTextActionsMenu.h \
           ./inc/ViewerMarkHandler.h \
           ./inc/ViewerSearchContextMenu.h\
           ./inc/ViewerSearchContextMenuLandscape.h\
           ./inc/ViewerSearchPopup.h \
           ./inc/ViewerSearchResultItem.h \
           ./inc/ViewerEditNotePopup.h \
           ./inc/ViewerBookListActions.h \
           ./inc/ViewerAnnotationsList.h \
           ./inc/ViewerAnnotationActions.h \
           ./inc/ViewerAnnotationItem.h \
           ./inc/ViewerPageHandler.h \
           ./inc/ViewerPageHandlerLandscape.h \
           ./inc/ViewerPdfPageHandler.h \
           ./inc/ViewerPdfPageHandlerLandscape.h \
           ./inc/ViewerBookmark.h \
           ./inc/ViewerBookmarkLandscape.h \
           ./inc/ViewerFloatingNote.h \
           ./inc/ViewerNoteActionsPopup.h \
           ./inc/ViewerStepsManager.h \
           ./inc/ViewerDelimiter.h \
           ./inc/MiniatureView.h \
           ./inc/MiniatureViewLandscape.h \
           ./inc/ViewerCollectionLayer.h \
           ./inc/ViewerCollectionItem.h

isEmpty(HACKERS_EDITION) {
HEADERS += ./inc/ViewerDictionary.h \
           ./inc/Dictionary.h \
           ./inc/semantix_dixio.h \
           ./inc/DictionaryLayer.h
}

SOURCES += ./src/Viewer.cpp \
           ./src/ViewerMenu.cpp \
           ./src/ViewerMenuPopUp.cpp \
           ./src/ViewerAppearancePopup.cpp \
           ./src/ViewerGotoPopup.cpp \
           ./src/ViewerBookSummary.cpp \
           ./src/ViewerContentsItem.cpp \
           ./src/ViewerContentsList.cpp \
           ./src/ViewerContentsPopup.cpp \
           ./src/ViewerVerticalPagerPopup.cpp \
           ./src/ViewerTextActionsPopup.cpp \
           ./src/ViewerTextActionsMenu.cpp \
           ./src/ViewerMarkHandler.cpp \
           ./src/ViewerSearchContextMenu.cpp\
           ./src/ViewerSearchContextMenuLandscape.cpp\
           ./src/ViewerSearchPopup.cpp \
           ./src/ViewerSearchResultItem.cpp \
           ./src/ViewerEditNotePopup.cpp \
           ./src/ViewerBookListActions.cpp \
           ./src/ViewerAnnotationsList.cpp \
           ./src/ViewerAnnotationActions.cpp \
           ./src/ViewerAnnotationItem.cpp \
           ./src/ViewerPageHandler.cpp \
           ./src/ViewerPageHandlerLandscape.cpp \
           ./src/ViewerPdfPageHandler.cpp \
           ./src/ViewerPdfPageHandlerLandscape.cpp \
           ./src/ViewerBookmark.cpp \
           ./src/ViewerBookmarkLandscape.cpp \
           ./src/ViewerFloatingNote.cpp \
           ./src/ViewerNoteActionsPopup.cpp\
           ./src/ViewerStepsManager.cpp \
           ./src/ViewerDelimiter.cpp\
           ./src/MiniatureView.cpp \
           ./src/MiniatureViewLandscape.cpp \
           ./src/ViewerCollectionLayer.cpp \
           ./src/ViewerCollectionItem.cpp

isEmpty(HACKERS_EDITION) {
SOURCES += ./src/ViewerDictionary.cpp \
           ./src/Dictionary.cpp \
           ./src/DictionaryLayer.cpp
}


FORMS +=   ./ui/Viewer.ui \
           ./ui/ViewerMenu.ui \
           ./ui/ViewerGotoPopup.ui \
           ./ui/ViewerAppearancePopup.ui \
           ./ui/ViewerBookSummary.ui \
           ./ui/ViewerContentsItem.ui \
           ./ui/ViewerContentsList.ui \
           ./ui/ViewerContentsPopup.ui \
           ./ui/ViewerVerticalPagerPopup.ui \
           ./ui/ViewerTextActionsPopup.ui \
           ./ui/ViewerTextActionsMenu.ui \
           ./ui/ViewerSearchContextMenu.ui\
           ./ui/ViewerSearchContextMenuLandscape.ui\
           ./ui/ViewerSearchPopup.ui \
           ./ui/ViewerSearchResultItem.ui \
           ./ui/ViewerEditNotePopup.ui \
           ./ui/ViewerDictionaryDefinitionPopup.ui \
           ./ui/ViewerBookListActions.ui \
           ./ui/ViewerAnnotationsList.ui \
           ./ui/ViewerAnnotationItem.ui \
           ./ui/ViewerAnnotationActions.ui \
           ./ui/ViewerPageHandler.ui \
           ./ui/ViewerPageHandlerLandscape.ui \
           ./ui/ViewerPdfPageHandler.ui \
           ./ui/ViewerPdfPageHandlerLandscape.ui \
           ./ui/ViewerFloatingNote.ui \
           ./ui/DictionaryLayer.ui \
           ./ui/Bookmark.ui \
           ./ui/BookmarkLandscape.ui \
           ./ui/Social.ui \
           ./ui/ViewerNoteActionsPopup.ui \
           ./ui/MiniatureView.ui \
           ./ui/MiniatureViewLandscape.ui \
           ./ui/ViewerDelimiter.ui \
           ./ui/ViewerCollectionLayer.ui \
           ./ui/ViewerCollectionItem.ui

QRC_FILES += Viewer800.qrc Viewer1024.qrc Viewer1448.qrc


TRANSLATIONS = tr/bqViewer_es.ts \
                tr/bqViewer_ca.ts \
		tr/bqViewer_en.ts \
                tr/bqViewer_pt.ts \
                tr/bqViewer_gl.ts \
                tr/bqViewer_eu.ts \
                tr/bqViewer_fr.ts \
                tr/bqViewer_de.ts \
                tr/bqViewer_it.ts


linux-arm-g++ { # Cross compilation for arm

LIBS += -lbz2 -ldl -lz -lssl -lpng -lrtmp -lzip -lpng12 -llzma -lp11-kit -lfreetype -lfontconfig


}

OTHER_FILES += \
    bqViewer/res/800/viewer_styles.qss \
    bqViewer/res/1024/viewer_styles.qss \
    bqViewer/res/1448/viewer_styles.qss \
    bqViewer/res/viewer_styles_generic.qss
