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

#include "Library.h"

#include "QBookApp.h"
#include "Model.h"
#include "BookInfo.h"
#include "Storage.h"
#include "Keyboard.h"

#include "LibraryIconGridViewer.h"
#include "LibraryLineGridViewer.h"
#include "LibraryReadingIconGridViewer.h"
#include "LibraryReadingLineGridViewer.h"
#include "LibraryBooksFilterLayer.h"
#include "LibraryBooksSubFilterLayer.h"
#include "LibraryAllBooksSubFilterLayer.h"
#include "LibrarySortBooksByLayer.h"
#include "LibrarySortBrowserBooksByLayer.h"
#include "LibraryBookSummary.h"
#include "LibraryImageFullScreen.h"
#include "LibraryLineGridCollections.h"
#include "LibraryEditCollection.h"

#include "InfoDialog.h"
#include "ConfirmDialog.h"
#include "ProgressDialog.h"
#include "SelectionDialog.h"
#include "PowerManager.h"
#include "bqUtils.h"

#ifndef DISABLE_ADOBE_SDK
#include "AdobeDRM.h"
#endif

#include <QButtonGroup>
#include <QFile>
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFileInfo>
#include <QImageReader>

#include <QDebug>

#define CHECKED "background-color:#DADADA;"
#define UNCHECKED "background-color:#FFFFFF;"
#define COLLECTION_MAX_LENGTH 10

#define PRESSED "background-color:#303030;color:#FFFFFF"
#define RELEASED "background-color:#FFFFFF; color:#000000;"

Library::Library( QWidget* parent ) :
    QBookForm(parent)
  , m_currentView(NULL)
  , m_currentIconView(NULL)
  , m_currentLineView(NULL)
  , m_previewImage(NULL)
  , m_sourceMode(ELSM_NONE)
  , m_filterMode(ELFM_NONE)
  , m_lastMode(ELFM_NONE)
  , m_viewMode(ELVM_ICON)
  , m_itemClickedMode(ELICM_OPEN_CONTENT)
  , m_booksSortMode(ELSM_DATE)
  , m_newBooksSortMode(ELSM_TITLE)
  , m_filesSortMode(EFSM_NAME)
  , m_keyboard(NULL)
  , m_userTyped(false)
  , m_userEvent(false)
  , m_reloadModel(true)
  , m_dialogSync(NULL)
  , m_bookInfoToUnarchive(NULL)
  , b_unarchivingBook(false)
  , b_hasSearch(false)
  , b_isFromViewer(false)
  , m_collectionsIdx(0)
{
    setupUi(this);

    // Applying styles
    QFile fileSpecific(":/res/library_styles.qss");
    QFile fileCommons(":/res/library_styles_generic.qss");
    fileSpecific.open(QFile::ReadOnly);
    fileCommons.open(QFile::ReadOnly);

    QString styles = QLatin1String(fileSpecific.readAll() + fileCommons.readAll());
    setStyleSheet(styles);

    // SearchLineEdit
    m_initialSearchLineEditText = librarySearchLineEdit->text();
    connect(librarySearchLineEdit,  SIGNAL(clicked()),                          this, SLOT(searchLineEditClicked()));
    connect(this,                   SIGNAL(searchFinished()),                   this, SLOT(handleSearchFinished()));

    // GridViewers
    m_iconGridViewer = new LibraryIconGridViewer(booksContainer);
    m_lineGridViewer = new LibraryLineGridViewer(booksContainer);
    m_readingIconGridViewer = new LibraryReadingIconGridViewer(booksContainer);
    m_readingLineGridViewer = new LibraryReadingLineGridViewer(booksContainer);
    m_lineGridCollection = new LibraryLineGridCollections(booksContainer);
    m_editCollection = new LibraryEditCollection(this);

    m_iconGridViewer->init();
    m_lineGridViewer->init();
    m_readingIconGridViewer->init();
    m_readingLineGridViewer->init();
    m_lineGridCollection->init();

    m_iconGridViewer->setLibrary(this);
    m_lineGridViewer->setLibrary(this);
    m_readingIconGridViewer->setLibrary(this);
    m_readingLineGridViewer->setLibrary(this);
    m_lineGridCollection->setLibrary(this);

    m_iconGridViewer->hide();
    m_lineGridViewer->hide();
    m_readingIconGridViewer->hide();
    m_readingLineGridViewer->hide();
    m_lineGridCollection->hide();
    m_editCollection->hide();

    connect(m_iconGridViewer,       SIGNAL(itemClicked(const QString&)),        this, SLOT(itemClicked(const QString&)));
    connect(m_lineGridViewer,       SIGNAL(itemClicked(const QString&)),        this, SLOT(itemClicked(const QString&)));
    connect(m_readingIconGridViewer,SIGNAL(itemClicked(const QString&)),        this, SLOT(itemClicked(const QString&)));
    connect(m_readingLineGridViewer,SIGNAL(itemClicked(const QString&)),        this, SLOT(itemClicked(const QString&)));
    connect(m_lineGridCollection,   SIGNAL(itemClicked(const QString&)),        this, SLOT(selectCollection(const QString&)));

    connect(m_iconGridViewer,       SIGNAL(itemLongPress(const QString&)),      this, SLOT(itemLongPressed(const QString&)));
    connect(m_lineGridViewer,       SIGNAL(itemLongPress(const QString&)),      this, SLOT(itemLongPressed(const QString&)));
    connect(m_readingIconGridViewer,SIGNAL(itemLongPress(const QString&)),      this, SLOT(itemLongPressed(const QString&)));
    connect(m_readingLineGridViewer,SIGNAL(itemLongPress(const QString&)),      this, SLOT(itemLongPressed(const QString&)));

    connect(m_iconGridViewer,       SIGNAL(removeFile(const QString&)),         this, SLOT(removeFile(const QString&)));
    connect(m_lineGridViewer,       SIGNAL(removeFile(const QString&)),         this, SLOT(removeFile(const QString&)));
    connect(m_readingIconGridViewer,SIGNAL(removeFile(const QString&)),         this, SLOT(removeFile(const QString&)));
    connect(m_readingLineGridViewer,SIGNAL(removeFile(const QString&)),         this, SLOT(removeFile(const QString&)));

    connect(m_iconGridViewer,       SIGNAL(copyFile(const QString&)),           this, SLOT(copyFile(const QString&)));
    connect(m_lineGridViewer,       SIGNAL(copyFile(const QString&)),           this, SLOT(copyFile(const QString&)));
    connect(m_readingIconGridViewer,SIGNAL(copyFile(const QString&)),           this, SLOT(copyFile(const QString&)));
    connect(m_readingLineGridViewer,SIGNAL(copyFile(const QString&)),           this, SLOT(copyFile(const QString&)));

    connect(m_lineGridCollection,   SIGNAL(deleteCollection(const QString&)),   this, SLOT(handleDeleteCollection(const QString&)));
    connect(m_lineGridCollection,   SIGNAL(editCollection(const QString&)),     this, SLOT(handleEditCollection(const QString&)));

    connect(m_editCollection,       SIGNAL(hideMe()),                           this, SLOT(hideEditCollection()));
    //

    // BooksFilterLayer
    m_booksFilterLayer = new LibraryBooksFilterLayer(this);
    m_booksFilterLayer->hide();
    connect(m_booksFilterLayer,     SIGNAL(myBooksSelected()),                  this, SLOT(myBooksSelected()));
    connect(m_booksFilterLayer,     SIGNAL(storeBooksSelected()),               this, SLOT(storeBooksSelected()));
    connect(m_booksFilterLayer,     SIGNAL(myCollectionsSelected()),            this, SLOT(myCollectionsSelected()));
    connect(m_booksFilterLayer,     SIGNAL(browserFileSelected()),              this, SLOT(browserFileSelected()));
    //

    // BooksSubFilterLayer
    m_booksSubFilterLayer = new LibraryBooksSubFilterLayer(this);
    m_booksSubFilterLayer->hide();
    connect(m_booksSubFilterLayer,  SIGNAL(allBooksSelected()),                 this, SLOT(allStoreBooksSelected()));
    connect(m_booksSubFilterLayer,  SIGNAL(newBooksSelected()),                 this, SLOT(newBooksSelected()));
    connect(m_booksSubFilterLayer,  SIGNAL(activeBooksSelected()),              this, SLOT(activeBooksSelected()));
    connect(m_booksSubFilterLayer,  SIGNAL(readBooksSelected()),                this, SLOT(readBooksSelected()));
    connect(m_booksSubFilterLayer,  SIGNAL(sampleBooksSelected()),              this, SLOT(sampleBooksSelected()));
    connect(m_booksSubFilterLayer,  SIGNAL(purchasedBooksSelected()),           this, SLOT(purchasedBooksSelected()));
    connect(m_booksSubFilterLayer,  SIGNAL(subscriptionBooksSelected()),        this, SLOT(subscriptionBooksSelected()));
    connect(m_booksSubFilterLayer,  SIGNAL(archivedStoreBooksSelected()),       this, SLOT(archivedStoreBooksSelected()));
    connect(m_booksSubFilterLayer,  SIGNAL(archivedPremiumBooksSelected()),     this, SLOT(archivedPremiumBooksSelected()));
    //

    // AllBooksSubFilterLayer
    m_allBooksSubFilterLayer = new LibraryAllBooksSubFilterLayer(this);
    m_allBooksSubFilterLayer->hide();
    connect(m_allBooksSubFilterLayer,  SIGNAL(allBooksSelected()),              this, SLOT(allBooksSelected()));
    connect(m_allBooksSubFilterLayer,  SIGNAL(allActiveBooksSelected()),        this, SLOT(allActiveBooksSelected()));
    connect(m_allBooksSubFilterLayer,  SIGNAL(allNewBooksSelected()),           this, SLOT(allNewBooksSelected()));
    connect(m_allBooksSubFilterLayer,  SIGNAL(readBooksSelected()),             this, SLOT(allReadBooksSelected()));
    //

    // LibrarySortBooksByLayer
    m_sortBooksByLayer = new LibrarySortBooksByLayer(this);
    m_sortBooksByLayer->hide();
    connect(m_sortBooksByLayer,  SIGNAL(recentSelected()),                      this, SLOT(sortBooksByDateClicked()));
    connect(m_sortBooksByLayer,  SIGNAL(titleSelected()),                       this, SLOT(sortBooksByTitleClicked()));
    connect(m_sortBooksByLayer,  SIGNAL(authorSelected()),                      this, SLOT(sortBooksByAuthorClicked()));
    connect(m_sortBooksByLayer,  SIGNAL(indexSelected()),                       this, SLOT(sortBooksByIndexClicked()));
    //

    // LibrarySortBrowserBooksByLayer
    m_sortBrowserBooksByLayer = new LibrarySortBrowserBooksByLayer(this);
    m_sortBrowserBooksByLayer->hide();
    connect(m_sortBrowserBooksByLayer,  SIGNAL(nameSelected()),                 this, SLOT(sortFilesByNameClicked()));
    connect(m_sortBrowserBooksByLayer,  SIGNAL(dateSelected()),                 this, SLOT(sortFilesByDateClicked()));

    //
    //LibraryImageFullScreen
    m_previewImage = new LibraryImageFullScreen(this);
    m_previewImage->setFixedSize(Screen::getInstance()->screenWidth(), Screen::getInstance()->screenHeight());
    m_previewImage->hide();
    connect(m_previewImage,     SIGNAL(hideMe()),                               this,   SLOT(hideImage()));
    connect(m_previewImage,     SIGNAL(previousImageRequest(const QString&)),   this,   SLOT(previousImageRequest(const QString&)));
    connect(m_previewImage,     SIGNAL(nextImageRequest(const QString&)),       this,   SLOT(nextImageRequest(const QString&)));
    //

    // BookSummary
    m_bookSummary = new LibraryBookSummary(this);
    m_bookSummary->hide();
    connect(m_bookSummary,          SIGNAL(hideMe()),                           this, SLOT(summaryHiding()));
#ifndef HACKERS_EDITION
    connect(m_bookSummary,          SIGNAL(buyBook(const BookInfo*)),           this, SLOT(buyBook(const BookInfo*)));
    connect(m_bookSummary,          SIGNAL(archiveBook(const BookInfo*)),       this, SLOT(archiveBook(const BookInfo*)));
    connect(m_bookSummary,          SIGNAL(unarchiveBook(const BookInfo*)),     this, SLOT(unarchiveBook(const BookInfo*)));
#endif
    connect(m_bookSummary,          SIGNAL(removeBook(const BookInfo*)),        this, SLOT(removeBook(const BookInfo*)));
    connect(m_bookSummary,          SIGNAL(copyBook(const QString&)),           this, SLOT(copyFile(const QString&)));
    connect(m_bookSummary,          SIGNAL(exportNotes(const QString)),         this, SLOT(exportNotes(QString)));
    connect(m_bookSummary,          SIGNAL(changeReadState(const BookInfo*, BookInfo::readStateEnum)),   this, SLOT(changeReadState(const BookInfo*,BookInfo::readStateEnum)));
    connect(m_bookSummary,          SIGNAL(addNewCollection(const BookInfo*)),  this, SLOT(createNewCollection(const BookInfo*)));
    connect(m_bookSummary,          SIGNAL(nextBookRequest(const BookInfo*)),   this, SLOT(nextBookRequest(const BookInfo*)));
    connect(m_bookSummary,          SIGNAL(previousBookRequest(const BookInfo*)),this, SLOT(previousBookRequest(const BookInfo*)));
    connect(m_bookSummary,          SIGNAL(openBook(const QString&)),           this, SLOT(itemClicked(const QString&)));

    //

    // Connect the buttons
    connect(bookActionsSelectBtn,   SIGNAL(clicked()),                          this, SLOT(bookActionsSelectBtnClicked()));

    connect(clearSearchBtn,         SIGNAL(clicked()),                          this, SLOT(searchClear()));
    connect(searchBtn,              SIGNAL(clicked()),                          this, SLOT(searchClicked()));
    connect(synchronizeBtn,         SIGNAL(clicked()),                          this, SLOT(synchronize()));
    connect(closeSearchBtn,         SIGNAL(clicked()),                          this, SLOT(closeSearchClicked()));
    connect(internalMemoryBtn,      SIGNAL(clicked()),                          this, SLOT(internalMemorySelected()));
    connect(sdBtn,                  SIGNAL(clicked()),                          this, SLOT(sdSelected()));

    connect(sortIconBtn,            SIGNAL(clicked()),                          this, SLOT(sortIconClicked()));
    connect(sortListBtn,            SIGNAL(clicked()),                          this, SLOT(sortListClicked()));
    connect(sortByBtn,              SIGNAL(clicked()),                          this, SLOT(sortByBtnClicked()));
    connect(addColectionBtn,        SIGNAL(clicked()),                          this, SLOT(handleEditCollection()));
    connect(backToCollectionsBtn,   SIGNAL(clicked()),                          this, SLOT(myCollectionsSelected()));

    connect(showBooksSelectBtn,      SIGNAL(clicked()),                         this, SLOT(subFilterBtnClicked()));

    connect(breadCrumb,             SIGNAL(changeDirectory(const QString&)),    this, SLOT(pressedBreadCrumb(const QString&)));

#ifndef HACKERS_EDITION
    connect(storeBtn,               SIGNAL(clicked()),          QBookApp::instance(), SLOT(goToShop()));
#endif
    // Model signals
    Model* model = QBookApp::instance()->getModel();
    connect(model, SIGNAL(modelChanged(QString, int)),                          this, SLOT(modelChanged(const QString&, int)));

    //Open images from search
    connect(QBookApp::instance(), SIGNAL(openImage(const QString&)),            this, SLOT(openImage(const QString&)));

    // Start the first view
    sortBooksDataCallback = &Library::sortBooksByDate;
    sortFilesDataCallback = &Library::sortFilesByName;
    sortCurrentDataCallback = sortBooksDataCallback;
    m_viewMode = ELVM_ICON;
    librarySearchLineEdit->hide();
    selectBooksLbl->hide();
    backToCollectionsBtn->hide();
    closeSearchBtn->hide();
    clearSearchBtn->hide();
    showResultsLbl->hide();
    memoryType->hide();
    sortIconBtn->hide();
    sortListBtn->show();
    m_currentView = m_iconGridViewer;
    gatherNumberOfItemsInFilters();
    m_booksFilterLayer->setAllBooksChecked();
    m_booksSubFilterLayer->setAllBooksChecked();
    m_sortBooksByLayer->setRecentChecked();
    m_sortBrowserBooksByLayer->setNameChecked();
    sortByBtn->setText(m_sortBooksByLayer->getRecentSortName());
    allBooksSelected();

    m_powerLock = PowerManager::getNewLock(this);
}

Library::~Library()
{
    qDebug() << Q_FUNC_INFO;

    delete m_dialogSync;
    m_dialogSync = NULL;

    delete m_powerLock;
    m_powerLock = NULL;

    m_iconGridViewer->done();
    m_lineGridViewer->done();
    m_readingIconGridViewer->done();
    m_readingLineGridViewer->done();

    m_currentView = NULL;
    m_currentIconView = NULL;
    m_currentLineView = NULL;
    m_iconGridViewer = NULL;
    m_lineGridViewer = NULL;
    m_readingIconGridViewer = NULL;
    m_readingLineGridViewer = NULL;
    m_booksFilterLayer = NULL;
    m_bookSummary = NULL;

    deleteData();

    m_selectedBooks.clear();
}

void Library::activateForm()
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->getStatusBar()->setMenuTitle(tr("Biblioteca"));
    if(m_filterMode != ELFM_SEARCH)
        librarySearchLineEdit->setText(m_initialSearchLineEditText);

    m_booksSubFilterLayer->setSubscriptionBooksShown(QBook::settings().value("subscription/subscriptionAllowed", true).toBool());
    if(reloadModel())
        m_currentView->paint();
    // TODO: Revisar esto!!
//    else if ((m_filterMode == ELFM_NOW_READING || m_filterMode == ELFM_DOWNLOADED || m_filterMode == ELFM_ARCHIVED) && m_books.size() == 0) {
//        m_booksFilterLayer->setAllBooksChecked();
//        allBooksSelected();
//    }
    QTimer::singleShot(0, m_currentView,SLOT(resume()));
    QTimer::singleShot(100, QBookApp::instance(),SLOT(syncSubcriptionInfo()));
}

void Library::deactivateForm()
{
    qDebug() << Q_FUNC_INFO;

    hideAllElements();

    cancelSearch();

    // NOTE Do not call to clearKeyboard, to preserve the text inside the librarySearchLineEdit
    clearKeyboard();
    if(b_isFromViewer)
        setFromViewer(false);

//    if(m_itemClickedMode == ELICM_SELECT)
//        closeEditBooksClicked();

    m_currentView->pause();
}

void Library::paintEvent( QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void Library::deleteData()
{
    qDebug() << Q_FUNC_INFO;

    m_books.clear();
    deleteSearchData();
}

void Library::deleteSearchData()
{
    qDebug() << Q_FUNC_INFO;

    m_searchBooks.clear();

    QList<const QFileInfo*>::iterator it = m_files.begin();
    QList<const QFileInfo*>::iterator itEnd = m_files.end();
    for(; it != itEnd; ++it)
    {
        delete *it;
    }
    m_files.clear();

    it = m_dirs.begin();
    itEnd = m_dirs.end();
    for(; it != itEnd; ++it)
    {
        delete *it;
    }
    m_dirs.clear();
    m_images.clear();
    m_browserBooks.clear();
    b_hasSearch = false;
}

void Library::mouseReleaseEvent( QMouseEvent* event )
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->hideKeyboard();
    if(m_booksFilterLayer->isVisible())
    {
        event->accept();
        m_booksFilterLayer->hide();
    }

    if(m_bookSummary->isVisible())
    {
        event->accept();
        m_bookSummary->hide();
    }
    if(m_currentView->clearActionsMenu())
        event->accept();
    if(m_allBooksSubFilterLayer->isVisible())
    {
        event->accept();
        hideAllElements();
    }
    if(m_booksSubFilterLayer->isVisible())
    {
        event->accept();
        hideAllElements();
    }
    if(m_sortBooksByLayer->isVisible())
    {
        event->accept();
        hideAllElements();
    }
    if(m_sortBrowserBooksByLayer->isVisible())
    {
        event->accept();
        hideAllElements();
    }
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void Library::hideAllElements()
{
    qDebug() << Q_FUNC_INFO;
    QBookApp::instance()->hideKeyboard();
    m_booksFilterLayer->hide();
    m_bookSummary->close();
    m_currentView->getPageHandler()->show();
    m_currentView->clearActionsMenu();
    m_allBooksSubFilterLayer->hide();
    m_booksSubFilterLayer->hide();
    m_sortBooksByLayer->hide();
    m_sortBrowserBooksByLayer->hide();
    hideImage();
}

void Library::bookActionsSelectBtnClicked()
{
    qDebug() << Q_FUNC_INFO;
    m_userEvent = true;

    clearKeyboard();

    m_currentView->clearActionsMenu();

    // Show Books Filter Layer as a Popup
    if(!m_booksFilterLayer->isVisible())
    {
       m_booksFilterLayer->show();
       gatherNumberOfItemsInFilters();
    }
    else
        m_booksFilterLayer->hide();
}

void Library::subFilterBtnClicked()
{
    // TODO: REvisar!!
    qDebug() << Q_FUNC_INFO;
    m_userEvent = true;

    if(m_keyboard != NULL && m_keyboard->isVisible())
        m_keyboard->hide();

    // Show Books Filter Layer as a Popup
    if(m_sourceMode == ELSM_ALL)
    {
        if(!m_allBooksSubFilterLayer->isVisible())
           m_allBooksSubFilterLayer->show();
        else
            m_allBooksSubFilterLayer->hide();
    }
    else
    {
        if(!m_booksSubFilterLayer->isVisible())
           m_booksSubFilterLayer->show();
        else
            m_booksSubFilterLayer->hide();
    }
}

void Library::sortByBtnClicked()
{
    qDebug() << Q_FUNC_INFO;
    m_userEvent = true;

    if(m_keyboard != NULL && m_keyboard->isVisible())
        m_keyboard->hide();

    if(m_sourceMode != ELSM_BROWSER)
    {
        // Show Books Filter Layer as a Popup
        if(!m_sortBooksByLayer->isVisible())
           m_sortBooksByLayer->show();
        else
            m_sortBooksByLayer->hide();
    }
    else
    {
        // Show Books Filter Layer as a Popup
        if(!m_sortBrowserBooksByLayer->isVisible())
           m_sortBrowserBooksByLayer->show();
        else
            m_sortBrowserBooksByLayer->hide();
    }
}

#ifndef HACKERS_EDITION
void Library::buyBooksBtnClicked()
{
    qDebug() << Q_FUNC_INFO;
}

void Library::synchronize(bool hideBtn)
{
    qDebug() << Q_FUNC_INFO;

    hideAllElements();

    //Disables OTA while sync
    disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), QBookApp::instance(), SLOT(checkFwVersion()));
    disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), QBookApp::instance(), SLOT(synchronization()));

    int iRet = setUpSync(hideBtn);
    if(iRet >= 0)
        disconnectSynchronizeWifiObserver();

    if(iRet <= 0)
    {
        // Enables OTA
        connect(ConnectionManager::getInstance(), SIGNAL(connected()), QBookApp::instance(), SLOT(checkFwVersion()), Qt::UniqueConnection);
        connect(ConnectionManager::getInstance(), SIGNAL(connected()), QBookApp::instance(), SLOT(synchronization()), Qt::UniqueConnection);
        return;
    }

    qDebug() << Q_FUNC_INFO << "SetUpSync done";

    emit syncStart();
    QBookApp::instance()->synchronization(false);
}

void Library::goToShop()
{
#ifndef HACKERS_EDITION
    QBookApp::instance()->goToShop();
#endif
}

void Library::syncEnd(int _status, bool dirtyLibrary)
{
    qDebug() << Q_FUNC_INFO;

    switch(_status){

    case 0:
        displaySyncEnd(false);
        break;
    case 2:
        displaySyncEnd(true);
        break;
    case 1:
    default:
        displaySyncCancel();
        break;
    }
    processSyncEnd();

    // Enables OTA
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), QBookApp::instance(), SLOT(checkFwVersion()), Qt::UniqueConnection);
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), QBookApp::instance(), SLOT(synchronization()), Qt::UniqueConnection);
}

int Library::setUpSync(bool hideBtn)
{
    qDebug() << Q_FUNC_INFO;

    if(!QBookApp::instance()->isLinked())
    {
        QBookApp::instance()->goToWizard();
        return -2;
    }

    if(!ConnectionManager::getInstance()->isConnected())
    {
        qDebug() << Q_FUNC_INFO << "Connection Manager not connected.";
        connectSynchronizeWifiObserver();
        connect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disconnectSynchronizeWifiObserver()), Qt::UniqueConnection);
        QBookApp::instance()->requestConnection();
        return -1;
    }

    connect(QBookApp::instance()->getSyncHelper(), SIGNAL(Finished(int, bool)), this, SLOT(syncEnd(int,bool)), Qt::UniqueConnection);
    processSyncStart();

    Screen::getInstance()->queueUpdates();

    if (!m_dialogSync)
    {
        if(hideBtn)
            m_dialogSync = new ProgressDialog(this,tr("Sincronizacion en curso... Para desarchivar, por favor espera unos minutos."));
        else
            m_dialogSync = new ProgressDialog(this,tr("Sincronizando... Por favor, espera unos minutos"));

        m_dialogSync->setHideBtn(hideBtn);
        m_dialogSync->setTextValue(false);
        connect(m_dialogSync,SIGNAL(cancel()), this, SLOT(displaySyncCancel()), Qt::UniqueConnection);
        connect(m_dialogSync,SIGNAL(hideDialog()), this, SLOT(hideDialog()), Qt::UniqueConnection);
    }

    connect(QBookApp::instance()->getSyncHelper(), SIGNAL(ProgressChanged(int)), m_dialogSync, SLOT(setProgressBar(int)), Qt::UniqueConnection);
    if(QBookApp::instance()->isSynchronizing())
        m_dialogSync->setProgressBar(QBookApp::instance()->getSyncHelper()->getSyncProgress());
    else
        m_dialogSync->setProgressBar(0);

    m_dialogSync->show(); //Modal dialog
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    if(QBookApp::instance()->isSynchronizing())
    {
        return 0;
    }

    return 1;
}

void Library::connectSynchronizeWifiObserver()
{
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(synchronize()), Qt::UniqueConnection);
}

void Library::disconnectSynchronizeWifiObserver()
{
    disconnect(ConnectionManager::getInstance(),SIGNAL(connected()),this,SLOT(synchronize()));
}

void Library::connectUnarchiveWifiObserver()
{
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(unarchiveBookWithConn()), Qt::UniqueConnection);
}

void Library::disconnectUnarchiveWifiObserver()
{
    disconnect(ConnectionManager::getInstance(),SIGNAL(connected()),this,SLOT(unarchiveBookWithConn()));
}

void Library::cancelUnarchiving()
{
    b_unarchivingBook = false;
}

void Library::processSyncStart()
{
    qDebug () << Q_FUNC_INFO;
    synchronizeBtn->setEnabled(false);
}

void Library::processSyncEnd()
{
    qDebug () << Q_FUNC_INFO;
    synchronizeBtn->setEnabled(true);
}

void Library::displaySyncEnd( bool wasOk, const QString& displayMsg )
{
    qDebug () << Q_FUNC_INFO << "DialogSync: " << m_dialogSync << ", wasOk: " << wasOk;

    if(!m_dialogSync)
        return;

    if(!wasOk)
    {
        Screen::getInstance()->lockScreen();
        delete m_dialogSync;
        m_dialogSync = NULL;
        summaryHiding();
        QCoreApplication::processEvents();
        Screen::getInstance()->releaseScreen();

        ConfirmDialog* dialog = new ConfirmDialog(this, tr("Fallo de sincronizacion."));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        dialog->exec();
        delete dialog;
        return;
    }

    Screen::getInstance()->lockScreen();
    delete m_dialogSync;
    m_dialogSync = NULL;
    summaryHiding();
    QCoreApplication::processEvents();

    InfoDialog* dialog;

    if(QBookApp::instance()->getSyncHelper()->GetBooksSynchronized() == QBookApp::instance()->getSyncHelper()->GetBooksToSync())
    {
        if( displayMsg.size() > 0 )
        {
            dialog = new InfoDialog(this, displayMsg);
            Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
            Screen::getInstance()->releaseScreen();
            dialog->hideSpinner();
            dialog->showForSpecifiedTime();
            delete dialog;
        }
        else{
            Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
            Screen::getInstance()->releaseScreen();
        }
    }
    else
    {
        dialog = new InfoDialog(this, tr("Biblioteca sincronizada\npero algunos libros han fallado."));
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->releaseScreen();
        dialog->hideSpinner();
        dialog->showForSpecifiedTime();
        delete dialog;
    }
}

void Library::displaySyncCancel()
{
    qDebug () << Q_FUNC_INFO;
    QBookApp::instance()->cancelSync();

    if(m_dialogSync)
    {
        Screen::getInstance()->queueUpdates();
        delete m_dialogSync;
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
        m_dialogSync = NULL;
    }
}

void Library::buyBook( const BookInfo* bookInfo )
{
    qDebug() << Q_FUNC_INFO;
    QString shopBookUrl = QBook::settings().value("serviceURLs/book").toString() + bookInfo->isbn;
    QBookApp::instance()->openStoreWithUrl(shopBookUrl);
}
#endif

void Library::changeReadState( const BookInfo* bookInfo, BookInfo::readStateEnum state )
{
    qDebug() << Q_FUNC_INFO;

    QString readText = tr("You are about to change the book state ");
    readText += tr("from ");

    if(bookInfo->path.contains(Storage::getInstance()->getPrivatePartition()->getMountPoint()))
    {
        QString shop = QBook::settings().value("shopName", "Tienda").toString();
        readText += shop + " eBooks. \n";
    }
    else if(bookInfo->path.contains(Storage::getInstance()->getPublicPartition()->getMountPoint()))
        readText += tr("internal memory. \n");
    else if(Storage::getInstance()->getRemovablePartition() && bookInfo->path.contains(Storage::getInstance()->getRemovablePartition()->getMountPoint()))
        readText += tr("SD card. \n");

    switch(state)
    {
    case BookInfo::NO_READ_BOOK:
        readText += tr("This book will be marked as no read. ");
    break;
    case BookInfo::READING_BOOK:
        readText += tr("This book will be marked as now reading. ");
    break;
    case BookInfo::READ_BOOK:
        readText += tr("This book will be marked as read. ");
    break;
    }

    SelectionDialog* readDialog = new SelectionDialog(this, readText, tr("Change"));
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
    readDialog->exec();

    Screen::getInstance()->queueUpdates();

    if(readDialog->result())
    {
        QBookApp::instance()->getModel()->changeReadState(QBookApp::instance()->getModel()->getBookInfo(bookInfo->path), state);
        resetBookSummary(bookInfo);
    }

    delete readDialog;

    Screen::getInstance()->flushUpdates();
}

void Library::removeBook( const BookInfo* book )
{
    qDebug() << Q_FUNC_INFO;

    QString deleteText = tr("You are about to remove the book ");
    deleteText += tr("from ");

    if(book->path.contains(Storage::getInstance()->getPrivatePartition()->getMountPoint())){
        QString shop = QBook::settings().value("shopName", "Tienda").toString();
        shop.replace("liberdrac","libelista");
        deleteText += shop + " eBooks. \n";
    }else if(book->path.contains(Storage::getInstance()->getPublicPartition()->getMountPoint()))
        deleteText += tr("internal memory. \n");
    else if(Storage::getInstance()->getRemovablePartition() && book->path.contains(Storage::getInstance()->getRemovablePartition()->getMountPoint()))
        deleteText += tr("SD card. \n");

    deleteText += tr("This action will remove the book from device. ");

    if(book->path.contains(Storage::getInstance()->getPrivatePartition()->getMountPoint())) {
        if(book->m_type==BookInfo::BOOKINFO_TYPE_DEMO)
            deleteText += tr("The sample book will not be available for download.");
        else
            deleteText += tr("The book will be always available for download.");
    }

    SelectionDialog* deleteDialog = new SelectionDialog(this, deleteText, tr("Remove"));
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
    deleteDialog->exec();

    if(deleteDialog->result())
    {
        Screen::getInstance()->queueUpdates();

        m_bookSummary->hideElements();
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        QBookApp::instance()->getSyncHelper()->removeBook(book->path);
        QBookApp::instance()->getModel()->removeBook(book);
        QtConcurrent::run(QBookApp::instance(), &QBookApp::syncModel);

        reloadModel();
        resetBookSummary(book);
        Screen::getInstance()->flushUpdates();

        InfoDialog* dialog = new InfoDialog(this,tr("Book correctly removed."));
        dialog->hideSpinner();
        dialog->showForSpecifiedTime();
        delete dialog;
        qDebug() << Q_FUNC_INFO << "End";
    }

    delete deleteDialog;
}

#ifndef HACKERS_EDITION
void Library::archiveBook( const BookInfo* bookInfo )
{
    qDebug () << Q_FUNC_INFO;

    if(QBookApp::instance()->isSynchronizing()){
        synchronize(true);
        return;
    }

    processSyncStart();

    QString archiveText = tr("You are about to archive the book ");
    archiveText += tr("from ");

    if(bookInfo->path.contains(Storage::getInstance()->getPrivatePartition()->getMountPoint())){
        QString shop = QBook::settings().value("shopName", "Tienda").toString();
        shop.replace("liberdrac","libelista");
        archiveText += shop + " eBooks. \n";
    }else if(bookInfo->path.contains(Storage::getInstance()->getPublicPartition()->getMountPoint()))
        archiveText += tr("internal memory. \n");
    else if(Storage::getInstance()->getRemovablePartition() && bookInfo->path.contains(Storage::getInstance()->getRemovablePartition()->getMountPoint()))
        archiveText += tr("SD card. \n");

    archiveText += tr("This action will remove the book from device. ");

    if(bookInfo->path.contains(Storage::getInstance()->getPrivatePartition()->getMountPoint()))
        archiveText += tr("The book will be always available for download.");

    SelectionDialog* archiveDialog = new SelectionDialog(this, archiveText, tr("Archive"));
    archiveDialog->exec();

    if(archiveDialog->result()){
        qDebug () << Q_FUNC_INFO << "Start archiving";
        Screen::getInstance()->queueUpdates();
        QBookApp::instance()->getModel()->archiveBook(bookInfo);
        QBookApp::instance()->getSyncHelper()->archiveBook(bookInfo->path);
        reloadModel();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
    }

    delete archiveDialog;

    processSyncEnd();
}

void Library::unarchiveBook( const BookInfo* bookInfo )
{
    qDebug () << Q_FUNC_INFO;
    if(QBookApp::instance()->isSynchronizing()){
        synchronize(true);
        return;
    }

    b_unarchivingBook = true;
    m_bookInfoToUnarchive = bookInfo;
    m_powerLock->activate();
    unarchiveBookWithConn();
}

void Library::unarchiveBookWithConn()
{
    qDebug () << Q_FUNC_INFO;

    disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), QBookApp::instance(), SLOT(checkFwVersion()));
    disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), QBookApp::instance(), SLOT(synchronization()));

    int iRet = setUpUnarchiving();
    if(iRet >= 0)
        disconnectUnarchiveWifiObserver();

    if(iRet <= 0)
    {
        // Enables OTA
        connect(ConnectionManager::getInstance(), SIGNAL(connected()), QBookApp::instance(), SLOT(checkFwVersion()), Qt::UniqueConnection);
        connect(ConnectionManager::getInstance(), SIGNAL(connected()), QBookApp::instance(), SLOT(synchronization()), Qt::UniqueConnection);
        m_powerLock->release();
        return;
    }

    qDebug() << Q_FUNC_INFO << "SetUpSync done";

    QBookApp::instance()->getSyncHelper()->unArchiveBook(m_bookInfoToUnarchive->path);
}

void Library::unArchivedBook(int status, bool /*dirtyLibrary*/)
{
    qDebug () << Q_FUNC_INFO;

    switch(status){
    case 0:
    case 1:
        displaySyncEnd(false);
        break;
    case 2:
    {
        QBookApp::instance()->getModel()->unArchiveBook(m_bookInfoToUnarchive);
        displaySyncEnd(true, tr("Libro correctamente desarchivado."));
        break;
    }
    case 3:
    {
        displaySyncEnd(true, tr("Has llegado al limite de subscripciones de tu cuenta. Por favor archiva algun otro antes de desarchivar."));
        break;
    }
    default:
        break;
    }

    b_unarchivingBook = false;
    reloadModel();
    m_powerLock->release();
    if(m_filterMode == ELFM_STORE_ARCHIVED && m_books.size() == 0)
    {
        m_booksFilterLayer->setAllBooksChecked();
        allBooksSelected();
    }

    // Enables OTA
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), QBookApp::instance(), SLOT(checkFwVersion()), Qt::UniqueConnection);
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), QBookApp::instance(), SLOT(synchronization()), Qt::UniqueConnection);
}
#endif

void Library::myBooksSelected()
{
    qDebug() << Q_FUNC_INFO;

    if( m_sourceMode != ELSM_ALL )
    {
        m_sourceMode = ELSM_ALL;

        Screen::getInstance()->queueUpdates();
        bookActionsSelectBtn->setText(m_booksFilterLayer->getAllBooksName());

        if(m_lastMode == ELFM_NONE || m_lastMode == ELFM_ALL)
            allBooksSelected();
        m_booksFilterLayer->hide();
        showAllFilterUIStuff();

        hideAllElements();
        Screen::getInstance()->flushUpdates();
    }
    else
        hideAllElements();
}

void Library::allBooksSelected(bool forced)
{
    qDebug() << Q_FUNC_INFO << " m_sourceMode:" << m_sourceMode << " m_filterMode: " << m_filterMode;

    if(!forced)
        m_sourceMode = ELSM_ALL;

    m_filterMode = ELFM_ALL;

    if(m_sourceMode == ELSM_ALL || m_sourceMode == ELSM_NONE)
    {
        bookActionsSelectBtn->setText(m_booksFilterLayer->getAllBooksName());
        showBooksSelectBtn->setText(m_allBooksSubFilterLayer->getAllBooksName());
        m_allBooksSubFilterLayer->setAllBooksChecked();
    }
    else
    {
        showBooksSelectBtn->setText(m_booksSubFilterLayer->getAllBooksName());
        m_booksSubFilterLayer->setAllBooksChecked();
        bookActionsSelectBtn->setText(m_booksFilterLayer->getStoreBooksName());
    }
    Screen::getInstance()->queueUpdates();

    // Sort mode
    setBooksSortModeCallback();
    handleBooksSortModeUI();

    if(m_userEvent)
        QBookApp::instance()->getStatusBar()->setSpinner(true);

    m_currentIconView = m_iconGridViewer;
    m_currentLineView = m_lineGridViewer;
    fillNewDataCallback = &Library::fillAllData;
    changeFilterMode(ELFM_ALL);
    showAllFilterUIStuff();

    hideAllElements();
    if(m_userEvent)
        QBookApp::instance()->getStatusBar()->setSpinner(false);

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void Library::allActiveBooksSelected()
{
    qDebug() << Q_FUNC_INFO;

    if(m_filterMode != ELFM_ALL_ACTIVE)
    {
        QBookApp::instance()->getStatusBar()->setSpinner(true);

        m_filterMode = ELFM_ALL_ACTIVE;
        Screen::getInstance()->queueUpdates();

        showBooksSelectBtn->setText(m_allBooksSubFilterLayer->getAllActiveBooksName());

        m_currentIconView = m_readingIconGridViewer;
        m_currentLineView = m_readingLineGridViewer;
        fillNewDataCallback = &Library::fillAllActiveData;

        // Sort mode
        handleBooksSortModeUI();
        sortCurrentDataCallback = &Library::sortBooksByDate;

        changeFilterMode(ELFM_ALL_ACTIVE);        

        sortByBtn->hide();

        hideAllElements();
        QBookApp::instance()->getStatusBar()->setSpinner(false);

        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
    }
    else
        hideAllElements();
}

void Library::allNewBooksSelected()
{
    qDebug() << Q_FUNC_INFO;

    if(m_filterMode != ELFM_ALL_NEW)
    {
        m_filterMode = ELFM_ALL_NEW;
        QBookApp::instance()->getStatusBar()->setSpinner(true);

        Screen::getInstance()->queueUpdates();

        showBooksSelectBtn->setText(m_allBooksSubFilterLayer->getNewBooksName());

        m_currentIconView = m_iconGridViewer;
        m_currentLineView = m_lineGridViewer;
        fillNewDataCallback = &Library::fillAllNewData;

        // Sort mode
        handleBooksSortModeUI();
        setBooksSortModeCallback();

        changeFilterMode(ELFM_ALL_NEW);

        hideAllElements();
        QBookApp::instance()->getStatusBar()->setSpinner(false);

        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
    }
    else
        hideAllElements();
}

void Library::allReadBooksSelected()
{
    qDebug() << Q_FUNC_INFO;

    if(m_filterMode != ELFM_ALL_READ)
    {
        m_filterMode = ELFM_ALL_READ;
        QBookApp::instance()->getStatusBar()->setSpinner(true);

        Screen::getInstance()->queueUpdates();

        showBooksSelectBtn->setText(m_allBooksSubFilterLayer->getReadBooksName());

        m_currentIconView = m_iconGridViewer;
        m_currentLineView = m_lineGridViewer;
        fillNewDataCallback = &Library::fillAllAlreadyReadData;

        // Sort mode
        handleBooksSortModeUI();
        setBooksSortModeCallback();

        changeFilterMode(ELFM_ALL_READ);

        hideAllElements();
        QBookApp::instance()->getStatusBar()->setSpinner(false);

        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
    }
    else
        hideAllElements();
}

void Library::storeBooksSelected()
{
    qDebug() << Q_FUNC_INFO;

    if( m_sourceMode != ELSM_STORE )
    {
        m_sourceMode = ELSM_STORE;

        Screen::getInstance()->queueUpdates();
        bookActionsSelectBtn->setText(m_booksFilterLayer->getStoreBooksName());
        if(m_lastMode == ELFM_NONE || m_lastMode == ELFM_STORE_ALL)
        {
            m_booksSubFilterLayer->setAllBooksChecked();
            allStoreBooksSelected();
        }
        m_booksFilterLayer->hide();
        showStoreFilterUIStuff();

        hideAllElements();
        Screen::getInstance()->flushUpdates();
    }
    else
        hideAllElements();
}

void Library::myCollectionsSelected()
{
    qDebug() << Q_FUNC_INFO;

    bool bMaintainCollectionPage = (m_sourceMode == ELSM_COLLECTION);
    if( m_sourceMode != ELSM_COLLECTION || m_filterMode != ELFM_COLLECTIONS )
    {
        m_sourceMode = ELSM_COLLECTION;
        m_filterMode = ELFM_COLLECTIONS;

        Screen::getInstance()->queueUpdates();
        m_currentView->stop();
        bookActionsSelectBtn->setText(m_booksFilterLayer->getMyCollectionsName());

        m_booksFilterLayer->hide();

        hideAllElements();
        showMyCollectionFilterUIStuff();

        m_currentView = m_lineGridCollection;
        m_currentView->setCollections(QBookApp::instance()->getModel()->getCollections());

        m_page = 0;
        m_totalPages = 0;

        // Show the new current view (maybe it will start running tasks like generating thumbnails)
        m_currentView->start(bMaintainCollectionPage ? m_collectionsIdx : m_page);
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
    }
    else
        hideAllElements();
}

void Library::browserFileSelected( QString path)
{
    qDebug() << Q_FUNC_INFO;

    if( m_sourceMode != ELSM_BROWSER )
    {
        m_sourceMode = ELSM_BROWSER;

        Screen::getInstance()->queueUpdates();

        switch(m_filesSortMode)
        {
            case EFSM_DATE:
                sortByBtn->setText(m_sortBrowserBooksByLayer->getDateSortName());
                sortFilesDataCallback = &Library::sortFilesByDate;
            break;

            default: // case EFSM_NAME:
                sortByBtn->setText(m_sortBrowserBooksByLayer->getNameSortName());
                sortFilesDataCallback = &Library::sortFilesByName;
            break;

        }

        sortCurrentDataCallback = sortFilesDataCallback;

        m_booksFilterLayer->hide();
        bookActionsSelectBtn->setText(m_booksFilterLayer->getBrowserFileName());
        showBrowserFilterUIStuff();

        internalMemorySelected(path);

        Screen::getInstance()->flushUpdates();
    }
    else
        hideAllElements();
}

void Library::allStoreBooksSelected()
{
    qDebug() << Q_FUNC_INFO;

    if(m_filterMode != ELFM_STORE_ALL)
    {
        m_filterMode = ELFM_STORE_ALL;

        if(m_userEvent)
            QBookApp::instance()->getStatusBar()->setSpinner(true);

        Screen::getInstance()->queueUpdates();

        showBooksSelectBtn->setText(m_booksSubFilterLayer->getAllBooksName());

        m_currentIconView = m_iconGridViewer;
        m_currentLineView = m_lineGridViewer;
        fillNewDataCallback = &Library::fillAllStoreData;

        // Sort mode
        handleBooksSortModeUI();
        setBooksSortModeCallback();

        changeFilterMode(ELFM_STORE_ALL);

        if(m_userEvent)
            QBookApp::instance()->getStatusBar()->setSpinner(false);

        hideAllElements();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();

        // TODO: We can change this by introducing LibrarySortModes
        // With a activate, deactivate methods
        // and a modelHasChanged (in Library we will need allBooksChanged, downloadedBooksChanged...and sdCardStatusChanged(bool isThereSD), etc )
    }
    else
        hideAllElements();
}

void Library::newBooksSelected()
{
    qDebug() << Q_FUNC_INFO;

    if(m_filterMode != ELFM_STORE_NEW)
    {
        m_filterMode = ELFM_STORE_NEW;
        QBookApp::instance()->getStatusBar()->setSpinner(true);

        Screen::getInstance()->queueUpdates();

        showBooksSelectBtn->setText(m_booksSubFilterLayer->getNewBooksName());

        m_currentIconView = m_iconGridViewer;
        m_currentLineView = m_lineGridViewer;
        fillNewDataCallback = &Library::fillNewData;

        handleBooksSortModeUI();
        setBooksSortModeCallback();

        changeFilterMode(ELFM_STORE_NEW);

        QBookApp::instance()->getStatusBar()->setSpinner(false);

        hideAllElements();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
    }
    else
        hideAllElements();
}

void Library::activeBooksSelected()
{
    qDebug() << Q_FUNC_INFO;

    if(m_filterMode != ELFM_STORE_ACTIVE)
    {
        QBookApp::instance()->getStatusBar()->setSpinner(true);

        Screen::getInstance()->queueUpdates();

        showBooksSelectBtn->setText(m_booksSubFilterLayer->getActiveBooksName());

        m_currentIconView = m_readingIconGridViewer;
        m_currentLineView = m_readingLineGridViewer;
        fillNewDataCallback = &Library::fillActiveData;
        
        // Sort mode
        sortCurrentDataCallback = &Library::sortBooksByDate;

        changeFilterMode(ELFM_STORE_ACTIVE);

        QBookApp::instance()->getStatusBar()->setSpinner(false);

        hideAllElements();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
    }
    else
        hideAllElements();
}

void Library::readBooksSelected()
{
    qDebug() << Q_FUNC_INFO;

    if(m_filterMode != ELFM_STORE_READ)
    {
        QBookApp::instance()->getStatusBar()->setSpinner(true);

        Screen::getInstance()->queueUpdates();

        showBooksSelectBtn->setText(m_booksSubFilterLayer->getReadBooksName());

        m_currentIconView = m_iconGridViewer;
        m_currentLineView = m_lineGridViewer;
        fillNewDataCallback = &Library::fillAlreadyReadData;

        // Sort mode
        sortCurrentDataCallback = &Library::sortBooksByDate;

        changeFilterMode(ELFM_STORE_READ);

        QBookApp::instance()->getStatusBar()->setSpinner(false);

        hideAllElements();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
    }
    else
        hideAllElements();
}

void Library::sampleBooksSelected()
{
    qDebug() << Q_FUNC_INFO;

    if(m_filterMode != ELFM_STORE_SAMPLE)
    {
        QBookApp::instance()->getStatusBar()->setSpinner(true);

        Screen::getInstance()->queueUpdates();

        showBooksSelectBtn->setText(m_booksSubFilterLayer->getSampleBooksName());

        m_currentIconView = m_iconGridViewer;
        m_currentLineView = m_lineGridViewer;
        fillNewDataCallback = &Library::fillSampleData;
        changeFilterMode(ELFM_STORE_SAMPLE);

        QBookApp::instance()->getStatusBar()->setSpinner(false);

        hideAllElements();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
    }
    else
        hideAllElements();
}

void Library::purchasedBooksSelected()
{
    qDebug() << Q_FUNC_INFO;

    if(m_filterMode != ELFM_STORE_PURCHASED)
    {
        QBookApp::instance()->getStatusBar()->setSpinner(true);

        Screen::getInstance()->queueUpdates();

        showBooksSelectBtn->setText(m_booksSubFilterLayer->getPurchasedBooksName());

        m_currentIconView = m_iconGridViewer;
        m_currentLineView = m_lineGridViewer;
        fillNewDataCallback = &Library::fillPurchasedData;
        changeFilterMode(ELFM_STORE_PURCHASED);

        QBookApp::instance()->getStatusBar()->setSpinner(false);

        hideAllElements();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
    }
    else
        hideAllElements();
}

void Library::subscriptionBooksSelected()
{
    qDebug() << Q_FUNC_INFO;

    if(m_filterMode != ELFM_STORE_SUBSCRIPTION)
    {
        QBookApp::instance()->getStatusBar()->setSpinner(true);

        Screen::getInstance()->queueUpdates();

        showBooksSelectBtn->setText(m_booksSubFilterLayer->getSubscriptionBooksName());

        m_currentIconView = m_iconGridViewer;
        m_currentLineView = m_lineGridViewer;
        fillNewDataCallback = &Library::fillSubscriptionData;
        changeFilterMode(ELFM_STORE_SUBSCRIPTION);

        QBookApp::instance()->getStatusBar()->setSpinner(false);

        hideAllElements();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
    }
    else
        hideAllElements();
}

void Library::archivedStoreBooksSelected()
{
    qDebug() << Q_FUNC_INFO;

    if(m_filterMode != ELFM_STORE_ARCHIVED)
    {
        QBookApp::instance()->getStatusBar()->setSpinner(true);

        Screen::getInstance()->queueUpdates();

        showBooksSelectBtn->setText(m_booksSubFilterLayer->getStoreArchivedBooksName());

        m_currentIconView = m_iconGridViewer;
        m_currentLineView = m_lineGridViewer;
        fillNewDataCallback = &Library::fillStoreArchivedData;
        changeFilterMode(ELFM_STORE_ARCHIVED);

        QBookApp::instance()->getStatusBar()->setSpinner(false);

        hideAllElements();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
    }
    else
        hideAllElements();
}

void Library::archivedPremiumBooksSelected()
{
    qDebug() << Q_FUNC_INFO;

    if(m_filterMode != ELFM_PREMIUM_ARCHIVED)
    {
        QBookApp::instance()->getStatusBar()->setSpinner(true);

        Screen::getInstance()->queueUpdates();

        showBooksSelectBtn->setText(m_booksSubFilterLayer->getPremiumArchivedBooksName());

        m_currentIconView = m_iconGridViewer;
        m_currentLineView = m_lineGridViewer;
        fillNewDataCallback = &Library::fillPremiumArchivedData;
        changeFilterMode(ELFM_PREMIUM_ARCHIVED);

        QBookApp::instance()->getStatusBar()->setSpinner(false);

        hideAllElements();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
    }
    else
        hideAllElements();
}

void Library::internalMemorySelected( QString path)
{
    qDebug() << Q_FUNC_INFO;

    if(m_filterMode != ELFM_INTERNAL)
    {
        QBookApp::instance()->getStatusBar()->setSpinner(true);

        Screen::getInstance()->queueUpdates();

        hideAllElements();

        internalMemoryBtn->setStyleSheet(PRESSED);
        sdBtn->setStyleSheet(RELEASED);

        // Set Paths
        m_basePath = Storage::getInstance()->getPublicPartition()->getMountPoint();
        m_currentPath = path;
        if(!path.isEmpty())
            breadCrumb->show();
        else
            breadCrumb->hide();

        qDebug() << Q_FUNC_INFO << m_basePath << m_currentPath;
        breadCrumb->setDirectory(m_currentPath);

        m_currentIconView = m_iconGridViewer;
        m_currentLineView = m_lineGridViewer;
        fillNewDataCallback = &Library::fillCurrentPathFiles;
        changeFilterMode(ELFM_INTERNAL);

        QBookApp::instance()->getStatusBar()->setSpinner(false);

        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
    }
}

void Library::sdSelected( QString path )
{
    qDebug() << Q_FUNC_INFO;

    if(m_filterMode != ELFM_SD)
    {
        QBookApp::instance()->getStatusBar()->setSpinner(true);

        Screen::getInstance()->queueUpdates();
        hideAllElements();

        internalMemoryBtn->setStyleSheet(RELEASED);
        sdBtn->setStyleSheet(PRESSED);

        // Set Paths
        m_basePath = Storage::getInstance()->getRemovablePartition()->getMountPoint();
        m_currentPath = path;
        qDebug() << Q_FUNC_INFO << m_basePath << m_currentPath;

        if(!path.isEmpty())
            breadCrumb->show();
        else
            breadCrumb->hide();

        breadCrumb->setDirectory(m_currentPath);

        m_currentIconView = m_iconGridViewer;
        m_currentLineView = m_lineGridViewer;
        fillNewDataCallback = &Library::fillCurrentPathFiles;
        changeFilterMode(ELFM_SD);

        QBookApp::instance()->getStatusBar()->setSpinner(false);

        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
    }
}

void Library::sortIconClicked()
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->getStatusBar()->setSpinner(true);

    Screen::getInstance()->queueUpdates();

    clearKeyboard();

    m_currentView->clearActionsMenu();

    sortIconBtn->hide();
    sortListBtn->show();

    changeViewMode(ELVM_ICON);

    QBookApp::instance()->getStatusBar()->setSpinner(false);

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void Library::sortListClicked()
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->getStatusBar()->setSpinner(true);

    Screen::getInstance()->queueUpdates();

    clearKeyboard();
    m_currentView->clearActionsMenu();

    sortIconBtn->show();
    sortListBtn->hide();

    changeViewMode(ELVM_LINE);

    QBookApp::instance()->getStatusBar()->setSpinner(false);

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void Library::applySortMethod()
{
    qDebug() << Q_FUNC_INFO;

    m_currentView->pause();

    m_page = 0;

    (this->*sortCurrentDataCallback)();

    m_currentView->paint();
    m_currentView->resume();
}

void Library::sortBooksByAuthorClicked()
{
    qDebug() << Q_FUNC_INFO << "m_books: " << m_books.size();

    PowerManagerLock* powerLock = PowerManager::getNewLock(this);
    powerLock->activate();

    QBookApp::instance()->getStatusBar()->setSpinner(true);

    Screen::getInstance()->queueUpdates();

    clearKeyboard();

    m_sortBooksByLayer->hide();

    sortByBtn->setText(m_sortBooksByLayer->getAuthorSortName());

    if(m_filterMode == ELFM_ALL_NEW)
        m_newBooksSortMode = ELSM_AUTHOR;
    else
        m_booksSortMode = ELSM_AUTHOR;

    sortBooksDataCallback = &Library::sortBooksByAuthor;
    sortCurrentDataCallback = sortBooksDataCallback;

    applySortMethod();

    QBookApp::instance()->getStatusBar()->setSpinner(false);
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
    delete powerLock;
}

void Library::sortBooksByIndexClicked()
{
    qDebug() << Q_FUNC_INFO << "m_books: " << m_books.size();

    PowerManagerLock* powerLock = PowerManager::getNewLock(this);
    powerLock->activate();

    QBookApp::instance()->getStatusBar()->setSpinner(true);

    Screen::getInstance()->queueUpdates();

    clearKeyboard();

    m_sortBooksByLayer->hide();

    sortByBtn->setText(m_sortBooksByLayer->getIndexSortName());

    sortBooksDataCallback = &Library::sortBooksByIndex;
    sortCurrentDataCallback = sortBooksDataCallback;

    applySortMethod();

    QBookApp::instance()->getStatusBar()->setSpinner(false);
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
    delete powerLock;
}

void Library::sortBooksByTitleClicked()
{
    qDebug() << Q_FUNC_INFO;

    PowerManagerLock* powerLock = PowerManager::getNewLock(this);
    powerLock->activate();

    QBookApp::instance()->getStatusBar()->setSpinner(true);
    Screen::getInstance()->queueUpdates();

    clearKeyboard();
   
    m_sortBooksByLayer->hide();

    sortByBtn->setText(m_sortBooksByLayer->getTitleSortName());

    if(m_filterMode == ELFM_ALL_NEW)
        m_newBooksSortMode = ELSM_TITLE;
    else
        m_booksSortMode = ELSM_TITLE;

    sortBooksDataCallback = &Library::sortBooksByTitle;
    sortCurrentDataCallback = sortBooksDataCallback;

    applySortMethod();

    QBookApp::instance()->getStatusBar()->setSpinner(false);
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
    delete powerLock;
}

void Library::sortBooksByDateClicked()
{
    qDebug() << Q_FUNC_INFO;

    PowerManagerLock* powerLock = PowerManager::getNewLock(this);
    powerLock->activate();

    QBookApp::instance()->getStatusBar()->setSpinner(true);

    Screen::getInstance()->queueUpdates();


    clearKeyboard();

    m_sortBooksByLayer->hide();

    sortByBtn->setText(m_sortBooksByLayer->getRecentSortName());

    m_booksSortMode = ELSM_DATE;
    sortBooksDataCallback = &Library::sortBooksByDate;
    sortCurrentDataCallback = sortBooksDataCallback;

    applySortMethod();

    QBookApp::instance()->getStatusBar()->setSpinner(false);
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
    delete powerLock;
}

void Library::sortFilesByNameClicked()
{
    qDebug() << Q_FUNC_INFO;

    PowerManagerLock* powerLock = PowerManager::getNewLock(this);
    powerLock->activate();

    QBookApp::instance()->getStatusBar()->setSpinner(true);

    Screen::getInstance()->queueUpdates();

    clearKeyboard();
    m_currentView->clearActionsMenu();

    m_sortBrowserBooksByLayer->hide();

    sortByBtn->setText(m_sortBrowserBooksByLayer->getNameSortName());

    // Sort mode.
    m_filesSortMode = EFSM_NAME;
    sortFilesDataCallback = &Library::sortFilesByName;
    sortCurrentDataCallback = sortFilesDataCallback;

    applySortMethod();

    QBookApp::instance()->getStatusBar()->setSpinner(false);
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
    delete powerLock;
}

void Library::sortFilesByDateClicked()
{
    qDebug() << Q_FUNC_INFO;

    PowerManagerLock* powerLock = PowerManager::getNewLock(this);
    powerLock->activate();

    QBookApp::instance()->getStatusBar()->setSpinner(true);

    Screen::getInstance()->queueUpdates();

    clearKeyboard();
    m_currentView->clearActionsMenu();

    m_sortBrowserBooksByLayer->hide();

    sortByBtn->setText(m_sortBrowserBooksByLayer->getDateSortName());

    // Sort mode.
    m_filesSortMode = EFSM_DATE;
    sortFilesDataCallback = &Library::sortFilesByDate;
    sortCurrentDataCallback = sortFilesDataCallback;

    applySortMethod();

    QBookApp::instance()->getStatusBar()->setSpinner(false);
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
    delete powerLock;
}

bool sortByCollectionIndex(const BookInfo* b1, const BookInfo* b2)
{
    qDebug() << Q_FUNC_INFO;

    QString collectionName = QBookApp::instance()->getLibrary()->getCollectionSelected();
    QHash<QString, double> collections1 = b1->getCollectionsList();
    QHash<QString, double> collections2 = b2->getCollectionsList();
    QHash<QString, double>::iterator it1 = collections1.find(collectionName);
    QHash<QString, double>::iterator it2 = collections2.find(collectionName);

    if(it1 != collections1.end() && it1.key() == collectionName && it2 == collections2.end() && it2.key() != collectionName)
        return true;
    else if(it1 == collections1.end() && it1.key() != collectionName && it2 != collections2.end() && it2.key() == collectionName)
        return false;
    else if(it1 != collections1.end() && it1.key() == collectionName && it2 != collections2.end() && it2.key() == collectionName)
    {
        if (it1.value() == it2.value())
            return (titleLessThan(b1, b2));
        else
            return (it1.value() < it2.value());
    }
}

void Library::sortBooksByAuthor()
{
    qDebug() << Q_FUNC_INFO << "m_books:" << m_books.size();

    // TODO: ask the model to sort it for us (but this way is faster, because we are omiting the books we don't care)

    qSort(m_books.begin(), m_books.end(), authorLessThan);

    foreach(const BookInfo* book, m_books){
        qDebug() << Q_FUNC_INFO << book->author;
    }

}

void Library::sortBooksByIndex()
{
    qDebug() << Q_FUNC_INFO << "m_books:" << m_books.size();

    // TODO: ask the model to sort it for us (but this way is faster, because we are omiting the books we don't care)

    qSort(m_books.begin(), m_books.end(), sortByCollectionIndex);

}

void Library::sortBooksByTitle()
{
    qDebug() << Q_FUNC_INFO;

    // TODO: ask the model to sort it for us (but this way is faster, because we are omiting the books we don't care)
    qSort(m_books.begin(), m_books.end(), titleLessThan);
}

void Library::sortBooksByDate()
{
    qDebug() << Q_FUNC_INFO;

    // TODO: ask the model to sort it for us (but this way is faster, because we are omiting the books we don't care)
    qSort(m_books.begin(), m_books.end(), readingTimeAfterThan);
}

bool dirBooksFilesSortedByName( const QFileInfo* f1, const QFileInfo* f2)
{
    if(f1->isDir() && f2->isDir())
        return f1->completeBaseName().toLower() < f2->completeBaseName().toLower();
    else if(!f1->isDir() && f2->isDir())
        return false;
    else if (f1->isDir() && !f2->isDir())
        return true;
    else
    {
        const BookInfo* bookInfo1 = QBookApp::instance()->getModel()->getBookInfo(f1->filePath());
        const BookInfo* bookInfo2 = QBookApp::instance()->getModel()->getBookInfo(f2->filePath());
        if(bookInfo1)
        {
            if(!bookInfo2)
                return true;
            else
                return bookInfo1->title.toLower().localeAwareCompare(bookInfo2->title.toLower()) < 0;
        }
        else
        {
            if(bookInfo2)
                return false;
            else
                return f1->completeBaseName().toLower() < f2->completeBaseName().toLower();
        }
    }
}

void Library::sortFilesByName()
{
    qDebug() << Q_FUNC_INFO;

    // TODO: ask the model to sort it for us (but this way is faster, because we are omiting the books we don't care)
    qSort(m_files.begin(), m_files.end(), dirBooksFilesSortedByName);
}

bool fileTimeLessThan( const QFileInfo* f1, const QFileInfo* f2)
{
    if(f1->isDir() && f2->isDir())
        return f1->lastModified() < f2->lastModified();
    else if(!f1->isDir() && f2->isDir())
        return false;
    else if (f1->isDir() && !f2->isDir())
        return true;
    else
    {
        const BookInfo* bookInfo1 = QBookApp::instance()->getModel()->getBookInfo(f1->filePath());
        const BookInfo* bookInfo2 = QBookApp::instance()->getModel()->getBookInfo(f2->filePath());
        if(bookInfo1)
        {
            if(!bookInfo2)
                return true;
            else
                return f1->lastModified() < f2->lastModified();
        }
        else
        {
            if(bookInfo2)
                return false;
            else
                return f1->lastModified() < f2->lastModified();
        }
    }
}

void Library::sortFilesByDate()
{
    qDebug() << Q_FUNC_INFO;

    // TODO: ask the model to sort it for us (but this way is faster, because we are omiting the books we don't care)
    qSort(m_files.begin(), m_files.end(), fileTimeLessThan);
}

void Library::summaryHiding()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();

    m_bookSummary->close();
    reloadModel();
    m_currentView->getPageHandler()->show();

    int pos = m_bookSummary->getCurrentBookOffset();

    int currentPage = int(pos/m_currentView->getItemsPerPage());

    while( currentPage > 0 )
    {
        if(currentPage == m_totalPages)
            --currentPage;
        else
        {
            m_page = currentPage;
            m_currentView->paint();
            break;
        }
    }

    Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void Library::pressedBreadCrumb( const QString& newPath )
{
    qDebug() << Q_FUNC_INFO << ": newPath = " << newPath;

    clearKeyboard();

    if(newPath.size())
        itemClicked(m_basePath + "/" + newPath);
    else
        itemClicked(m_basePath);
}

void Library::itemClicked( const QString& path )
{
    qDebug() << Q_FUNC_INFO << "Path: " << path;

    if(clearKeyboard())
        return;
    if(m_currentView->clearActionsMenu())
        return;

    if( m_itemClickedMode == ELICM_OPEN_CONTENT )
    {
        const BookInfo* bookInfo = QBookApp::instance()->getModel()->getBookInfo(path);

        if(bookInfo)
        {
            if(!bookInfo->m_archived)
            {
                stopThumbnailGeneration();
                QBookApp::instance()->openContent(bookInfo);
            }
            else
            {
                if(m_filterMode == ELFM_SEARCH)
                {
                    closeSearchClicked();
                }

                itemLongPressed(path);
            }
        }
        else if(QBookApp::instance()->isImage(path))
        {
            QBookApp::instance()->getStatusBar()->setBusy(true);
            Screen::getInstance()->queueUpdates();
            openImage(path);
            QBookApp::instance()->getStatusBar()->setBusy(false);
            Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
            Screen::getInstance()->flushUpdates();
        }
        else if(path.toLower().endsWith(".acsm"))
        {
            processFulfillment(path);
        }
        else
        {
            QFileInfo fi(path);
            if(fi.isDir())
            {
                if(m_filterMode == ELFM_SEARCH)
                {
                    m_filterMode = ELFM_NONE;
                    openSearchPath(path);
                    closeSearchClicked();
                    return;
                }

                qDebug() << Q_FUNC_INFO << " path = " << path << ", base: " << m_basePath << ", current: " << m_currentPath;

                Screen::getInstance()->queueUpdates();

                m_currentView->stop();

                m_currentPath = path;
                m_currentPath.remove(0, m_basePath.size());// Remove the m_basePath

                // Fill data
                loadDir(path);

                // Sort
                (this->*sortCurrentDataCallback)();

                m_page = 0;

                // Set Breadcrumb
                if(m_currentPath.size())
                {
                    breadCrumb->show();
                    breadCrumb->setDirectory(m_currentPath);
                }
                else
                    breadCrumb->hide();

                m_currentView->start();

                Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
                Screen::getInstance()->flushUpdates();
            }
            else
            {
                qWarning() << "We don't know how to open this, yet";
                InfoDialog* dialog = new InfoDialog(this,tr("Document format not supported"));
                dialog->hideSpinner();
                dialog->showForSpecifiedTime();
                delete dialog;
            }
        }
    }
    else // ELICM_SELECT
    {
        const BookInfo* bookInfo = QBookApp::instance()->getModel()->getBookInfo(path);
        if(bookInfo)
        {
            if(!m_selectedBooks.contains(bookInfo->path))
            {
                m_selectedBooks.insert(bookInfo->path, bookInfo);// Select it
            }
            else
            {
                m_selectedBooks.take(bookInfo->path);// Deselect it
            }
        }
    }
}

void Library::itemLongPressed( const QString& path )
{
    qDebug() << Q_FUNC_INFO << "Path: " << path;
    const BookInfo* bookInfo = QBookApp::instance()->getModel()->getBookInfo(path);

    if(clearKeyboard())
        return;
    if(m_currentView->clearActionsMenu())
        return;

    if(bookInfo)
    {
        Screen::getInstance()->queueUpdates();
        m_currentView->getPageHandler()->hide();

        if(bookInfo->thumbnail.isEmpty())
        {
            BookInfo* book = new BookInfo(*bookInfo);
            QBookApp::instance()->generateBookCover(book);
        }

        int size, pos = 1;
        QList<const BookInfo*>::const_iterator it;
        QList<const BookInfo*>::const_iterator itEnd;

        if (isReallySearchView())
        {
            it = m_searchBooks.begin();
            itEnd = m_searchBooks.end();
            size = m_searchBooks.size();
        }
        else if(m_sourceMode == ELSM_BROWSER)
        {
            it = m_browserBooks.begin();
            itEnd = m_browserBooks.end();
            size = m_browserBooks.size();
        }
        else
        {
            it = m_books.begin();
            itEnd = m_books.end();
            size = m_books.size();
        }
        while(it != itEnd)
        {
            if((*it) == bookInfo)
                break;

            pos++;
            it++;
        }

        m_bookSummary->show();
        m_bookSummary->setBook(bookInfo, pos, size);

        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
    }
}

void Library::removeFile( const QString& path)
{
    qDebug() << Q_FUNC_INFO << path;

    m_currentView->clearActionsMenu();
    QFileInfo fi(path);
    QString removeText = tr("You are about to remove the ");

    if(fi.isDir())
        removeText += tr("folder ");
    else
        removeText += tr("file ");

    if(path.contains(Storage::getInstance()->getPublicPartition()->getMountPoint()))
        removeText += tr("from internal memory.");
    else if(Storage::getInstance()->getRemovablePartition() && path.contains(Storage::getInstance()->getRemovablePartition()->getMountPoint()))
        removeText += tr("from SD card.");
    removeText += tr("\nDo you want to continue?");
    SelectionDialog* removeDialog = new SelectionDialog(this, removeText, tr("Remove"));
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    removeDialog->exec();

    if(removeDialog->result())
    {
        bool result = false;
        if(fi.isDir())
        {
            QBookApp::instance()->getStatusBar()->setBusy(true);
            result = bqUtils::removeDir(path);
            QBookApp::instance()->getModel()->removeDir(path);

            QBookApp::instance()->getStatusBar()->setBusy(false);
            Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        }
        else
            result = QFile::remove(path);

        if(result)
        {
            InfoDialog* dialog;
            if(fi.isDir())
                dialog = new InfoDialog(this,tr("Folder correctly removed."));
            else
                dialog = new InfoDialog(this,tr("File correctly removed."));

            dialog->hideSpinner();
            Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
            dialog->showForSpecifiedTime();
            delete dialog;

            system("sync");
            changeFilterMode(m_filterMode);
        }
        else
        {
            InfoDialog* dialog;
            if(fi.isDir())
                dialog = new InfoDialog(this,tr("Remove failure, the folder can not be removed."));
            else
                dialog = new InfoDialog(this,tr("Remove failure, the file can not be removed."));

            dialog->hideSpinner();
            dialog->showForSpecifiedTime();
            delete dialog;
        }
    }
    delete removeDialog;
}

void Library::copyFile(const QString& path)
{
    qDebug() << Q_FUNC_INFO << path;

    Screen::getInstance()->queueUpdates();
    m_currentView->clearActionsMenu();

    QFileInfo fi(path);
    QString destination;

    QString copyText = tr("You are about to copy the ");
    bool isDir = false;
    if(fi.isDir())
    {
        isDir = true;
        copyText += tr("folder ");
    }
    else
        copyText += tr("file ");


    StoragePartition* partition = Storage::getInstance()->getRemovablePartition();
    if(path.contains(Storage::getInstance()->getPublicPartition()->getMountPoint()) && (!partition || !partition->isMounted())){
        InfoDialog* errorDialog = new InfoDialog(this,tr("SD card is not mounted."));
        errorDialog->hideSpinner();
        Screen::getInstance()->flushUpdates();
        errorDialog->showForSpecifiedTime();
        delete errorDialog;
        return;
    }

    if(path.contains(Storage::getInstance()->getPublicPartition()->getMountPoint()) && Storage::getInstance()->getRemovablePartition()) {
        copyText += tr("from internal memory to SD.");
        destination = Storage::getInstance()->getRemovablePartition()->getMountPoint() + "/" + fi.fileName();
    }
    else if(Storage::getInstance()->getRemovablePartition() && path.contains(Storage::getInstance()->getRemovablePartition()->getMountPoint())) {
        copyText += tr("from SD card to internal memory.");
        destination = Storage::getInstance()->getPublicPartition()->getMountPoint() + "/" + fi.fileName();
    }

    copyText += tr("\nDo you want to continue?");
    qDebug() << Q_FUNC_INFO << "destination: " << destination;
    ConfirmDialog* errorDialog = NULL;

    if((QFile::exists(destination) && isDir) || !isDir && QDir(destination).exists())
    {
        QFileInfo fileDestination(destination);
        if(!fileDestination.isDir())
            errorDialog = new ConfirmDialog(this, tr("The file already exists.\nPlease delete the file before copy it."));
        else
            errorDialog = new ConfirmDialog(this, tr("Already exists a folder with this name.\nPlease delete the file before copy it."));
    }

    if(errorDialog)
    {
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_WAITFORCOMPLETION, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
        errorDialog->showForSpecifiedTime(5000);
        delete errorDialog;
        errorDialog = NULL;
        return;
    }
    SelectionDialog* copyDialog = new SelectionDialog(this, copyText, tr("Copy"));
    m_bookSummary->hideElements();
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_WAITFORCOMPLETION, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
    copyDialog->exec();

    if(copyDialog->result())
    {
        m_powerLock->activate();
        QFile file(destination);
        InfoDialog* dialog;
        ProgressDialog* copyingFilesDialog;

        int filesToLoad = 0;
        int filesCopied = 0;
        int positivelyCopied = 0;
        bool result = false;
        if(fi.isDir())
        {
            qDebug() << Q_FUNC_INFO << "before files to copy";
            if(bqUtils::filesToCopy(path, filesToLoad))
            {
                copyingFilesDialog = new ProgressDialog(this, tr("Copying files..."));
                copyingFilesDialog->hideCancelButton();
                copyingFilesDialog->setHideBtn(false);
                copyingFilesDialog->setTextValue(false);
                copyingFilesDialog->setModal(true);
                copyingFilesDialog->show();

                QCoreApplication::flush();
                QCoreApplication::processEvents();
                connect(this, SIGNAL(fileCopied(int)), copyingFilesDialog, SLOT(setProgressBar(int)), Qt::UniqueConnection);
                qDebug() << Q_FUNC_INFO << "filesToLoad: " << filesToLoad << " filesCopied: " << filesCopied;
                result = copyDir(path, destination, filesToLoad, filesCopied, positivelyCopied);

                qDebug() << Q_FUNC_INFO << " filesCopied: " << filesCopied;
                copyingFilesDialog->hide();
                delete copyingFilesDialog;
                copyingFilesDialog = NULL;
            }
        }else
        {
            QBookApp::instance()->getStatusBar()->setBusy(true);
            result = QFile::copy(path, destination);
            QBookApp::instance()->getStatusBar()->setBusy(false);
        }

        if(result)
        {
            if(fi.isDir())
                dialog = new InfoDialog(this,tr("Folder correctly copied."),3000);
            else
                dialog = new InfoDialog(this,tr("File correctly copied."),3000);

            dialog->hideSpinner();
            dialog->showForSpecifiedTime();
            QBookApp::instance()->getModel()->addDir(destination);
            QBookApp::instance()->getModel()->loadDefaultInfo(destination);
            QtConcurrent::run(QBookApp::instance(), &QBookApp::syncModel);
            const BookInfo* bookInfo = QBookApp::instance()->getModel()->getBookInfo(path);
            if(bookInfo)
            {
                reloadModel();
                resetBookSummary(bookInfo);
            }
        }
        else
        {
            if(!fi.isDir())
            {
                if(!file.exists())
                    dialog = new InfoDialog(this,tr("Copy has failed. Please check your SD card."));
                else
                    dialog = new InfoDialog(this,tr("Copy failure, the file already exists."));
            }
            else
                dialog = new InfoDialog(this,tr("Copy has failed. Please check your SD card."));

            dialog->hideSpinner();
            dialog->showForSpecifiedTime();
            QtConcurrent::run(QBookApp::instance(), &QBookApp::syncModel);
        }
        delete dialog;
        m_powerLock->release();
    }
    delete copyDialog;
}

void Library::exportNotes(const QString& path)
{
    qDebug() << Q_FUNC_INFO << path;

    QString infoText = tr("You are about to export notes from this book ");
    const BookInfo* bookInfo = QBookApp::instance()->getModel()->getBookInfo(path);
    QFileInfo fi = QFileInfo(path);
    QString destination;

    if(bookInfo->path.contains(Storage::getInstance()->getPrivatePartition()->getMountPoint())){
         infoText += tr("from your library to internal memory.");
         destination = Storage::getInstance()->getPublicPartition()->getMountPoint() + "/" + fi.fileName();
    }
    else if(bookInfo->path.contains(Storage::getInstance()->getPublicPartition()->getMountPoint())){
        infoText += tr("from internal memory to internal memory.");
        destination = Storage::getInstance()->getPublicPartition()->getMountPoint() + "/" + fi.fileName();
    }
    else if(Storage::getInstance()->getRemovablePartition() && bookInfo->path.contains(Storage::getInstance()->getRemovablePartition()->getMountPoint())) {
         infoText += tr("from SD card to SD card.");
         destination = Storage::getInstance()->getRemovablePartition()->getMountPoint() + "/" + fi.fileName();
    }

    infoText += tr("\nDo you want to continue?");

    SelectionDialog* exportDialog = new SelectionDialog(this, infoText, tr("Export"));
    exportDialog->exec();

    if(exportDialog->result())
    {
        QBookApp::instance()->getStatusBar()->setBusy(true);
        Screen::getInstance()->queueUpdates();
        BookInfo* book = QBookApp::instance()->openDocWithoutActivateForm(bookInfo);
        QBookApp::instance()->getStatusBar()->setMenuTitle(tr("Biblioteca"));
        m_bookSummary->hideElements();
        if(QBookApp::instance()->getModel()->writeNotesToFile(book, "html", destination))
        {
            QBookApp::instance()->getStatusBar()->setBusy(false);
            Screen::getInstance()->flushUpdates();
            InfoDialog* dialog = new InfoDialog(this,tr("Notes exported correctly."));
            dialog->hideSpinner();
            Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
            dialog->showForSpecifiedTime();
            delete dialog;
        }else{
            QBookApp::instance()->getStatusBar()->setBusy(false);
            Screen::getInstance()->flushUpdates();
        }

        delete book;
    }
    delete exportDialog;
    if(m_sourceMode == ELSM_BROWSER)
        m_reloadModel = true;
}


void Library::gatherNumberOfItemsInFilters()
{
    Model* model = QBookApp::instance()->getModel();

    int allBooksCount = 0;//model->getAllBooksCount();
    int newBooksCount = 0;
    int storeBooksCount = 0;//model->getBooksInPathCount(Storage::getInstance()->getPrivatePartition()->getMountPoint());
    int activeBooksCount = 0;//model->getNowReadingBooksCount(Storage::getInstance()->getPrivatePartition()->getMountPoint());
    int sampleBooksCount = 0;//model->getSampleBooksCount();
    int purchasedBooksCount = 0;//model->getSubscriptionBooksCount();
    int subscriptionBooksCount = 0;//model->getSubscriptionBooksCount();
    int archivedStoreBooksCount = 0;//model->getArchivedBooksCount();
    int archivedPremiumBooksCount = 0;//model->getArchivedBooksCount();
    int allActiveBooksCount = 0;//model->getAllActiveBooksCount();
    int allNewBooksCount = 0;//model->getAllNewBooksCount();
    int allAlreadyReadBooksCount = 0;
    int alreadyReadBooksCount = 0;

    qDebug() << Q_FUNC_INFO << "allActiveBooksCount: " << allActiveBooksCount;
    model->getBooksCounters(Storage::getInstance()->getPrivatePartition()->getMountPoint(), allBooksCount, allActiveBooksCount, allAlreadyReadBooksCount, alreadyReadBooksCount, allNewBooksCount, newBooksCount, storeBooksCount, activeBooksCount, sampleBooksCount, purchasedBooksCount, subscriptionBooksCount, archivedStoreBooksCount, archivedPremiumBooksCount);

    m_booksFilterLayer->setAllBooksNumber(allBooksCount);

    m_booksFilterLayer->setStoreBooksNumber(storeBooksCount);
    bool enabled = archivedStoreBooksCount > 0 || archivedPremiumBooksCount > 0 || storeBooksCount > 0;
    m_booksFilterLayer->setStoreBooksBtnEnabled(enabled);

    m_booksSubFilterLayer->setAllBooksNumber(storeBooksCount);
    m_booksSubFilterLayer->setActiveBooksNumber(activeBooksCount);
    m_booksSubFilterLayer->setActiveBooksEnabled(activeBooksCount > 0);

    m_booksSubFilterLayer->setNewBooksNumber(newBooksCount);
    m_booksSubFilterLayer->setNewBooksEnabled(newBooksCount > 0);

    m_booksSubFilterLayer->setReadBooksNumber(alreadyReadBooksCount);
    m_booksSubFilterLayer->setReadBooksEnabled(alreadyReadBooksCount > 0);

    m_booksSubFilterLayer->setSampleBooksNumber(sampleBooksCount);
    m_booksSubFilterLayer->setSampleBooksEnabled(sampleBooksCount > 0);

    m_booksSubFilterLayer->setPurchasedBooksNumber(purchasedBooksCount);
    m_booksSubFilterLayer->setPurchasedBooksEnabled(purchasedBooksCount > 0);

    m_booksSubFilterLayer->setSubscriptionBooksNumber(subscriptionBooksCount);
    m_booksSubFilterLayer->setSubscriptionBooksEnabled(subscriptionBooksCount > 0);

    bool subscriptionHasExisted = QBook::settings().value("subscription/subscriptionAllowed", true).toBool();
    m_booksSubFilterLayer->setSubscriptionBooksShown(subscriptionHasExisted);

    m_booksSubFilterLayer->setStoreArchivedBooksNumber(archivedStoreBooksCount);
    m_booksSubFilterLayer->setStoreArchivedBooksEnabled(archivedStoreBooksCount > 0);

    m_booksSubFilterLayer->setPremiumArchivedBooksNumber(archivedPremiumBooksCount);
    m_booksSubFilterLayer->setPremiumArchivedBooksEnabled(archivedPremiumBooksCount > 0);

    m_allBooksSubFilterLayer->setAllBooksNumber(allBooksCount);
    m_allBooksSubFilterLayer->setAllActiveBooksNumber(allActiveBooksCount);
    m_allBooksSubFilterLayer->setNewBooksNumber(allNewBooksCount);
    m_allBooksSubFilterLayer->setReadBooksNumber(allAlreadyReadBooksCount);

    // Is there's no SD Card , disable the button
    StoragePartition* partition = Storage::getInstance()->getRemovablePartition();
    sdBtn->setEnabled(partition && partition->isMounted());

    // TODO: No se puede apañar de otra manera?
    switch(m_sourceMode)
    {
        case ELSM_ALL:
        {
            bookActionsSelectBtn->setText(m_booksFilterLayer->getAllBooksName());

            switch(m_filterMode)
            {
                case ELFM_ALL:
                    showBooksSelectBtn->setText(m_allBooksSubFilterLayer->getAllBooksName());
                    break;
                case ELFM_ALL_ACTIVE:
                    showBooksSelectBtn->setText(m_allBooksSubFilterLayer->getAllActiveBooksName());
                    break;
                case ELFM_ALL_NEW:
                    showBooksSelectBtn->setText(m_allBooksSubFilterLayer->getNewBooksName());
                    break;
                case ELFM_ALL_READ:
                    showBooksSelectBtn->setText(m_allBooksSubFilterLayer->getReadBooksName());
                    break;
                default:
                    break;
            }
        }
        break;
        case ELSM_STORE:
        {
            bookActionsSelectBtn->setText(m_booksFilterLayer->getStoreBooksName());
            switch(m_filterMode)
            {
                case ELFM_STORE_ALL:
                    showBooksSelectBtn->setText(m_booksSubFilterLayer->getAllBooksName());
                    break;
                case ELFM_STORE_NEW:
                    showBooksSelectBtn->setText(m_booksSubFilterLayer->getNewBooksName());
                    break;
                case ELFM_STORE_ACTIVE:
                    showBooksSelectBtn->setText(m_booksSubFilterLayer->getActiveBooksName());
                    break;
                case ELFM_STORE_READ:
                    showBooksSelectBtn->setText(m_booksSubFilterLayer->getReadBooksName());
                    break;
                case ELFM_STORE_SAMPLE:
                    showBooksSelectBtn->setText(m_booksSubFilterLayer->getSampleBooksName());
                    break;
                case ELFM_STORE_PURCHASED:
                    showBooksSelectBtn->setText(m_booksSubFilterLayer->getPurchasedBooksName());
                    break;
                case ELFM_STORE_SUBSCRIPTION:
                    showBooksSelectBtn->setText(m_booksSubFilterLayer->getSubscriptionBooksName());
                    break;
                case ELFM_STORE_ARCHIVED:
                    showBooksSelectBtn->setText(m_booksSubFilterLayer->getStoreArchivedBooksName());
                    break;
                case ELFM_PREMIUM_ARCHIVED:
                    showBooksSelectBtn->setText(m_booksSubFilterLayer->getPremiumArchivedBooksName());
                    break;
                default:
                    break;
            }
        }
        break;
        case ELSM_BROWSER:
            bookActionsSelectBtn->setText(m_booksFilterLayer->getBrowserFileName());
            break;
        default:
            break;
    }
}

void Library::modelChanged( const QString& path, int updateType )
{
    qDebug() << Q_FUNC_INFO;
    if(m_filterMode == ELFM_SEARCH)
        closeSearchClicked();
    gatherNumberOfItemsInFilters();

    const BookInfo* bookInfo = QBookApp::instance()->getModel()->getBookInfo(path);
    if(bookInfo)
    {
        // It's a book
        m_reloadModel = true;

        if(updateType == UPDATE_FULL)
        {
            Screen::getInstance()->queueUpdates();

            // NOTE: We want to reload the model.
            // But we can't call reload model directly because it closes the filters dialog, it resets the m_page,
            //  So we are doing here a subset of the functionality of the reload model.
            gatherNumberOfItemsInFilters();

            // Hide current view (maybe it will stop its running tasks like generating thumbnails)
            m_currentView->stop();

            // Clear data
            deleteData();

            // Fill new data
            (this->*fillNewDataCallback)();

            m_totalPages = 0;

            // Sort the books
            (this->*sortCurrentDataCallback)();
            // Show the new current view (maybe it will start running tasks like generating thumbnails)
            m_currentView->start();

            Screen::getInstance()->flushUpdates();
        }
    }
    else
    {
        // It's a dir
        if(m_filterMode == ELFM_SD)
        {
            if(Storage::getInstance()->getRemovablePartition() && !Storage::getInstance()->getRemovablePartition()->isMounted())
            {
                m_booksFilterLayer->setAllBooksChecked();
                allBooksSelected();
            }

            m_reloadModel = true;
        } else if( m_filterMode == ELFM_EDIT_COLLECTION)
            handleEditCollection(m_collection);
        else if(m_filterMode == ELFM_COLLECTIONS)
        {
            m_filterMode = ELFM_NONE;
            myCollectionsSelected();
        }
        else
            changeFilterMode(m_filterMode);
    }
}

bool Library::reloadModel()
{
    qDebug() << Q_FUNC_INFO;

    if(m_reloadModel)
    {
        m_reloadModel = false;

        gatherNumberOfItemsInFilters();

        Screen::getInstance()->queueUpdates();
        if(m_filterMode == ELFM_COLLECTIONS)
        {
            m_filterMode = ELFM_NONE;
            myCollectionsSelected();
        }
        else if(m_filterMode != ELFM_NONE && m_filterMode != ELFM_SEARCH)
            changeFilterMode(m_filterMode);
        Screen::getInstance()->flushUpdates();
        return true;
    }
    return false;
}

void Library::changeFilterMode( ELibraryFilterMode mode )
{
    PowerManagerLock* powerLock = PowerManager::getNewLock(this);
    powerLock->activate();

    m_booksFilterLayer->hide();

    m_filterMode = mode;

    // Hide current view (maybe it will stop its running tasks like generating thumbnails)
    m_currentView->stop();

    // Clear data
    deleteData();

    // Fill new data
    (this->*fillNewDataCallback)();

    if((m_sourceMode == ELSM_ALL && m_filterMode == ELFM_ALL_ACTIVE) || (m_sourceMode == ELSM_STORE && m_filterMode == ELFM_STORE_ACTIVE))
        sortByBtn->hide();
    else
        sortByBtn->show();

    if(m_books.size() == 0)
    {
        if(mode != ELFM_STORE_ALL && mode != ELFM_ALL && mode != ELFM_COLLECTION &&  m_sourceMode != ELSM_BROWSER)
        {
            gatherNumberOfItemsInFilters();
            if(m_sourceMode == ELSM_ALL)
            {
                m_allBooksSubFilterLayer->setAllBooksChecked();
                allBooksSelected();
            }else
            {
                m_booksFilterLayer->setAllBooksChecked();
                m_booksSubFilterLayer->setAllBooksChecked();
                allBooksSelected();
            }
            delete powerLock;
            return;
        }
    }

    // Activate current view: Icon or List
    if(m_viewMode == ELVM_ICON)
        m_currentView = m_currentIconView;
    else
        m_currentView = m_currentLineView;

    m_page = 0;
    m_totalPages = 0;

    // Sort the books
    (this->*sortCurrentDataCallback)();
    if(m_filterMode == ELFM_COLLECTION)
        showCollectionFilterUIStuff();
    else
        m_sortBooksByLayer->enableIndex(false);
    // Show the new current view (maybe it will start running tasks like generating thumbnails)
    m_currentView->start();
    delete powerLock;
}

void Library::changeViewMode( ELibraryViewMode mode )
{
    if(m_viewMode != mode)
    {
        m_viewMode = mode;

        m_currentView->stop();

        if(m_viewMode == ELVM_ICON)
            m_currentView = m_currentIconView;
        else
            m_currentView = m_currentLineView;

        m_currentView->start();
   }
}

void Library::fillAllData()
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->getModel()->getAllBooks(m_books);
}

void Library::fillAllStoreData()
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->getModel()->getBooksInPath(m_books, Storage::getInstance()->getPrivatePartition()->getMountPoint());
}

void Library::fillAllActiveData()
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->getModel()->getNowReadingBooks(m_books, "");
}

void Library::fillActiveData()
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->getModel()->getNowReadingBooks(m_books, Storage::getInstance()->getPrivatePartition()->getMountPoint());
}

void Library::fillAllAlreadyReadData()
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->getModel()->getAlreadyReadBooks(m_books, "");
    qDebug() << Q_FUNC_INFO << "size: " << m_books.size();
}

void Library::fillAlreadyReadData()
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->getModel()->getAlreadyReadBooks(m_books, Storage::getInstance()->getPrivatePartition()->getMountPoint());
    qDebug() << Q_FUNC_INFO << "size: " << m_books.size();
}

void Library::fillAllNewData()
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->getModel()->getNewBooks(m_books, "");
}

void Library::fillNewData()
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->getModel()->getNewBooks(m_books, Storage::getInstance()->getPrivatePartition()->getMountPoint());
}

void Library::fillSampleData()
{
    qDebug() << Q_FUNC_INFO;

    // TODO!!
    QBookApp::instance()->getModel()->getSampleBooks(m_books);
}

void Library::fillPurchasedData()
{
    qDebug() << Q_FUNC_INFO;

    // TODO!!
    QBookApp::instance()->getModel()->getPurchasedBooks(m_books, Storage::getInstance()->getPrivatePartition()->getMountPoint());
}

void Library::fillSubscriptionData()
{
    qDebug() << Q_FUNC_INFO;

    // TODO!!
    QBookApp::instance()->getModel()->getSubscriptionBooks(m_books);
}

void Library::fillStoreArchivedData()
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->getModel()->getStoreArchivedBooks(m_books);
}

void Library::fillPremiumArchivedData()
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->getModel()->getPremiumArchivedBooks(m_books);
}

void Library::fillCollectionData()
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->getModel()->getBooksInCollection(m_books, m_collection);
}

void Library::fillCurrentPathFiles()
{
    qDebug() << Q_FUNC_INFO << m_basePath << m_currentPath;

    // Load base path
    loadDir(m_basePath + m_currentPath);
}

void Library::openSearchPath(const QString &path, bool fromHomeSearch)
{
    m_currentIconView = m_iconGridViewer;
    m_currentLineView = m_lineGridViewer;
    m_sourceMode = ELSM_BROWSER;

    if(fromHomeSearch)
    {
        m_currentPath = "";
        m_booksFilterLayer->setBrowserChecked();
        if(Storage::getInstance()->getRemovablePartition() && path.contains(Storage::getInstance()->getRemovablePartition()->getMountPoint()))
        {
            m_basePath = Storage::getInstance()->getRemovablePartition()->getMountPoint();
            sdSelected(m_currentPath);
            m_filterMode = ELFM_SD;
        }
        else
        {
            m_basePath = Storage::getInstance()->getPublicPartition()->getMountPoint();
            internalMemorySelected(m_currentPath);
            m_filterMode = ELFM_INTERNAL;
        }
        changeFilterMode(m_filterMode);
    }
    showBrowserFilterUIStuff();
    itemClicked(path);
}

bool ImageNameLessThan(const QString &s1, const QString &s2)
{
    QFileInfo f1(s1);
    QFileInfo f2(s2);
    return f1.completeBaseName().toLower() < f2.completeBaseName().toLower();
}

void Library::loadDir ( const QString& path )
{
    qDebug() << Q_FUNC_INFO << " path = " << path;

    m_files.clear();
    m_images.clear();
    m_browserBooks.clear();
    QDir dir(path);
    dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files | QDir::Readable);
    dir.setSorting(QDir::DirsFirst | QDir::Name);
    dir.mkpath(Q_BOOK_THUMBNAIL_DIR);

    QStringList list = dir.entryList();

    int size = list.size();
    for (int i = 0; i < size; ++i)
    {
        QString file_path(dir.filePath(list.at(i)));//dir.absoluteFilePath(list.at(i));
        QFileInfo* fi = new QFileInfo(file_path);
        m_files.append(fi);

        if(QBookApp::instance()->isImage(file_path))
        {
            m_images.append(file_path);
        }
        else
        {
            const BookInfo* bookInfo = QBookApp::instance()->getModel()->getBookInfo(file_path);

            if(bookInfo)
            {
                m_browserBooks.append(bookInfo);
            }
        }
    }
    qSort(m_browserBooks.begin(), m_browserBooks.end(), titleLessThan);
    qSort(m_images.begin(), m_images.end(), ImageNameLessThan);
}

//void Library::updateSelectedBooks()
//{
//    qDebug() << Q_FUNC_INFO;

//    m_currentView->pause();
//    m_currentView->resume();

//    if(m_selectedBooks.size())
//    {
//        archiveBooksBtn->show();
//        deleteBooksBtn->show();
//    }
//    else
//    {
//        archiveBooksBtn->hide();
//        deleteBooksBtn->hide();
//    }
//}

void Library::showAllFilterUIStuff()
{
    qDebug() << Q_FUNC_INFO;

    // Show/Hide necessary stuff
    selectBooksLbl->hide();
    showBooksSelectBtn->show();
    searchBtn->show();
    breadCrumb->hide();
    memoryType->hide();
    allCollectionsLbl->hide();
    addColectionBtn->hide();
    showResultsLbl->hide();
    backToCollectionsBtn->hide();
    if(m_viewMode == Library::ELVM_ICON)
    {
        sortListBtn->show();
        sortIconBtn->hide();
    }else if(m_viewMode == Library::ELVM_LINE)
    {
        sortIconBtn->show();
        sortListBtn->hide();
    }else
    {
        sortIconBtn->hide();
        sortListBtn->hide();
    }
}

void Library::showStoreFilterUIStuff()
{
    qDebug() << Q_FUNC_INFO;

    // Show/Hide necessary stuff
    selectBooksLbl->hide();
    showBooksSelectBtn->show();
    searchBtn->show();
    breadCrumb->hide();
    memoryType->hide();
    allCollectionsLbl->hide();
    addColectionBtn->hide();
    showResultsLbl->hide();
    backToCollectionsBtn->hide();

    if(m_viewMode == Library::ELVM_ICON)
    {
        sortListBtn->show();
        sortIconBtn->hide();
    }else if(m_viewMode == Library::ELVM_LINE)
    {
        sortIconBtn->show();
        sortListBtn->hide();
    }else
    {
        sortIconBtn->hide();
        sortListBtn->hide();
    }
}

void Library::showMyCollectionFilterUIStuff()
{
    qDebug() << Q_FUNC_INFO;

    // Show/Hide necessary stuff
    allCollectionsLbl->setText(tr("Todas las colecciones"));
    allCollectionsLbl->show();
    addColectionBtn->show();
    searchBtn->hide();
    sortByBtn->hide();
    sortIconBtn->hide();
    sortListBtn->hide();
    memoryType->hide();
    showBooksSelectBtn->hide();
    selectBooksLbl->hide();
    breadCrumb->hide();
    memoryType->hide();
    showResultsLbl->hide();
    backToCollectionsBtn->hide();
}

void Library::showCollectionFilterUIStuff()
{
    qDebug() << Q_FUNC_INFO;

    // Show/Hide necessary stuff
    QString nameCollection = bqUtils::truncateStringToLength(tr("%1").arg(m_collection),COLLECTION_MAX_LENGTH);
    allCollectionsLbl->setText(nameCollection + tr(" (%1)").arg(m_books.size()));
    allCollectionsLbl->show();
    addColectionBtn->hide();
    sortByBtn->show();
    searchBtn->show();
    memoryType->hide();
    showBooksSelectBtn->hide();
    selectBooksLbl->hide();
    breadCrumb->hide();
    memoryType->hide();
    showResultsLbl->hide();
    backToCollectionsBtn->show();
    if(m_viewMode == Library::ELVM_ICON)
    {
        sortListBtn->show();
        sortIconBtn->hide();
    }else if(m_viewMode == Library::ELVM_LINE)
    {
        sortIconBtn->show();
        sortListBtn->hide();
    }else
    {
        sortIconBtn->hide();
        sortListBtn->hide();
    }
}

void Library::showBrowserFilterUIStuff()
{
    qDebug() << Q_FUNC_INFO;

    // Show/Hide necessary stuff
    selectBooksLbl->hide();
    showBooksSelectBtn->hide();
    breadCrumb->hide();
    searchBtn->show();
    allCollectionsLbl->hide();
    addColectionBtn->hide();
    memoryType->show();
    showResultsLbl->hide();
    backToCollectionsBtn->hide();
    if(m_viewMode == Library::ELVM_ICON)
    {
        sortListBtn->show();
        sortIconBtn->hide();
    }else if(m_viewMode == Library::ELVM_LINE)
    {
        sortIconBtn->show();
        sortListBtn->hide();
    }else
    {
        sortIconBtn->hide();
        sortListBtn->hide();
    }

    switch(m_filesSortMode)
    {
        case EFSM_NAME: m_sortBrowserBooksByLayer->setNameChecked(); break;
        case EFSM_DATE: m_sortBrowserBooksByLayer->setDateChecked(); break;
    }
}

QString Library::getCurrentPathParent() const
{
    QDir dir(m_basePath + "/" + m_currentPath);
    dir.cdUp();
    return dir.absolutePath();
}

void Library::searchLineEditClicked()
{
    qDebug() << Q_FUNC_INFO;
    if(m_currentView->clearActionsMenu())
        return;

//    QBookApp::instance()->getStatusBar()->setBusy(false);

    if(!m_userTyped)
    {
        librarySearchLineEdit->clear();
    }
    if(!m_keyboard || !m_keyboard->isVisible())
        showKeyboard();
}

void Library::keyboardPressedChar( const QString& input )
{
    if(!m_userTyped)
    {
//        QApplication::postEvent(librarySearchLineEdit, new QMouseEvent( QEvent::MouseButtonPress, searchForm->searchLineEdit->pos(), Qt::LeftButton, Qt::NoButton, Qt::NoModifier));
//        QApplication::postEvent(librarySearchLineEdit, new QMouseEvent( QEvent::MouseButtonRelease, searchForm->searchLineEdit->pos(), Qt::LeftButton, Qt::NoButton, Qt::NoModifier));

        librarySearchLineEdit->clear();
        librarySearchLineEdit->setText(input);

        m_userTyped = true;

        disconnect(m_keyboard, SIGNAL(pressedChar(const QString &)), this, SLOT(keyboardPressedChar(const QString &)));
    }
}

void Library::searchClicked()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();

    librarySearchLineEdit->show();
    bookActionsSelectBtn->hide();
    clearSearchBtn->show();
    closeSearchBtn->show();
    searchBtn->hide();
    synchronizeBtn->hide();
    storeBtn->hide();
    // TODO: Hide sortIcon y sortList
    sortBooksCont->hide();

    if(m_filterMode != ELFM_SEARCH)
        m_lastMode = m_filterMode;

    m_sourceMode = ELSM_SEARCH;
    m_filterMode = ELFM_SEARCH;

    showKeyboard();

    Screen::getInstance()->flushUpdates();
}

void Library::performSearch()
{
    qDebug() << Q_FUNC_INFO;

    // What to search
    const QString patternToSearch = librarySearchLineEdit->text();

    if(!m_userTyped || patternToSearch.trimmed().isEmpty())
        return;

    Screen::getInstance()->queueUpdates();

    QBookApp::instance()->getStatusBar()->setSpinner(true);

    QBookApp::instance()->hideKeyboard();// NOTE Just hide it, do not clear it.

    //We change the state when click in search button so always been in the Search filter.

    // Cancel previous search (Just in case)
    cancelSearch();

    deleteSearchData();

    // Set Paths
    if(Storage::getInstance()->getRemovablePartition()
            && Storage::getInstance()->getRemovablePartition()->isMounted()
            && m_filterMode == ELFM_SD)
        m_basePath = Storage::getInstance()->getRemovablePartition()->getMountPoint();

    if(m_filterMode != ELFM_SEARCH)
        m_lastMode = m_filterMode;

    // Hide elements
    breadCrumb->hide();

    // Show/Hide specific search elements
    QString str = QString(tr("Mostrando (%1) resultados")).arg(m_dirs.size() + m_files.size() + m_searchBooks.size());
    showResultsLbl->setText(str);
    showResultsLbl->show();

    // Launch it
    m_concurrentSearch = QtConcurrent::run(this, &Library::performSearchConcurrent, patternToSearch);

    b_hasSearch = true;

    Screen::getInstance()->flushUpdates();
}

void Library::searchInFilesRecursively( const QString& path, const QString& pattern )
{
    qDebug() << Q_FUNC_INFO << "Path: " << path << ", pattern: " << pattern;

    QDir dir(path);
    dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files | QDir::Readable);
    dir.setSorting(QDir::DirsFirst | QDir::Name);
    dir.mkpath(Q_BOOK_THUMBNAIL_DIR);

    QStringList list = dir.entryList();

    Model* model = QBookApp::instance()->getModel();

    int size = list.size();
    QList<const BookInfo*>  books_title;
    QList<const BookInfo*>  books_author;
    QFileInfo fi;
    for (int i = 0; i < size; ++i)
    {
        QString filePath(dir.filePath(list.at(i)));
        fi.setFile(filePath);
        if(fi.isDir())
        {
            searchInFilesRecursively(filePath, pattern);
        }

        const BookInfo* bookInfo = model->getBookInfo(filePath);
        if(filePath.contains(pattern, Qt::CaseInsensitive) && fi.isDir())
        {
            qDebug() << Q_FUNC_INFO << "Adding filePath: " << filePath;
            QFileInfo* dir = new QFileInfo(fi);
            m_dirs.append(dir);
        }
        else if(bookInfo) {
            if(bqUtils::simplify(bookInfo->title).contains(pattern, Qt::CaseInsensitive))
            {
                qDebug() << Q_FUNC_INFO << "Adding filePath: " << filePath;
                books_title.append(bookInfo);
            }
            else if(bqUtils::simplify(bookInfo->author).contains(pattern, Qt::CaseInsensitive))
            {
                qDebug() << Q_FUNC_INFO << "Adding filePath: " << filePath;
                books_author.append(bookInfo);
            }
        }
        else if(bqUtils::simplify(filePath).contains(pattern, Qt::CaseInsensitive) )
        {
            qDebug() << Q_FUNC_INFO << "Adding filePath: " << filePath;
            QFileInfo* file = new QFileInfo(fi);
            m_files.append(file);
            if(QBookApp::instance()->isImage(filePath))
                m_images.append(filePath);
        }
    }
    qSort(m_dirs.begin(), m_dirs.end(), fileNameLessThan);
    qSort(books_title.begin(), books_title.end(), titleLessThan);
    m_searchBooks.append(books_title);
    qSort(books_author.begin(), books_author.end(), authorLessThan);
    m_searchBooks.append(books_author);
    qSort(m_files.begin(), m_files.end(), fileNameLessThan);
    qSort(m_images.begin(), m_images.end(), ImageNameLessThan);
}

void Library::searchInBooks(const QString& _pattern )
{
    int size = m_books.size();
    QList<const BookInfo*>  books_title;
    QList<const BookInfo*>  books_author;
    QList<const BookInfo*>  books_path;
    const QString pattern(bqUtils::simplify(_pattern));
    for (int i = 0; i < size; ++i)
    {
        const BookInfo* bookInfo = m_books[i];
        if(bookInfo) {
            if(bqUtils::simplify(bookInfo->title).contains(pattern, Qt::CaseInsensitive) )
            {
                qDebug() << Q_FUNC_INFO << "Adding filePath: " << bookInfo->title;
                books_title.append(bookInfo);
            }
            else if(bqUtils::simplify(bookInfo->author).contains(pattern, Qt::CaseInsensitive) )
            {
                qDebug() << Q_FUNC_INFO << "Adding filePath: " << bookInfo->title;
                books_author.append(bookInfo);
            }
            else if(bqUtils::simplify(bookInfo->path).contains(pattern, Qt::CaseInsensitive) )
            {
                qDebug() << Q_FUNC_INFO << "Adding filePath: " << bookInfo->title;
                books_path.append(bookInfo);
            }
        }
    }
    qSort(books_title.begin(), books_title.end(), titleLessThan);
    m_searchBooks.append(books_title);
    qSort(books_author.begin(), books_author.end(), authorLessThan);
    m_searchBooks.append(books_author);
    qSort(books_path.begin(), books_path.end(), pathLessThan);
    m_searchBooks.append(books_path);
}

void Library::performSearchConcurrent( const QString& pattern )
{
    qDebug() << Q_FUNC_INFO;

    // Search. Búsqueda recursiva
    if(m_lastMode < ELFM_LIBRARY_MODE || m_lastMode == ELFM_COLLECTION)
        searchInBooks(pattern);
    else if (m_lastMode == ELFM_INTERNAL)
        searchInFilesRecursively(Storage::getInstance()->getPublicPartition()->getMountPoint() + m_currentPath, pattern);
    else if(Storage::getInstance()->getRemovablePartition() && Storage::getInstance()->getRemovablePartition()->isMounted())
        searchInFilesRecursively(Storage::getInstance()->getRemovablePartition()->getMountPoint() + m_currentPath, pattern);

    emit searchFinished();
}

void Library::handleSearchFinished()
{
    qDebug() << Q_FUNC_INFO << "Results: " << m_files.size() + m_searchBooks.size() + m_dirs.size();

    // Present the results
    QBookApp::instance()->getStatusBar()->setSpinner(false);
    QString str;

    if (m_lastMode < ELFM_LIBRARY_MODE)
        str = QString(tr("Mostrando (%1) resultados")).arg(m_searchBooks.size());
    else
        str = QString(tr("Mostrando (%1) resultados")).arg(m_files.size() + m_searchBooks.size() + m_dirs.size());
    showResultsLbl->setText(str);

    // Activate current view
    // Hide current View (TODO: Si es la misma view de la búsqueda?)
    if(m_currentView)
        m_currentView->stop();
    m_currentView = m_lineGridViewer;

    m_page = 0;
    m_totalPages = 0;

    // Show the new current view (maybe it will start running tasks like generating thumbnails)
    m_currentView->start();
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void Library::cancelSearch()
{
    qDebug() << Q_FUNC_INFO;
    if(!m_concurrentSearch.isFinished())
    {
        m_concurrentSearch.cancel();
        m_concurrentSearch.waitForFinished();

        QBookApp::instance()->getStatusBar()->setSpinner(false);
    }
}

void Library::searchClear()
{
    librarySearchLineEdit->clear();
    librarySearchLineEdit->setText("");
    librarySearchLineEdit->setFocus();
}

void Library::closeSearchClicked( )
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->lockScreen();
    cancelSearch();

    deleteSearchData();

    librarySearchLineEdit->setText(m_initialSearchLineEditText);
    librarySearchLineEdit->hide();
    setFocus();

    // Show/Hide specific search elements
    bookActionsSelectBtn->show();
    clearSearchBtn->hide();
    closeSearchBtn->hide();
    searchBtn->show();
    synchronizeBtn->show();
    storeBtn->show();
    // TODO: Show sortIcon y sortList
    sortBooksCont->show();
    showResultsLbl->hide();
    m_userTyped = false;
    b_hasSearch = false;
    clearKeyboard();
    qDebug() << Q_FUNC_INFO << m_books.size() << m_files.size() << m_searchBooks.size();

    switch(m_lastMode)
    {
    case ELFM_ALL_ACTIVE:
        myBooksSelected();
        allActiveBooksSelected();
        break;
    case ELFM_ALL_NEW:
        myBooksSelected();
        allNewBooksSelected();
        break;
    case ELFM_ALL_READ:
        myBooksSelected();
        allReadBooksSelected();
        break;
    case ELFM_STORE_ALL:
        storeBooksSelected();
        break;
    case ELFM_STORE_ACTIVE:
        storeBooksSelected();
        activeBooksSelected();
        break;
    case ELFM_STORE_NEW:
        storeBooksSelected();
        newBooksSelected();
        break;
    case ELFM_STORE_READ:
        storeBooksSelected();
        readBooksSelected();
        break;
    case ELFM_STORE_SAMPLE:
        storeBooksSelected();
        sampleBooksSelected();
        break;
    case ELFM_STORE_PURCHASED:
        storeBooksSelected();
        purchasedBooksSelected();
        break;
    case ELFM_STORE_SUBSCRIPTION:
        storeBooksSelected();
        subscriptionBooksSelected();
        break;
    case ELFM_STORE_ARCHIVED:
        storeBooksSelected();
        archivedStoreBooksSelected();
        break;
    case ELFM_PREMIUM_ARCHIVED:
        storeBooksSelected();
        archivedPremiumBooksSelected();
        break;
    case ELFM_COLLECTION:
        selectCollection(m_collection);
        break;
    case ELFM_SD:
        browserFileSelected(m_currentPath);
        sdSelected(m_currentPath);
        break;
    case ELFM_INTERNAL:
        browserFileSelected(m_currentPath);
        internalMemorySelected(m_currentPath);
        break;
    default:
        myBooksSelected();
        break;
    }
    m_lastMode = ELFM_NONE;

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->releaseScreen();
}

void Library::keyReleaseEvent( QKeyEvent* event )
{
    qDebug() << Q_FUNC_INFO << (uint)event->key() << QApplication::focusWidget();

    switch(event->key())
    {
        case QBook::QKEY_BACK:
        {
            event->accept();
            if(anyPopUpShown())
            {
                Screen::getInstance()->queueUpdates();
                clearKeyboard();
                hideAllElements();
                Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
                Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
                Screen::getInstance()->flushUpdates();
            }
            else
            {
                if(m_filterMode == ELFM_SEARCH)
                {
                    closeSearchClicked();
                }
                else
                {
                    QBookApp::instance()->goToHome();
                }
            }
            return;
        }
    }

    QBookForm::keyReleaseEvent(event);
}

void Library::showKeyboard()
{
    // Show keyboard
    m_keyboard = QBookApp::instance()->showKeyboard(tr("Buscar"));
    m_keyboard->handleMyQLineEdit(librarySearchLineEdit);
    connect(m_keyboard, SIGNAL(actionRequested()), this, SLOT(performSearch()));
    connect(m_keyboard, SIGNAL(newLinePressed()),  this, SLOT(performSearch()));
    connect(m_keyboard, SIGNAL(pressedChar(const QString &)), this, SLOT(keyboardPressedChar(const QString &)));
    //
}


bool Library::clearKeyboard()
{
    if(m_keyboard)
    {
        if(m_keyboard->isVisible()) {
            m_keyboard->hide();
            return true;
        }

        m_keyboard = NULL;

        // TODO: Desconectar señales del keyboard??
        librarySearchLineEdit->setText(m_initialSearchLineEditText);
        m_userTyped = false;

    }
    return false;
}

bool Library::hideKeyboard()
{
    if(m_keyboard && m_keyboard->isVisible())
    {
        m_keyboard->hide();
        m_keyboard = NULL;

        return true;
    }
    return false;
}

bool Library::anyPopUpShown() const
{
    return ( (m_keyboard && m_keyboard->isVisible()) || m_booksFilterLayer->isVisible() || m_bookSummary->isVisible() || m_previewImage->isVisible());
}

void Library::stopThumbnailGeneration()
{
    qDebug() << Q_FUNC_INFO;
    if(m_currentView == m_iconGridViewer)
        m_iconGridViewer->pauseThumbnailGeneration();
}

void Library::resumeThumbnailGeneration()
{
    qDebug() << Q_FUNC_INFO;
    if(m_currentView == m_iconGridViewer)
        m_iconGridViewer->resumeThumbnailGeneration();
}

#ifndef HACKERS_EDITION
int Library::setUpUnarchiving()
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->requestConnection();

    if(!ConnectionManager::getInstance()->isConnected())
    {
        qDebug() << Q_FUNC_INFO << "Connection Manager not connected.";
        connectUnarchiveWifiObserver();
        connect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disconnectUnarchiveWifiObserver()), Qt::UniqueConnection);
        connect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(cancelUnarchiving()), Qt::UniqueConnection);
        return -1;
    }

    Screen::getInstance()->queueUpdates();

    if (!m_dialogSync)
    {
        m_dialogSync = new ProgressDialog(this,tr("Sincronizando... Por favor, espera unos minutos"));
        m_dialogSync->setTextValue(false);
        connect(m_dialogSync,SIGNAL(cancel()), this, SLOT(displaySyncCancel()), Qt::UniqueConnection);
    }

    connect(QBookApp::instance()->getSyncHelper(), SIGNAL(ProgressChanged(int)), m_dialogSync, SLOT(setProgressBar(int)), Qt::UniqueConnection);
    m_dialogSync->setHideBtn(false);
    m_dialogSync->setProgressBar(0);
    m_dialogSync->setModal(true);
    m_dialogSync->show();

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    if(QBookApp::instance()->isSynchronizing())
    {
        return 0;
    }

    return 1;
}
#endif

void Library::hideDialog()
{
    qDebug() << Q_FUNC_INFO;
    if(m_dialogSync)
    {
        Screen::getInstance()->queueUpdates();
        m_dialogSync->setModal(false);
        m_dialogSync->hide();
        synchronizeBtn->setEnabled(true);
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
    }
}

void Library::connectedToPc(bool value)
{
    if(value && (m_filterMode == ELFM_INTERNAL || m_filterMode == ELFM_SD))
        pressedBreadCrumb("");
}

void Library::reportedStoreLink()
{
    gatherNumberOfItemsInFilters();
}

/*static*/ void Library::fromCover2Thumbnail(const QString& coverPath){
    qDebug() << Q_FUNC_INFO << coverPath;

    QImage fullSizeCover(coverPath);
    if(fullSizeCover.isNull()){
        qDebug() << Q_FUNC_INFO << "ERROR at" << coverPath;
        return;
    }

    QSize homeThumbnailSize = QBookApp::instance()->getHomeThumbnailSize();

    fullSizeCover = fullSizeCover.scaled(homeThumbnailSize.width(),homeThumbnailSize.height(), Qt::KeepAspectRatioByExpanding);
    fullSizeCover.save(coverPath,"jpg");
}

void Library::processFulfillment( const QString& path )
{
#ifndef DISABLE_ADOBE_SDK
    qDebug() << Q_FUNC_INFO << path << ", linked: " << AdobeDRM::getInstance()->isLinked();

    if(!AdobeDRM::getInstance()->isLinked())
        return;

    QFile file(path);
    if(file.open(QIODevice::ReadOnly))
    {
        m_waitingDialog = new InfoDialog(this,tr("Your book is being downloaded. Please wait."));
        m_waitingDialog->show();

        AdobeDRM* drm_hdlr = AdobeDRM::getInstance();
        connect(drm_hdlr, SIGNAL(fulfillmentOK(QString,bool,QString)),  this, SLOT(handleFulfillmentDone(QString,bool,QString)));
        connect(drm_hdlr, SIGNAL(fulfillmentKO(QString)),               this, SLOT(handleFulfillmentError(QString)));

        drm_hdlr->fulfillment(file);

        file.close();
    }
#endif
}

void Library::handleFulfillmentDone(QString /*fulfillmentId*/, bool /*bReturnable*/, QString /*fulfillmentDocPath*/)
{
    qDebug() << Q_FUNC_INFO;
#ifndef DISABLE_ADOBE_SDK

    AdobeDRM* drm_hdlr = AdobeDRM::getInstance();
    disconnect(drm_hdlr, SIGNAL(fulfillmentOK(QString,bool,QString)),   this, SLOT(handleFulfillmentDone(QString,bool,QString)));
    disconnect(drm_hdlr, SIGNAL(fulfillmentKO(QString)),                this, SLOT(handleFulfillmentError(QString)));

    if(m_waitingDialog != NULL)
    {
        m_waitingDialog->close();
        delete m_waitingDialog;
        m_waitingDialog = NULL;
    }

    QPointer<InfoDialog> dialog = new InfoDialog(this,tr("Your book is now available at your Digital Editions folder"));

    QtConcurrent::run(QBookApp::instance(), &QBookApp::syncModel);// Sync before adding
    QBookApp::instance()->getModel()->addDir(qgetenv("ADOBE_DE_DOC_FOLDER"));
//    QtConcurrent::run(QBookApp::instance(), &QBookApp::syncModel);

//    m_fulfillmentId = fulfillmentId;
//    m_bReturnable = bReturnable;
//    m_fulfillmentDocPath = fulfillmentDocPath;

    dialog->showForSpecifiedTime();
    delete dialog;
#endif
}

void Library::handleFulfillmentError( QString errorMsg )
{
#ifndef DISABLE_ADOBE_SDK
    qDebug() << Q_FUNC_INFO << errorMsg;

    AdobeDRM* drm_hdlr = AdobeDRM::getInstance();
    disconnect(drm_hdlr, SIGNAL(fulfillmentOK(QString,bool,QString)),   this, SLOT(handleFulfillmentDone(QString,bool,QString)));
    disconnect(drm_hdlr, SIGNAL(fulfillmentKO(QString)),                this, SLOT(handleFulfillmentError(QString)));

    if(m_waitingDialog != NULL)
    {
        m_waitingDialog->close();
        delete m_waitingDialog;
        m_waitingDialog = NULL;
    }

    QString errorInfo;
    if(errorMsg == "E_IO_TEMP_FILE_WRITE")
        errorInfo = tr("Not enough space in the device");
    else if(errorMsg == "E_LIC_WRONG_DEVICE_TYPE")
        errorInfo = tr("Check your license activation");
    else  if(errorMsg == "E_ADEPT_REQUEST_EXPIRED")
        errorInfo = tr("Download request has expired");
    QPointer<InfoDialog> dialog = new InfoDialog(this, tr("Error while fulfilling book. ") + errorInfo);
    dialog->showForSpecifiedTime();
    delete dialog;
#endif
}

void Library::viewActiveBooks()
{
    qDebug() << Q_FUNC_INFO;

    closeSearchClicked();
    allBooksSelected();
    allActiveBooksSelected();
    m_allBooksSubFilterLayer->setAllActiveBooksChecked();
    m_booksFilterLayer->setAllBooksChecked();
}

void Library::handleBooksSortModeUI()
{
    if((m_filterMode == ELFM_ALL_NEW && m_sourceMode == ELSM_ALL)
        || (m_filterMode == ELFM_STORE_NEW && m_sourceMode == ELSM_STORE))
        m_sortBooksByLayer->enableRecent(false);
    else
        m_sortBooksByLayer->enableRecent(true);

    if(m_filterMode == ELFM_ALL_NEW || m_filterMode == ELFM_STORE_NEW)
    {
        switch(m_newBooksSortMode)
        {
            case ELSM_TITLE:
                sortByBtn->setText(m_sortBooksByLayer->getTitleSortName());
                m_sortBooksByLayer->setTitleChecked();
            break;

            case ELSM_AUTHOR:
                sortByBtn->setText(m_sortBooksByLayer->getAuthorSortName());
                m_sortBooksByLayer->setAuthorChecked();
            break;
        }
    }
    else
    {
        switch(m_booksSortMode)
        {
            case ELSM_DATE:
                sortByBtn->setText(m_sortBooksByLayer->getRecentSortName());
                m_sortBooksByLayer->setRecentChecked();
            break;

            case ELSM_TITLE:
                sortByBtn->setText(m_sortBooksByLayer->getTitleSortName());
                m_sortBooksByLayer->setTitleChecked();
            break;

            case ELSM_AUTHOR:
                sortByBtn->setText(m_sortBooksByLayer->getAuthorSortName());
                m_sortBooksByLayer->setAuthorChecked();
            break;
        }
    }
}

void Library::setBooksSortModeCallback()
{
    if(m_filterMode == ELFM_ALL_NEW || m_filterMode == ELFM_STORE_NEW)
    {
        switch(m_newBooksSortMode)
        {
            case ELSM_TITLE:
                sortBooksDataCallback = &Library::sortBooksByTitle;
            break;

            case ELSM_AUTHOR:
                sortBooksDataCallback = &Library::sortBooksByAuthor;
            break;
        }
    }
    else if(m_filterMode != ELFM_COLLECTION)
    {
        switch(m_booksSortMode)
        {
            case ELSM_DATE:
                sortBooksDataCallback = &Library::sortBooksByDate;
            break;

            case ELSM_TITLE:
                sortBooksDataCallback = &Library::sortBooksByTitle;
            break;

            case ELSM_AUTHOR:
                sortBooksDataCallback = &Library::sortBooksByAuthor;
            break;
        }
    }
    else
        sortBooksDataCallback = &Library::sortBooksByIndex;

    sortCurrentDataCallback = sortBooksDataCallback;
}

QString Library::modifyStyleSheet(QString styleSheet, QString styleToModify)
{
    if(styleToModify.isEmpty()) return styleSheet; // Nothing to do.

    if(styleSheet.isEmpty()) return styleToModify; // Nothing to do.

    QStringList styleToModifyPieces = styleToModify.split(":");

    if(styleToModifyPieces.count() != 2) return styleSheet; // This function only support simple styles.

    if(!styleSheet.contains(styleToModifyPieces[0])) return (styleSheet + ";" + styleToModify); // Add style.

    QStringList styleList = styleSheet.split(";");

    QStringList::iterator it = styleList.end();

    QStringList auxStringList;
    int i = styleList.size();

    while(it != styleList.begin())
    {
        --it; --i;
        auxStringList = it->split(":");
        if(auxStringList[0] == styleToModifyPieces[0])
        {
            styleList.replace(i, styleToModify);
            return styleList.join(";");
        }
    }

    if(auxStringList[0] != styleToModifyPieces[0]) return QString(); // ERROR!

}

void Library::setReloadModel(bool value )
{
    m_reloadModel = value || m_reloadModel;
}

void Library::setupLibrary()
{
    qDebug() << Q_FUNC_INFO;
    gatherNumberOfItemsInFilters();
    changeFilterMode(m_filterMode);
}

void Library::handleEditCollection(const QString & collection)
{
    qDebug() << Q_FUNC_INFO << collection;

    Screen::getInstance()->queueUpdates();

    if(m_filterMode != ELFM_EDIT_COLLECTION)
        m_filterMode = ELFM_EDIT_COLLECTION;
    m_editCollection->setup(collection);
    m_editCollection->show();

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void Library::handleDeleteCollection(const QString & collection)
{
    qDebug() << Q_FUNC_INFO << collection;
    m_collection = collection;
    fillNewDataCallback = &Library::fillCollectionData;

    QString deleteCollectionText = tr("Are you sure you want to delete %1 collection?\nThe books in the collection will not be deleted.").arg(collection);
    SelectionDialog* deleteCollection = new SelectionDialog (this, deleteCollectionText, tr("Remove"));
    deleteCollection->exec();
    if(deleteCollection->result())
    {
        m_currentView->pause();
        // Clear data
        deleteData();

        // Fill new data
        (this->*fillNewDataCallback)();
        QList<const BookInfo*>::const_iterator it = m_books.constBegin();
        QList<const BookInfo*>::const_iterator itEnd = m_books.constEnd();
        for(; it != itEnd; ++it)
        {
            BookInfo* book = new BookInfo (*(*it));
            book->removeCollection(collection);
            QBookApp::instance()->getModel()->updateBook(book);
        }
        QBookApp::instance()->getModel()->removeCollection(m_collection);
        m_lineGridCollection->setCollections(QBookApp::instance()->getModel()->getCollections());
        m_currentView->start();
    }
}

void Library::hideEditCollection()
{
    Screen::getInstance()->queueUpdates();
    if(m_editCollection->isVisible())
        m_editCollection->hide();
    if(!m_editCollection->isFromBookSummary() || m_editCollection->collectionSaved() && !b_isFromViewer)
        myCollectionsSelected();
    else
    {
        switch(m_sourceMode)
        {
        case ELSM_ALL:
            m_booksFilterLayer->setAllBooksChecked();
            break;
        case ELSM_STORE:
            m_booksFilterLayer->setStoreBooksChecked();
            break;
        case ELSM_BROWSER:
            m_booksFilterLayer->setBrowserChecked();
            break;
        }
    }
    if(b_isFromViewer)
    {
        setFromViewer(false);
        emit returnToViewer();
    }
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void Library::selectCollection(const QString& collection)
{
    qDebug() << Q_FUNC_INFO;

    if(m_filterMode != ELFM_COLLECTION && collection != "")
    {
        QBookApp::instance()->getStatusBar()->setSpinner(true);

        Screen::getInstance()->queueUpdates();

        m_collectionsIdx = m_page;

        handleBooksSortModeUI();

        m_currentIconView = m_iconGridViewer;
        m_currentLineView = m_lineGridViewer;
        m_collection = collection;
        fillNewDataCallback = &Library::fillCollectionData;
        // Sort mode
        sortByBtn->setText(m_sortBooksByLayer->getIndexSortName());
        m_sortBooksByLayer->enableIndex(true);
        m_sortBooksByLayer->setIndexChecked();
        sortCurrentDataCallback = &Library::sortBooksByIndex;
        changeFilterMode(ELFM_COLLECTION);
        qSort(m_books.begin(), m_books.end(),sortByCollectionIndex);
        showCollectionFilterUIStuff();

        QBookApp::instance()->getStatusBar()->setSpinner(false);

        hideAllElements();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
    }
    else
        hideAllElements();
}

bool Library::isCurrentPathEmpty () const
{
    qDebug() << Q_FUNC_INFO;
    if(m_filterMode == ELFM_INTERNAL || m_filterMode == ELFM_SD || (m_filterMode == ELFM_SEARCH && !b_hasSearch))
        return m_currentPath.size() == 0;
    else
        return true;
}

void Library::openImage( const QString& path)
{

    Screen::getInstance()->queueUpdates();
    QList<QString>::const_iterator it = m_images.begin();
    QList<QString>::const_iterator itEnd = m_images.end();
    int pos = 1;
    while(it != itEnd)
    {
        if((*it) == path)
            break;

        pos++;
        it++;
    }

    m_currentView->getPageHandler()->hide();
    m_previewImage->setup(path,pos, m_images.size());
    m_previewImage->show();

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_WAITFORCOMPLETION, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void Library::hideImage()
{
    qDebug() << Q_FUNC_INFO;
    if(!m_previewImage->isVisible()) return;

    Screen::getInstance()->queueUpdates();

    m_currentView->getPageHandler()->show();
    m_previewImage->hide();

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void Library::previousImageRequest(const QString& currentImagePath)
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(true);
    Screen::getInstance()->flushUpdates();

    QString path = "";
    int pos = 0;
    QList<QString>::const_iterator it = m_images.begin();
    QList<QString>::const_iterator itEnd = m_images.end();

    while(it != itEnd)
    {
        qDebug() << Q_FUNC_INFO << " (*it):" << (*it);
        if((*it) == currentImagePath)
        {
            if(pos == 0)
                path = (*--itEnd);
            else
                path = (*--it);
            break;
        }
        ++pos;
        ++it;
    }
    openImage(path);
    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(false);
    Screen::getInstance()->flushUpdates();
}

void Library::nextImageRequest(const QString& currentImagePath)
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(true);
    Screen::getInstance()->flushUpdates();

    QString path = "";
    int pos = 0;
    QList<QString>::const_iterator itInitial = m_images.begin();
    QList<QString>::const_iterator it = m_images.begin();
    QList<QString>::const_iterator itEnd = m_images.end();

    while(it != itEnd)
    {
        qDebug() << Q_FUNC_INFO << " (*it):" << (*it);
        if((*it) == currentImagePath)
        {
            if(pos == m_images.size() - 1)
                path = (*itInitial);
            else
                path = (*++it);
            break;
        }
        pos++;
        it++;
    }
    openImage(path);
    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(false);
    Screen::getInstance()->flushUpdates();
}

void Library::setImageList(QList<QString> imagesList)
{
    qDebug() << Q_FUNC_INFO;
    m_images.clear();
    m_reloadModel = false;
    m_images = imagesList;
}

bool Library::copyDir(const QString &srcPath, const QString &dstPath, int& filesToCopy, int& filesCopied, int& positivelyCopied)
{
    qDebug() << Q_FUNC_INFO << " filesToCopy:" << filesToCopy << " filesCopied: " << filesCopied;

    QDir parentDstDir(QFileInfo(dstPath).path());

    if(parentDstDir.exists(QFileInfo(dstPath).fileName()))
    {
        qDebug() << Q_FUNC_INFO << "folder already exits. Do nothing and continue";
    }
    else if(!parentDstDir.mkdir(QFileInfo(dstPath).fileName()))
    {
        qDebug() << Q_FUNC_INFO << "cannot create the folder";
        return false;
    }

    QDir srcDir(srcPath);
    foreach(const QFileInfo &info, srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
    {
        QString srcItemPath = srcPath + "/" + info.fileName();
        QString dstItemPath = dstPath + "/" + info.fileName();
        if (info.isDir())
        {
            if(!copyDir(srcItemPath, dstItemPath, filesToCopy, filesCopied, positivelyCopied))
            {
                qDebug() << Q_FUNC_INFO << "cannot copy the folder";
                return false;
            }
        }
        else if (info.isFile())
        {
            qDebug() << Q_FUNC_INFO << "fileName" << info.fileName();
            if(QFile::exists(dstItemPath))
            {
                qDebug() << Q_FUNC_INFO << "folder already exits. Do nothing and continue";
                filesCopied++;
                int progress = ((100 * (float)filesCopied) /  filesToCopy);
                emit fileCopied(progress);
            }
            else if (!QFile::copy(srcItemPath, dstItemPath))
            {
                qDebug() << Q_FUNC_INFO << "cannot copy:" << srcItemPath << " to: " << dstItemPath;
                return false;
            }
            else
            {
                positivelyCopied++;
                filesCopied++;
                int progress = ((100 * (float)filesCopied) /  filesToCopy);
                emit fileCopied(progress);
            }
        }
        else
            qDebug() << Q_FUNC_INFO << "Unhandled item" << info.filePath() << "in cpDir";
    }

    return true;
}

void Library::createNewCollection(const BookInfo* bookToAdd)
{
    qDebug() << Q_FUNC_INFO << bookToAdd->title;
    m_booksFilterLayer->setCollectionsChecked();
    hideAllElements();
    QString collection = "";
    m_editCollection->setup(collection, bookToAdd);
    m_editCollection->show();
    m_editCollection->checkBookToAdd(bookToAdd);
}

void Library::previousBookRequest(const BookInfo* currentBook)
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(true);
    Screen::getInstance()->flushUpdates();

    BookInfo* book;
    int pos = 0;
    QList<const BookInfo*>::const_iterator it;
    QList<const BookInfo*>::const_iterator itEnd;

    if (isReallySearchView())
    {
        it = m_searchBooks.begin();
        itEnd = m_searchBooks.end();
    }
    else if(m_sourceMode == ELSM_BROWSER)
    {
        it = m_browserBooks.begin();
        itEnd = m_browserBooks.end();
    }
    else
    {
        it = m_books.begin();
        itEnd = m_books.end();
    }
    while(it != itEnd)
    {
        qDebug() << Q_FUNC_INFO << " (*it):" << (*it);
        if((*it) == currentBook)
        {
            if(pos == 0)
                book = new BookInfo(*(*--itEnd));
            else
                book = new BookInfo(*(*--it));
            break;
        }
        ++pos;
        ++it;
    }
    if(book != currentBook)
        itemLongPressed(book->path);
    delete book;
    book = NULL;
    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(false);
    Screen::getInstance()->flushUpdates();
}

void Library::nextBookRequest(const BookInfo* currentBook)
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(true);
    Screen::getInstance()->flushUpdates();

    BookInfo* book = NULL;
    int size, pos = 0;
    QList<const BookInfo*>::const_iterator itInitial;
    QList<const BookInfo*>::const_iterator it;
    QList<const BookInfo*>::const_iterator itEnd;

    if (isReallySearchView())
    {
        itInitial = m_searchBooks.begin();
        it = m_searchBooks.begin();
        itEnd = m_searchBooks.end();
        size = m_searchBooks.size();
    }
    else if(m_sourceMode == ELSM_BROWSER)
    {
        itInitial = m_browserBooks.begin();
        it = m_browserBooks.begin();
        itEnd = m_browserBooks.end();
        size = m_browserBooks.size();
    }
    else
    {
        itInitial = m_books.begin();
        it = m_books.begin();
        itEnd = m_books.end();
        size = m_books.size();
    }

    if (size == 0)
    {
        summaryHiding();
        QBookApp::instance()->getStatusBar()->setSpinner(false);
        return;
    }
    while(it != itEnd)
    {
        qDebug() << Q_FUNC_INFO << " (*it):" << (*it);
        if((*it) == currentBook)
        {
            if(pos == size - 1)
                book = new BookInfo(*(*itInitial));
            else
                book = new BookInfo(*(*++it));
            break;
        }
        pos++;
        it++;
    }
    if(!book)
    {
        int offset = m_bookSummary->getCurrentBookOffset() - 1;
        if(offset >= size)
            offset = 0;
        if (isReallySearchView())
            book = new BookInfo(*m_searchBooks[offset]);
        else if(m_sourceMode == ELSM_BROWSER)
            book = new BookInfo(*m_browserBooks[offset]);
        else
            book = new BookInfo(*m_books[offset]);
    }
    if(book != currentBook)
        itemLongPressed(book->path);
    delete book;
    book = NULL;
    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(false);
    Screen::getInstance()->flushUpdates();
}

void Library::resetBookSummary(const BookInfo* bookInfo )
{
    reloadModel();
    Screen::getInstance()->queueUpdates();
    m_bookSummary->hideElements();
    QList<const BookInfo*>::const_iterator it;
    QList<const BookInfo*>::const_iterator itEnd;

    int size, pos = 1;
    if (isReallySearchView())
    {
        it = m_searchBooks.begin();
        itEnd = m_searchBooks.end();
        size = m_searchBooks.size();
    }
    else if(m_sourceMode == ELSM_BROWSER)
    {
        it = m_browserBooks.begin();
        itEnd = m_browserBooks.end();
        size = m_browserBooks.size();
    }
    else
    {
        it = m_books.begin();
        itEnd = m_books.end();
        size = m_books.size();
    }
    if(size == 0)
    {
        summaryHiding();
        Screen::getInstance()->flushUpdates();
        return;
    }
    while(it != itEnd)
    {
        if((*it) == bookInfo)
            break;

        pos++;
        it++;
    }
    if(pos > size)
        nextBookRequest(bookInfo);
    else
        m_bookSummary->setBook(bookInfo, pos, size);
    m_bookSummary->show();
    Screen::getInstance()->flushUpdates();
}
