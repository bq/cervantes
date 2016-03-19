/*************************************************************************

bq Cervantes e-book reader application
Copyright (C) 2011-2016  Mundoreader, S.L

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the source code.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************/

#ifndef LIBRARY_H
#define LIBRARY_H

#include "QBookForm.h"
#include "BookInfo.h"
#include "ui_Library.h"
#include <QFuture>

#define THUMBNAIL_SUFIX ".thumbnail"
#define NO_COVER_RESOURCE ":/list/default_txt"

// Predeclarations
class QWidget;
class LibraryMode;
class LibraryGridViewer;
class LibraryIconGridViewer;
class LibraryLineGridViewer;
class LibraryReadingIconGridViewer;
class LibraryReadingLineGridViewer;
class LibraryLineGridCollections;
class LibraryImageFullScreen;
class LibraryEditCollection;
class LibraryBooksFilterLayer;
class LibraryActionsMenu;
class LibraryBooksSubFilterLayer;
class LibraryAllBooksSubFilterLayer;
class LibrarySortBooksByLayer;
class LibrarySortBrowserBooksByLayer;
class LibraryBookSummary;
class QFileInfo;
class Keyboard;
class QButtonGroup;
class ProgressDialog;
class PowerManagerLock;
class InfoDialog;

class Library : public QBookForm, protected Ui::Library
{
    Q_OBJECT
public:
    enum ELibrarySourceMode
    {
        ELSM_NONE,

        ELSM_ALL,
        ELSM_STORE,
        ELSM_COLLECTION,
        ELSM_BROWSER,
        ELSM_SEARCH
    };

    enum ELibraryFilterMode
    {
        ELFM_NONE,

        // ELSM_ALL
        ELFM_ALL,

        // ALL_SUBFILTER
        ELFM_ALL_ACTIVE,
        ELFM_ALL_NEW,
        ELFM_ALL_READ,

        // ELSM_STORE
        ELFM_STORE_ALL,
        ELFM_STORE_NEW,
        ELFM_STORE_ACTIVE,
        ELFM_STORE_READ,
        ELFM_STORE_SAMPLE,
        ELFM_STORE_PURCHASED,
        ELFM_STORE_SUBSCRIPTION,
        ELFM_STORE_ARCHIVED,
        ELFM_PREMIUM_ARCHIVED,

        ELFM_LIBRARY_MODE, // This helps to know where we have our data, in m_books or m_files.


        // ELSM_BROWSER
        ELFM_INTERNAL,
        ELFM_SD,

        ELFM_SEARCH,
        ELFM_COLLECTION,
        ELFM_EDIT_COLLECTION,
        ELFM_COLLECTIONS
    };

    enum ELibraryViewMode
    {
        ELVM_ICON,
        ELVM_LINE
    };

    enum ELibrarySortMode
    {
        ELSM_DATE,
        ELSM_TITLE,
        ELSM_AUTHOR
    };

    enum EFilesSortMode
    {
        EFSM_NAME,
        EFSM_DATE
    };

public:
                                        Library                             ( QWidget* parent = 0 );
    virtual                             ~Library                            ();

    void                                activateForm                        ();
    void                                deactivateForm                      ();

    ELibraryFilterMode                  getFilterMode                       () const { return m_filterMode; }
    ELibraryFilterMode                  getLastMode                         () const { return m_lastMode; }
    ELibraryViewMode                    getViewMode                         () const { return m_viewMode; }
    void                                setFilterMode                       (ELibraryFilterMode filter) { m_filterMode = filter; }



    QList<const BookInfo*>              m_books;
    QList<const BookInfo*>              m_searchBooks;
    QList<const BookInfo*>              m_browserBooks;
    QList<const QFileInfo*>             m_files;
    QList<QString>                      m_images;
    QList<const QFileInfo*>             m_dirs;
    int                                 m_page;
    int                                 m_totalPages;// NOTE epaz We put this here, instead on every GridViewer, to save some memory

    bool                                isCurrentPathEmpty                  () const;
    bool                                anyPopUpShown                       () const;
    QString                             getCurrentPathParent                () const;
    void                                setReloadModel                      ( bool value );
    bool                                reloadModel                         ();
    void                                hideAllElements                     ();

    QHash<QString, const BookInfo*>     m_selectedBooks;
    bool                                getUnarchivingBook                  () { return b_unarchivingBook; }
    bool                                isSearchView                        () { return b_hasSearch && m_filterMode == ELFM_SEARCH; }
    static void                         fromCover2Thumbnail                 (const QString& coverPath);

    void                                viewActiveBooks                     ();
    void                                setupLibrary                        ();

    static QString                      modifyStyleSheet                    (QString styleSheet, QString styleToModify);
    QString                             getCollectionSelected               () { return m_collection; }

public slots:
    void                                itemClicked                         ( const QString& path );
    void                                itemLongPressed                     ( const QString& path );
    void                                openSearchPath                      ( const QString& path , bool fromHomeSearch = false);

    void                                modelChanged                        ( const QString& path, int ); // Call it when the model has changed
    void                                changeReadState                     ( const BookInfo* bookInfo, BookInfo::readStateEnum state );

#ifndef HACKERS_EDITION
    void                                buyBook                             ( const BookInfo* bookInfo );
    void                                archiveBook                         ( const BookInfo* bookInfo );
    void                                unarchiveBook                       ( const BookInfo* bookInfo );
#endif

    void                                removeBook                          ( const BookInfo* bookInfo );
    void                                stopThumbnailGeneration             ( );
    void                                resumeThumbnailGeneration           ( );
    bool                                isUnarchivingBook                   ( ){ return b_unarchivingBook;}

    void                                connectedToPc                       ( bool );
    void                                reportedStoreLink                   ( );
    void                                setImageList                        (QList<QString> imagesList);
    void                                createNewCollection                 (const BookInfo* bookToAdd);
    void                                setFromViewer                       (bool fromViewer) { b_isFromViewer = fromViewer; }

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                        paintEvent                          ( QPaintEvent* event );
    virtual void                        mouseReleaseEvent                   ( QMouseEvent* event );
    virtual void                        keyReleaseEvent                     ( QKeyEvent* event );

signals:
    void                                searchFinished                      ();
    void                                fileCopied                          ( int );
    void                                returnToViewer                      ();

#ifndef HACKERS_EDITION
    void                                syncFinished                        ();
    void                                connectionReadyToSync               ();
    void                                syncStart                           ();
#endif

protected slots:
    void                                searchLineEditClicked               ();
    void                                performSearch                       ();
    void                                keyboardPressedChar                 ( const QString& input );
    void                                performSearchConcurrent             ( const QString& pattern );
    void                                handleSearchFinished                ();
    void                                closeSearchClicked                  ();
    void                                searchClicked                       ();
    void                                searchClear                         ();

    // Filters
    void                                allBooksSelected                    (bool forced = false);
    void                                allActiveBooksSelected              ();
    void                                allNewBooksSelected                 ();
    void                                allReadBooksSelected                ();
    void                                myBooksSelected                     ();
    void                                storeBooksSelected                  ();
    void                                myCollectionsSelected               ();
    void                                browserFileSelected                 ( QString path = "");

    void                                allStoreBooksSelected               ();
    void                                newBooksSelected                    ();
    void                                activeBooksSelected                 ();
    void                                readBooksSelected                   ();
    void                                sampleBooksSelected                 ();
    void                                purchasedBooksSelected              ();
    void                                subscriptionBooksSelected           ();
    void                                archivedStoreBooksSelected          ();
    void                                archivedPremiumBooksSelected        ();

    void                                internalMemorySelected              ( QString path = "");
    void                                sdSelected                          ( QString path = "");
    void                                selectCollection                    (const QString&);

    //Preview images handler
    void                                openImage                           ( const QString& path );
    void                                hideImage                           ();
    void                                previousImageRequest                ( const QString& curretViewPath );
    void                                nextImageRequest                    ( const QString& curretViewPath );
    void                                previousBookRequest                 (const BookInfo* currentBook);
    void                                nextBookRequest                     (const BookInfo *currentBook);

    // End Filters

#ifndef HACKERS_EDITION
    void                                buyBooksBtnClicked                  ();
    void                                synchronize                         ( bool hideBtn = false );
    void                                syncEnd                             ( int _status, bool dirtyLibrary );
    void                                goToShop                            ();
#endif
    void                                bookActionsSelectBtnClicked         ();
    void                                subFilterBtnClicked                 ();
    void                                sortByBtnClicked                    ();

//    void                                editBooksClicked                    ();
//    void                                selectAllBooks                      ();
//    void                                closeEditBooksClicked               ();

//    void                                archiveBooksClicked                 ();
//    void                                deleteBooksClicked                  ();

    void                                sortIconClicked                     ();
    void                                sortListClicked                     ();

    void                                sortBooksByAuthorClicked            ();
    void                                sortBooksByIndexClicked             ();
    void                                sortBooksByTitleClicked             ();
    void                                sortBooksByDateClicked              ();

    void                                sortFilesByNameClicked              ();
    void                                sortFilesByDateClicked              ();

    void                                pressedBreadCrumb                   ( const QString& );
    void                                copyFile                            ( const QString& );
    void                                exportNotes                         ( const QString& );
    void                                removeFile                          ( const QString& );
    void                                handleEditCollection                ( const QString& = "");
    void                                handleDeleteCollection              ( const QString& );

    void                                summaryHiding                       ();

#ifndef HACKERS_EDITION
    void                                displaySyncCancel                   ();
    void                                connectSynchronizeWifiObserver      ();
    void                                disconnectSynchronizeWifiObserver   ();
    void                                connectUnarchiveWifiObserver        ();
    void                                disconnectUnarchiveWifiObserver     ();
    void                                unarchiveBookWithConn               ();
    void                                unArchivedBook                      (int status, bool dirtyLibrary);
    void                                cancelUnarchiving                   ();
#endif
    void                                hideDialog                          ();

    void                                handleFulfillmentDone               ( QString fulfillmentId, bool bReturnable, QString fulfillmentDocPath );
    void                                handleFulfillmentError              ( QString errorMsg );
    void                                hideEditCollection                  ();
    void                                resetBookSummary                    (const BookInfo* bookInfo );

private:
    enum ELibraryItemClickedMode
    {
        ELICM_OPEN_CONTENT,
        ELICM_SELECT
    };

    LibraryGridViewer*                  m_currentView;// Weak ptr
    LibraryGridViewer*                  m_currentIconView;// Weak ptr
    LibraryGridViewer*                  m_currentLineView;// Weak ptr

    LibraryIconGridViewer*              m_iconGridViewer;
    LibraryLineGridViewer*              m_lineGridViewer;
    LibraryReadingIconGridViewer*       m_readingIconGridViewer;
    LibraryReadingLineGridViewer*       m_readingLineGridViewer;
    LibraryLineGridCollections*         m_lineGridCollection;
    LibraryImageFullScreen*             m_previewImage;
    LibraryEditCollection*              m_editCollection;

    LibraryBooksFilterLayer*            m_booksFilterLayer;
    LibraryBooksSubFilterLayer*         m_booksSubFilterLayer;
    LibraryAllBooksSubFilterLayer*      m_allBooksSubFilterLayer;
    LibrarySortBooksByLayer*            m_sortBooksByLayer;
    LibrarySortBrowserBooksByLayer*     m_sortBrowserBooksByLayer;

    LibraryBookSummary*                 m_bookSummary;
    LibraryActionsMenu*                 m_actionsMenu;

    ELibrarySourceMode                  m_sourceMode;
    ELibraryFilterMode                  m_filterMode;
    ELibraryFilterMode                  m_lastMode;
    ELibraryViewMode                    m_viewMode;
    ELibraryItemClickedMode             m_itemClickedMode;
    ELibrarySortMode                    m_booksSortMode, m_newBooksSortMode;
    EFilesSortMode                      m_filesSortMode;

    void                                deleteData                          ();
    void                                deleteSearchData                    ();
    void                                loadDir                             ( const QString& path );

    void                                showAllFilterUIStuff                ();
    void                                showStoreFilterUIStuff              ();
    void                                showMyCollectionFilterUIStuff       ();
    void                                showCollectionFilterUIStuff         ();
    void                                showBrowserFilterUIStuff            ();
    bool                                copyDir                             ( const QString&,const QString&, int& filesToCopy, int& filesCopied, int& positivelyCopied);

//    void                                updateSelectedBooks                 ();

    // Filter mode
    typedef                             void (Library::*fillDataCallback)(void);
    fillDataCallback                    fillNewDataCallback;
    void                                changeFilterMode                    ( ELibraryFilterMode mode );
    void                                fillAllData                         ();
    void                                fillAllStoreData                    ();
    void                                fillAllActiveData                   ();
    void                                fillActiveData                      ();
    void                                fillAllNewData                      ();
    void                                fillNewData                         ();
    void                                fillAllAlreadyReadData              ();
    void                                fillAlreadyReadData                 ();
    void                                fillSampleData                      ();
    void                                fillPurchasedData                   ();
    void                                fillSubscriptionData                ();
    void                                fillStoreArchivedData               ();
    void                                fillPremiumArchivedData             ();
    void                                fillCollectionData                  ();
    void                                fillCurrentPathFiles                ();

    //
    void                                searchInBooks                       ( const QString &_pattern );
    void                                searchInFilesRecursively            ( const QString& path, const QString& pattern );
    // Sort mode
    typedef                             void (Library::*sortDataCallback)(void);
    sortDataCallback                    sortCurrentDataCallback;
    sortDataCallback                    sortBooksDataCallback;
    sortDataCallback                    sortFilesDataCallback;
    void                                applySortMethod                     ();
    void                                sortBooksByAuthor                   ();
    void                                sortBooksByIndex                    ();
    void                                sortBooksByTitle                    ();
    void                                sortBooksByDate                     ();
    void                                sortFilesByName                     ();
    void                                sortFilesByDate                     ();
    //

    // View Mode
    void                                changeViewMode                      ( ELibraryViewMode mode );

    void                                gatherNumberOfItemsInFilters        ();

    QString                             m_basePath;
    QString                             m_currentPath;
    QString                             m_collection;

    // Search stuff
    QFuture<void>                       m_concurrentSearch;
    QString                             m_initialSearchLineEditText;
    Keyboard*                           m_keyboard;
    bool                                m_userTyped;
    bool                                m_userEvent;
    void                                cancelSearch                        ();
    //

    bool                                m_reloadModel;

    ProgressDialog*                     m_dialogSync;
    int                                 setUpSync                           (bool hideBtn);
    int                                 setUpUnarchiving                    ();
    void                                processSyncStart                    ();
    void                                processSyncEnd                      ();
    void                                displaySyncEnd                      ( bool wasOk, const QString& displayMsg = "" );
    const BookInfo*                     m_bookInfoToUnarchive;

    bool                                clearKeyboard                       ();
    bool                                hideKeyboard                        ();
    void                                showKeyboard                        ();

    PowerManagerLock*                   m_powerLock;
    bool                                b_unarchivingBook;
    bool                                b_hasSearch;
    bool                                b_isFromViewer;

    InfoDialog*                         m_waitingDialog;
    void                                processFulfillment                  ( const QString& path );

    void                                handleBooksSortModeUI               ();
    void                                setBooksSortModeCallback            ();
    bool                                isReallySearchView                  () { return m_filterMode == ELFM_SEARCH && b_hasSearch; }

    int                                 m_collectionsIdx;
};

#endif // LIBRARY_H
