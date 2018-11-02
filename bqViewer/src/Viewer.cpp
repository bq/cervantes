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

#include "Viewer.h"

#ifndef DISABLE_ADOBE_SDK
#include "QAdobeDocView.h"
#endif
#include "PowerManager.h"
#include "PowerManagerDefs.h"
#include "MouseFilter.h"
#include "InfoDialog.h"
#include "Keyboard.h"
#include "QBookApp.h"
#include "Screen.h"
#include "Model.h"
#include "SelectionDialog.h"
#include "ConfirmDialog.h"
#include "QBook.h"
#include "BookInfo.h"
#include "Storage.h"
#include "EpubMetaDataExtractor.h"
#include "Fb2MetaDataExtractor.h"
#include "MobiMetaDataExtractor.h"
#include "Library.h"

#include "SettingsReaderMenu.h"
#include "SettingsReaderPageTurning.h"
#include "bqUtils.h"

#include "ViewerSearchPopup.h"
#include "ViewerAppearancePopup.h"
#include "ViewerGotoPopup.h"
#include "ViewerMenu.h"
#include "ViewerBookSummary.h"
#include "ViewerContentsPopup.h"
#include "ViewerMarkHandler.h"
#include "ViewerAnnotationsList.h"
#include "ViewerDictionary.h"
#include "ViewerStepsManager.h"

#include <QDebug>
#include <QWidget>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QTimer>
#include <QBasicTimer>
#include <QPoint>
#include <QRegExp>
#include <QtCore/qmath.h>
#include <QPainter>
#include <QDir>
#include <QPixmap>
#include <Qt>
#include <QImageReader>

#define STRING_MAX_LENGTH              30
#define STRING_MAX_LENGTH_LANDSCAPE    20

#define MAX_SECS_FOR_STEP 300
#define MAX_MSECS_FOR_STEP 300 * 1000
#define MAX_SCREEN_STEPS 5

#define TIME_SHOW_SUBSCRIPTION_DIALOG 60000

Viewer::Viewer(QWidget* parent)
    : QBookForm(parent)
    , i_fontType(-1)
    , i_refreshCounter(1)
    , b_hiliMode(false)
    , b_reloading(false)
    , b_opening(false)
    , b_indexReady(false)
    , currentChapter(false)
    , m_errorAfterLoading(false)
    , m_docView(NULL)
    , m_gotoPopup(NULL)
    , m_currentWidget(NULL)
    , m_currentMenuPopup(NULL)
    , m_bookInfo(NULL)
    , timePerView(0)
    , timeGone()
    , currentView(0)
    , initialStepPos(0)
    , finalStepPos(0)
    , mSecsNeedToReadStep(0)
    , initialStepTimestamp(0)
    , finalStepTimestamp(0)
    , b_searchigWikipedia(false)
{

    qDebug() << Q_FUNC_INFO;
    setupUi(this);

    // Applying styles
    QFile fileSpecific(":/res/viewer_styles.qss");
    QFile fileCommons(":/res/viewer_styles_generic.qss");
    fileSpecific.open(QFile::ReadOnly);
    fileCommons.open(QFile::ReadOnly);

    QString styles = QLatin1String(fileSpecific.readAll() + fileCommons.readAll());
    setStyleSheet(styles);

    // Set up textBody
    QVBoxLayout* tbLayout = new QVBoxLayout(textBody);
    tbLayout->setMargin(0);
    tbLayout->setSpacing(0);
    textBody->setLayout(tbLayout);

    // Install Mouse filter
    m_pMouseFilter = new MouseFilter(this); // TODO: MOVE TO QBOOKAPP to make it global
    this->installEventFilter(m_pMouseFilter);

    // Build Viewer menu
    m_viewerMenu = new ViewerMenu(this);
    connect(m_viewerMenu,   SIGNAL(fontBtnPress()),                 this, SLOT(handleFontBtnPress()));
    connect(m_viewerMenu,   SIGNAL(goToPageBtnPress()),             this, SLOT(handleGoToPagePress()));
    connect(m_viewerMenu,   SIGNAL(searchReq()),                    this, SLOT(handleSearchReq()));
    connect(m_viewerMenu,   SIGNAL(indexReq()),                     this, SLOT(showContents()));
    connect(m_viewerMenu,   SIGNAL(summaryReq()),                   this, SLOT(handleSummaryReq()));
    connect(m_viewerMenu,   SIGNAL(viewerConfReq()),                this, SIGNAL(viewerConf()));

    // Search
    m_searchPopup = new ViewerSearchPopup(this);
    connect(m_searchPopup,  SIGNAL(hideMe()),                       this, SLOT(hideAllElements()));
    m_searchPopup->hide();

    // Font Settings
    m_viewerAppearancePopup = new ViewerAppearancePopup(this);
    connect(m_viewerAppearancePopup,  SIGNAL(hideMe()),             this, SLOT(hideAllElements()));
    m_viewerAppearancePopup->hide();

    // Book Summary
    m_bookSummary = new ViewerBookSummary(this);
    connect(m_bookSummary,  SIGNAL(hideMe()),                       this, SLOT(hideAllElements()));
    connect(m_bookSummary,  SIGNAL(addNewCollection(const BookInfo*)), this, SIGNAL(createNewCollection(const BookInfo*)));
    m_bookSummary->hide();

    // Goto popup
    m_gotoPopup = new ViewerGotoPopup(this);
    connect(m_gotoPopup,    SIGNAL(goPage(int)),                    this, SLOT(goToPage(int)));
    connect(m_gotoPopup,    SIGNAL(hideMe()),                       this, SLOT(hideAllElements()));
    m_gotoPopup->hide();

    // Dictionary and Marks
#ifndef HACKERS_EDITION
    m_viewerDictionary = new ViewerDictionary(this);
#endif
    m_markHandler = new ViewerMarkHandler(this);

#ifndef HACKERS_EDITION
    connect(m_markHandler,  SIGNAL(dictioSearchReq(QStringList)),   m_viewerDictionary, SLOT(dictioSearch(QStringList)));
    connect(m_viewerDictionary, SIGNAL(hideMe()),                    this,               SLOT(hideAllElements()));
    connect(m_markHandler,  SIGNAL(wikiSearchReq(QString)),         m_viewerDictionary, SLOT(wikiSearch(QString)));
#endif
    connect(m_markHandler,  SIGNAL(bookSearchReq(const QString&)),  this,               SLOT(performSearchRequest(const QString&)));
    connect(m_markHandler,  SIGNAL(setBookmark(bool)),              this,               SLOT(showBookmark(bool)));
    // Book Index
    m_bookIndex = new ViewerContentsPopup(this);
    m_bookIndex->hide();
    connect(m_bookIndex,    SIGNAL(editNote(bool)),                 m_markHandler,  SLOT(handleNoteAction(bool)));
    connect(m_bookIndex,    SIGNAL(deleteNote(bool)),               m_markHandler,  SLOT(handleDeleteAction(bool)));
    connect(m_bookIndex,    SIGNAL(goToMark(QString)),              this,           SLOT(goToMark(QString)));
    connect(m_bookIndex,    SIGNAL(hideMe()),                       this,           SLOT(hideAllElements()));

    // Screen icons
    connect(bookmark,               SIGNAL(changeBookmark()),       this,           SLOT(handleBookmark()));
    connect(bookmarkLandscape,      SIGNAL(changeBookmark()),       this,           SLOT(handleBookmark()));
#ifndef HACKERS_EDITION
    connect(buySampleBtn,           SIGNAL(clicked()),              this,           SLOT(buyBook()));
#endif

    connect(pageWindow,             SIGNAL(goBack()),                    this,       SLOT(goPageBack()));
    connect(pageWindow,             SIGNAL(pdfMenuBtnClicked()),         this,       SLOT(pdfMenuBtnClicked()));
    connect(pageWindow,             SIGNAL(tapEvent(TouchEvent*, bool)), this,       SLOT(processPressEvent(TouchEvent*, bool)));
    connect(pageWindow,             SIGNAL(touchEvent(TouchEvent*)),     this,       SLOT(processTouchEvent(TouchEvent*)));

    connect(pageWindowLandscape,    SIGNAL(goBack()),                    this,       SLOT(goPageBack()));
    connect(pageWindowLandscape,    SIGNAL(pdfMenuBtnClicked()),         this,       SLOT(pdfMenuBtnClicked()));

    // Connect time update (timer is necessary just in case awake because of sync)
    connect(PowerManager::getInstance(),SIGNAL(backFromSuspend()),  this, SLOT(updateTime()));
    connect(PowerManager::getInstance(),SIGNAL(backFromSleep()),    this, SLOT(updateTime()));
    connect(&m_updateTimeTimer,SIGNAL(timeout()),                   this, SLOT(updateTime()));
    m_updateTimeTimer.setInterval(POWERMANAGER_UPDATE_CLOCK_TIME*1000);

    // pdfToolsWindow
    connect(pdfToolsWindow, SIGNAL(zoomMinusBtnClicked()),          this, SIGNAL(zoomOut()));
    connect(pdfToolsWindow, SIGNAL(zoomPlusBtnClicked()),           this, SIGNAL(zoomIn()));
    connect(pdfToolsWindow, SIGNAL(fullScreenBtnClicked()),         this, SLOT(screenAdjust()));
    connect(pdfToolsWindow, SIGNAL(widthAdjustBtnClicked()),        this, SLOT(widthAdjust()));
    connect(pdfToolsWindow, SIGNAL(heightAdjustBtnClicked()),       this, SLOT(heightAdjust()));
    connect(pdfToolsWindow, SIGNAL(landscapeModeBtnClicked()),      this, SLOT(handleLandscapeMode()));
    connect(pdfToolsWindow, SIGNAL(closeBtnClicked()),              this, SLOT(closePdfToolsWindow()));

    // pdfToolsWindowLandscape
    connect(pdfToolsWindowLandscape, SIGNAL(zoomMinusBtnClicked()),          this, SIGNAL(zoomOut()));
    connect(pdfToolsWindowLandscape, SIGNAL(zoomPlusBtnClicked()),           this, SIGNAL(zoomIn()));
    connect(pdfToolsWindowLandscape, SIGNAL(fullScreenBtnClicked()),         this, SLOT(screenAdjust()));
    connect(pdfToolsWindowLandscape, SIGNAL(widthAdjustBtnClicked()),        this, SLOT(widthAdjust()));
    connect(pdfToolsWindowLandscape, SIGNAL(heightAdjustBtnClicked()),       this, SLOT(heightAdjust()));
    connect(pdfToolsWindowLandscape, SIGNAL(landscapeModeBtnClicked()),      this, SLOT(handleLandscapeMode()));
    connect(pdfToolsWindowLandscape, SIGNAL(closeBtnClicked()),              this, SLOT(closePdfToolsWindow()));

    m_powerLock = PowerManager::getNewLock(this);
    i_maxQuickRefresh = QBook::settings().value("settings/viewer/maxRefresh",5).toInt();
    m_showTitle = titleShouldBeShown();
    m_showDateTime = dateTimeShouldBeShown();

    buySampleBtn->raise();
    ViewerContLandscape->hide();
    miniature->hide();
    miniatureLandscape->hide();

    m_viewerStepsManager = new ViewerStepsManager();

    switch(QBook::getInstance()->getResolution())
    {
        case QBook::RES1072x1448:
            i_minPxSwipeLenght = SWIPE_MIN_LENGTH_FHD;
            i_scrollAreaWidth = SCROLL_AREA_WIDTH_FHD;
            break;
        case QBook::RES758x1024:
            i_minPxSwipeLenght = SWIPE_MIN_LENGTH_HD;
            i_scrollAreaWidth = SCROLL_AREA_WIDTH_HD;
            break;
        case QBook::RES600x800: default:
            i_minPxSwipeLenght = SWIPE_MIN_LENGTH;
            i_scrollAreaWidth = SCROLL_AREA_WIDTH;
            break;
    }

    bookmark->updateDisplay(false);
    bookmarkLandscape->updateDisplay(false);
}

Viewer::~Viewer()
{
    qDebug() << Q_FUNC_INFO;

    delete m_viewerMenu;
    m_viewerMenu = NULL;
#ifndef HACKERS_EDITION
    delete m_viewerDictionary;
#endif
    m_viewerDictionary = NULL;
    delete m_markHandler;
    m_markHandler = NULL;
    delete m_searchPopup;
    m_searchPopup = NULL;
    delete m_viewerAppearancePopup;
    m_viewerAppearancePopup = NULL;
    delete m_bookSummary;
    m_bookSummary = NULL;
    delete m_bookIndex;
    m_bookIndex = NULL;

    delete m_docView;
    m_docView = NULL;
    delete m_powerLock;
    m_powerLock = NULL;
    delete m_gotoPopup;
    m_gotoPopup = NULL;
    delete m_pMouseFilter;
    m_pMouseFilter = NULL;
    delete m_bookInfo;
    m_bookInfo = NULL;
}

void Viewer::activateForm()
{
    qDebug() << Q_FUNC_INFO;

    b_opening = true;
    if (m_docView)
    {
        if(hasChangedOptions() && !m_docView->isHorizontal()) reloadCurrentBook();
        m_docView->setFormActivated(true);
        m_docView->setBlockPaintEvents(false);           
    }

    // Clear Screen
    resetScreen();

    QBookApp::instance()->getStatusBar()->hide();
    QBookApp::instance()->getStatusBar()->hideReadbutton();

    qDebug() << Q_FUNC_INFO << ": getting read mode";

    setChapterPosInfo();
    if(m_docView && m_bookInfo)
    {
        m_bookInfo->pageCount = m_docView->pageCount() - 1;
        pageWindow->setCurrentReadingPercent(m_bookInfo->readingProgress*100);
        pageWindowLandscape->setCurrentReadingPercent(m_bookInfo->readingProgress*100);
        pageWindow->setCurrentPageMode(isPdfBtnAllowed());
        pageWindowLandscape->setCurrentPageMode(isPdfBtnAllowed());
    }

    i_maxQuickRefresh = QBook::settings().value("settings/viewer/maxRefresh",5).toInt();
    m_showTitle = titleShouldBeShown();
    m_showDateTime = dateTimeShouldBeShown();
    m_shownYet = false;
    i_refreshCounter = 1;

    if(m_showDateTime) m_updateTimeTimer.start();

    // Connect to model changed
    connect(QBookApp::instance()->getModel(), SIGNAL(modelChanged(QString,int)), this, SLOT(modelChanged(QString,int)), Qt::UniqueConnection);

    QTimer::singleShot(0,this,SLOT(checkAndExtractCover()));

    qDebug() << Q_FUNC_INFO << QBookApp::instance()->isSynchronizing();
    qDebug() << Q_FUNC_INFO << QBookApp::instance()->isBuying();
    qDebug() << Q_FUNC_INFO << QBookApp::instance()->isResumingWifi();
    qDebug() << Q_FUNC_INFO << searchigWikipedia();

    if(!QBookApp::instance()->isSynchronizing() &&
       !QBookApp::instance()->isBuying() &&
       !QBookApp::instance()->isResumingWifi() &&
       !searchigWikipedia())
    {
        disconnectWifi();
    }
    else
    {
        connect(QBookApp::instance(), SIGNAL(periodicSyncFinished()), this, SLOT(disconnectWifi()), Qt::UniqueConnection);
        connect(QBookApp::instance(), SIGNAL(resumingWifiFailed()),   this, SLOT(disconnectWifi()), Qt::UniqueConnection);
    }

    qDebug() << Q_FUNC_INFO << "BEFORE CHECK WIFI USE STATUS";

    showTimeTitleLabels();
    setCurrentChapterInfo();
    // Start time
    updateTimeOnPage();
    m_viewerStepsManager->init(m_bookInfo->totalReadingTime);
    m_bookInfo->readingStatus = BookInfo::READING_BOOK;
    b_opening = false;
}

void Viewer::deactivateForm()
{
    qDebug() << Q_FUNC_INFO;

    if (m_docView)
    {
        m_searchPopup->stopSearch();
        m_docView->setFormActivated(false);
        // NOTE: Disable rendering when deactivating the Viewer.
        m_docView->setBlockPaintEvents(true);
    }

    if(b_hiliMode)
    { // New Highlight ongoing
        m_docView->removeHighlight(i_hiliId);
        b_hiliMode=false;
    }
    history.clear();
    pageWindow->setNeedToPaint(true);
    pageWindow->hideBackBtn();
    pageWindowLandscape->setNeedToPaint(true);
    pageWindowLandscape->hideBackBtn();

    if(m_timeOnPage.isValid())
    {
        int secsToCurrentTime = m_timeOnPage.secsTo(QDateTime::currentDateTimeUtc());
        if(secsToCurrentTime > MAX_SECS_FOR_STEP) // Only add 5 minutes for each screen.
            finalStepTimestamp = initialStepTimestamp + MAX_MSECS_FOR_STEP;
        else
            finalStepTimestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();

        if(secsToCurrentTime > mSecsNeedToReadStep)
            registerStepRead();
    }

    if(m_bookInfo)
        m_bookInfo->appendPercent(m_viewerStepsManager->getStringFromRanges());

    // Disconnect to model changed, for updating bookInfo
    disconnect(QBookApp::instance()->getModel(), SIGNAL(modelChanged(QString, int)), this, SLOT(modelChanged(QString, int)));

    setTimestamp(QDateTime::currentDateTime().toMSecsSinceEpoch());
    updateBookInfo();

    // Connecting it again so our copy is updated
    connect(QBookApp::instance()->getModel(), SIGNAL(modelChanged(QString, int)), this, SLOT(modelChanged(QString, int)), Qt::UniqueConnection);

    hideAllElements();
    dateTimeLbl->hide();
    pageWindowLandscape->hideDateTime();
    m_updateTimeTimer.stop();
    titleLbl->hide();
    pageWindowLandscape->hideTitle();
    m_shownYet = false;
    QBookApp::instance()->getStatusBar()->showButtons();

    if (!QBook::settings().value("wifi/disabled",false).toBool() && !searchigWikipedia())
    {
        // Reconnect to wifi if it was enabled
        // Singleshot with timeout to queue connection instead of execute
        // Prevent delay on screen refresh
        QTimer::singleShot(100, this, SLOT(resumeWifi()));
    }
    clearStepsProcess();
}

// FIXME: Se está llamando al modelChanged cuando salimos del visor, se actualiza el libro leido,
// y se llama al reloadCurrentBook (no tiene sentido)
void Viewer::modelChanged( QString path, int updateType )
{
    qDebug() << Q_FUNC_INFO;

    if(m_bookInfo && QDir::cleanPath(m_bookInfo->path) == QDir::cleanPath(path))
    {
        const BookInfo* modelBookInfo = QBookApp::instance()->getModel()->getBookInfo(path);
        if(modelBookInfo)
        {
            bool shouldReload = false;

            if(updateType & UPDATE_ARCHIVED && modelBookInfo->m_archived)
            {
                // Reset Book
                resetViewerBookInfo();
                // Exit Viewer
                if(QBookApp::instance()->isViewerCurrentForm()){
                    emit hideMe();
                }
                return;
            }

            if (updateType & UPDATE_EXPIRATION && updateType & UPDATE_CLOSED && modelBookInfo->hasExpired())
            {
                // Exit Viewer
                qDebug() << Q_FUNC_INFO << "modelBookInfo expired";
                resetViewerBookInfo();

                if(QBookApp::instance()->isViewerCurrentForm() && !QBookApp::instance()->isPoweringOffDevice())
                {
                    qDebug() << Q_FUNC_INFO << "is viewer current form";
                    ConfirmDialog* confirmDialog = new ConfirmDialog(this,tr("Your subscription has expired so the book has been closed."));
                    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
                    confirmDialog->showForSpecifiedTime(TIME_SHOW_SUBSCRIPTION_DIALOG);
                    delete confirmDialog;
                    emit hideMe();
                }
                return;
            }

            if (updateType & UPDATE_TYPE)
            {
               if((modelBookInfo->m_type == BookInfo::BOOKINFO_TYPE_PURCHASE || modelBookInfo->m_type == BookInfo::BOOKINFO_TYPE_SUBSCRIPTION) && m_bookInfo->m_type == BookInfo::BOOKINFO_TYPE_DEMO)
               {
                   m_bookInfo->m_type = modelBookInfo->m_type;
                   m_bookInfo->update(modelBookInfo);
                   qs_bookMark = modelBookInfo->lastReadLink;

                   buySampleBtn->hide();
                   shouldReload = true;
               }
            }

            if(updateType & UPDATE_CLOSED && !modelBookInfo->lastTimeRead.isValid() && !shouldReload)
            {
                // Reset Book
                resetViewerBookInfo();
                // Exit Viewer
                if(QBookApp::instance()->isViewerCurrentForm()){
                    emit hideMe();
                }
                return;
            }

            bool userChoice = false;

            if (updateType & UPDATE_READING_POSITION && m_bookInfo && modelBookInfo->lastReadLink != m_bookInfo->lastReadLink && !shouldReload)
            {
                qDebug() << Q_FUNC_INFO << "updating reading point";

                if(QBookApp::instance()->isViewerCurrentForm() && !QBookApp::instance()->isSleeping())
                {
                    SelectionDialog* confirmDialog = new SelectionDialog(this,tr("The readingPoing has changed. Do you want to go to the new position?"));
                    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
                    confirmDialog->exec();

                    userChoice = confirmDialog->result();

                    if(userChoice)
                    {
                        m_bookInfo->lastReadLink = modelBookInfo->lastReadLink;
                        m_bookInfo->readingProgress = modelBookInfo->readingProgress;
                    }
                    delete confirmDialog;

                }else{
                    m_bookInfo->lastReadLink = modelBookInfo->lastReadLink;
                    m_bookInfo->readingProgress = modelBookInfo->readingProgress;
                }

                if(!shouldReload && userChoice){
                    Screen::getInstance()->queueUpdates();
                    goToMark(m_bookInfo->lastReadLink);
                    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
                    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
                    Screen::getInstance()->flushUpdates();
                }
            }

            if (updateType & UPDATE_READING_METADATA)
            {
                qDebug() << Q_FUNC_INFO << "updating locations";
                m_bookInfo->setLocations(modelBookInfo->getLocations());
                if(m_docView)
                    m_markHandler->reloadMarks();
            }

            if (shouldReload)
            {
                if(b_opening)
                    connect(this, SIGNAL(openingFinished()), this, SLOT(reloadCurrentBook()), Qt::UniqueConnection);
                else
                {
                    if(QBookApp::instance()->isViewerCurrentForm() && !QBookApp::instance()->isSleeping())
                        QBookApp::instance()->getStatusBar()->setBusy(true);

                    Screen::getInstance()->queueUpdates();
                    reloadCurrentBook();

                    if(m_docView)
                        m_bookInfo->pageCount = m_docView->pageCount();

                    if(m_docView->isHorizontal())
                    {
                        if(pageWindowLandscape) pageWindowLandscape->updateDisplay();
                    }
                    else
                    {
                        if(pageWindow) pageWindow->updateDisplay();
                    }


                    if(QBookApp::instance()->isViewerCurrentForm() && !QBookApp::instance()->isSleeping())
                        QBookApp::instance()->getStatusBar()->setBusy(false);

                    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
                    Screen::getInstance()->flushUpdates();
                }
            }
        }
        else
        {
            if (updateType & UPDATE_EXPIRATION && updateType & UPDATE_CLOSED && m_bookInfo && m_bookInfo->hasExpired())
            {
                ConfirmDialog* confirmDialog = new ConfirmDialog(this,tr("Your subscription has expired so the book has been closed because ."));
                Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
                confirmDialog->showForSpecifiedTime(TIME_SHOW_SUBSCRIPTION_DIALOG);
                delete confirmDialog;
            }
            // Exit Viewer
            resetViewerBookInfo();
            qDebug() << Q_FUNC_INFO << "deleting book";

            if(QBookApp::instance()->isViewerCurrentForm())
            {
                qDebug() << Q_FUNC_INFO << "is viewer currentForm";
                emit hideMe();
            }
        }
    }
}

void Viewer::updateBookInfo( bool writeToFile )
{

    qDebug() << Q_FUNC_INFO;
    if(m_bookInfo && m_docView)
    {
        QDocView::Location* loc = m_docView->bookmark();
        if (loc)
        {
            m_bookInfo->lastReadLink = loc->ref;
            if(m_docView->getPageEnd()) m_bookInfo->lastReadPage = m_docView->getPageEnd();

            if(m_bookInfo->pageCount > 0)
            {
                m_bookInfo->readingProgress = float(m_bookInfo->lastReadPage) / float(m_bookInfo->pageCount);
                if(m_bookInfo->readingPercentage < m_bookInfo->readingProgress)
                    m_bookInfo->readingPercentage = m_bookInfo->readingProgress;
            }

            delete loc;
        }

        qDebug() << Q_FUNC_INFO << " m_bookInfo->getReadingPeriod(): " << m_bookInfo->getReadingPeriod();
        if(m_timeOnPage.isValid())
        {
            if(m_timeOnPage.secsTo(QDateTime::currentDateTimeUtc()) > MAX_SECS_FOR_STEP) // Only add 5 minutes for each screen.
                m_bookInfo->setReadingPeriod(m_bookInfo->getReadingPeriod() + MAX_SECS_FOR_STEP);
            else
                m_bookInfo->setReadingPeriod(m_bookInfo->getReadingPeriod() + m_timeOnPage.secsTo(QDateTime::currentDateTimeUtc()));
        }
        qDebug() << Q_FUNC_INFO << " m_bookInfo->getReadingPeriod(): " << m_bookInfo->getReadingPeriod();

        m_docView->setFormActivated(false);
        m_bookInfo->lastTimeRead = QDateTime::currentDateTime();

        // Update only and only if the bookInfo exists in the model
        if(writeToFile)
        {
            qDebug() << Q_FUNC_INFO << "Updating the book";
            disconnect(QBookApp::instance()->getModel(), SIGNAL(modelChanged(QString, int)), this, SLOT(modelChanged(QString, int)));
            QBookApp::instance()->getModel()->updateBook(m_bookInfo);
            connect(QBookApp::instance()->getModel(), SIGNAL(modelChanged(QString, int)), this, SLOT(modelChanged(QString, int)), Qt::UniqueConnection);
        }
    }
}

void Viewer::resetViewerWidgets()
{
    pageWindow->setPdfToolbarState(false);
    pageWindowLandscape->setPdfToolbarState(false);
    pdfToolsWindow->hide();
    pdfToolsWindowLandscape->hide();
    pdfToolsWindow->setLandscapeImages(false);
    miniature->hide();
    miniatureLandscape->hide();

    if (m_docView && m_docView->isHorizontal())
    {
        ViewerCont->hide();
        viewerHeader->hide();
        bookMarkCont->hide();
        ViewerContLandscape->show();
    }
    else
    {
        ViewerCont->show();
        viewerHeader->show();
        bookMarkCont->show();
        ViewerContLandscape->hide();
    }
}

void Viewer::openDoc(const BookInfo* content)
{
    qDebug() << Q_FUNC_INFO;
    if(b_opening) return;

    b_opening = true;

    // NOTE: Disable rendering until the loading is done.
    if(m_docView)
        m_docView->setBlockPaintEvents(true);

    if(!content)
    {
        emit openingFinished();
        b_opening = false;
        return;
    }

    delete m_bookInfo;
    m_bookInfo = new BookInfo(*content);

    if(content->lastReadPage == 1)
    {
        pageWindow->resetPager();
        pageWindowLandscape->resetPager();
        pdfToolsWindow->hide();
        pdfToolsWindowLandscape->hide();
    }

    qDebug() << Q_FUNC_INFO << "Title: " << m_bookInfo->title;
    QString path = m_bookInfo->path;

    if(m_bookInfo->m_type == BookInfo::BOOKINFO_TYPE_DEMO) buySampleBtn->show();
    else                                                   buySampleBtn->hide();

    if (path.isNull())
    { // Check path param
        qDebug() << Q_FUNC_INFO << "Null path";
        emit openingFinished();
        b_opening = false;
        return;
    }

    QFile file(path);// Check file exists
    if (!file.exists())
    {
        qDebug() << Q_FUNC_INFO << "File does NOT exist";
        emit openingFinished();
        b_opening = false;
        return;
    }

    // Set absolute file path, as emulator uses relative paths
    path = QFileInfo(file).absoluteFilePath();

    qs_bookMark = m_bookInfo->lastReadLink;

    if (i_loadState == QDocView::LOAD_FAILED)
        resetDocView();

    m_warningMsgs.clear();

    if(isOtherBook(path) || b_reloading)
    {
        if(m_bookInfo->language.isEmpty())
        {
            qDebug() << Q_FUNC_INFO << "language has to be read before opening";
            m_bookInfo->language = MetaDataExtractor::getLanguage(path);
        }
        b_reloading = false;
        qs_docPath = path;
        pageWindow->resetPager();
        pageWindowLandscape->resetPager();
        pdfToolsWindow->hide();
        pdfToolsWindowLandscape->hide();
        m_bookIndex->clearContentList();
        loadDocument();
        if(!m_docView)
        {
            b_opening = false;
            return;
        }
    }
    else
    {
        if(m_docView->isDrmFile())
            loadDocument();
        else
        {
            m_docView->setBookInfo(*m_bookInfo);

            qDebug() << Q_FUNC_INFO << "Same book opened";
            // Reload marks, in case we have new ones from sync
            m_markHandler->applyMarks();
        }
    }

    resetViewerWidgets();
    clearStepsProcess();

    if(m_docView)
    {
        m_errorAfterLoading = false;

        QBookApp::instance()->getStatusBar()->setMenuTitle(m_bookInfo->title);

        // Viewer's Menu setup
        m_bookSummary->setup();
        m_gotoPopup->setup();
        m_searchPopup->setup();
        //

        gotoLastBookMark();
        m_bookIndex->setup();
        m_bookInfo->resetReadingPeriod();

        if(m_errorAfterLoading)
        {
            resetDocView();
        }
        else
        {
            m_docView->setBlockPaintEvents(false); // PaintEvents blocked during loadDocument()
            m_docView->update();
        }
    }

    emit openingFinished();
    b_opening = false;
}

void Viewer::registerStepRead()
{
    qDebug() << Q_FUNC_INFO;
    m_viewerStepsManager->stepRead(initialStepPos,finalStepPos, initialStepTimestamp, finalStepTimestamp);
}

void Viewer::loadDocument()
{
    qDebug() << Q_FUNC_INFO << m_bookInfo;

    QTime time;
    time.start();

    textBody->layout()->removeWidget(m_docView);
    i_docExtension = getFileExtension(qs_docPath);

    setTextBodyMargins(m_bookInfo);

    if(i_docExtension == EXT_EPUB)
        connect(m_markHandler, SIGNAL(numPageMarks(int,int)), this, SLOT(numPageMarks(int,int)), Qt::UniqueConnection);

    delete m_docView; m_docView = 0;

    /// Load book
    QUrl url = QUrl::fromLocalFile(qs_docPath);
    QString mime = QDocView::guessMimeType(url);

    m_docView = QDocView::create(mime, qs_docPath, textBody);
    if (!m_docView)
    {
        qDebug() << Q_FUNC_INFO << "m_docview error";
        return;
    }

    m_docView->setBlockPaintEvents(true);
    m_docView->setBookInfo(*m_bookInfo);
    m_docView->setFormActivated(true);

    // Set textBody and m_viewer size
    setTextBodyGeometry();
    m_viewerAppearancePopup->applyMargin();// NOTE: This needs to be done before m_docView->setUrl

    time.restart();

    // Load QDocView in Ui
    textBody->layout()->addWidget(m_docView);

    if(history.isEmpty())
    {
        pageWindow->setNeedToPaint(true);
        pageWindow->hideBackBtn();
        pageWindowLandscape->setNeedToPaint(true);
        pageWindowLandscape->hideBackBtn();
    }
    else
    {
        if(m_docView->isHorizontal())
            pageWindowLandscape->showBackBtn();
        else
            pageWindow->showBackBtn();
    }

    if(m_bookInfo->lastReadPage == 1)
    {
        pageWindow->resetPager();
        pageWindowLandscape->resetPager();
        pdfToolsWindow->hide();
        pdfToolsWindowLandscape->hide();
    }

    i_loadState = QDocView::LOAD_NONE;
    m_errorMsg.clear();
    m_loadDocumentError = QDocView::EDVLE_NONE;

    connect(m_docView, SIGNAL(stateChanged(int)),            this,      SLOT(handleLoadState(int)));
    connect(m_docView, SIGNAL(fatalOccurred()),              this,      SLOT(handleFatal()));
    connect(m_docView, SIGNAL(errorOccurred(QString, int)),  this,      SLOT(handleError(QString, int)));
    connect(m_docView, SIGNAL(warningOccurred(QStringList)), this,      SLOT(handleWarnings(QStringList)));
    connect(m_docView, SIGNAL(arrowShow(QList<bool>)),       this,      SLOT(handleScrollArrows(QList<bool>)));
    connect(m_docView, SIGNAL(aspectRatio(QRectF)),          miniature, SLOT(changeAspectRatio(QRectF)));
    connect(m_docView,  SIGNAL(aspectRatio(QRectF)),         miniatureLandscape, SLOT(changeAspectRatio(QRectF)));

    m_docView->setUrl(url);

    disconnect(m_docView, SIGNAL(stateChanged(int)),            this, SLOT(handleLoadState(int)));
    disconnect(m_docView, SIGNAL(fatalOccurred()),              this, SLOT(handleFatal()));
    disconnect(m_docView, SIGNAL(errorOccurred(QString, int)),  this, SLOT(handleError(QString, int)));
    disconnect(m_docView, SIGNAL(warningOccurred(QStringList)), this, SLOT(handleWarnings(QStringList)));

    qDebug() << Q_FUNC_INFO << "setUrl finished. time=" << time.elapsed();
    time.restart();

    // Error checking from handleErrors. This handles the error opening a PDF with password
    if(m_loadDocumentError == (QDocView::EDVLE_PASSWORD_ERROR | QDocView::EDVLE_FATAL_ERROR))
        i_loadState = QDocView::LOAD_FAILED;

    if(i_loadState == QDocView::LOAD_FAILED)
    {
        qDebug() << Q_FUNC_INFO << "Error settingUrl";
        if (m_docView)
            resetDocView();

        if(!m_errorMsg.size())
            m_errorMsg = tr("Error detected when trying to open the book.");

        QBookApp::instance()->getStatusBar()->setBusy(false);
        InfoDialog* dialog = new InfoDialog(this, m_errorMsg, 4000);
        dialog->hideSpinner();
        dialog->showForSpecifiedTime();
        delete dialog;

        emit loadDocError();
        return;
    }

    if(m_warningMsgs.size())
    {
        qDebug() << Q_FUNC_INFO << "Warnings settingUrl";

        QString message(tr("The following warnings occurred: "));
        message.append(m_warningMsgs.join(", "));
        qWarning() << Q_FUNC_INFO << "Warnings:"  << m_warningMsgs;

        /* Not need to show the message to the user.*/
        //InfoDialog* dialog = new InfoDialog(this, message, 4000);
        //dialog->hideSpinner();
        //dialog->showForSpecifiedTime();
        //delete dialog;
    }

    // Connect for after loading errors or warnings
    connect(m_docView, SIGNAL(fatalOccurred()),                 this, SLOT(handleFatalAfterLoading()));
    connect(m_docView, SIGNAL(errorOccurred(QString, int)),     this, SLOT(handleErrorAfterLoading(QString, int)));
    connect(m_docView, SIGNAL(warningOccurred(QStringList)),    this, SLOT(handleWarningsAfterLoading(QStringList)));

    // Setup dependent objects
    m_markHandler->setCurrentDoc(m_docView);

    // Set pageModeChange enable
    if(m_docView->isPageModeSupported(QDocView::MODE_HARD_PAGES) && m_docView->isPageModeSupported(QDocView::MODE_REFLOW_PAGES))
        b_pdfModeSupported = true;
    else
        b_pdfModeSupported = false;

    //check if supported file
    Viewer::isSupportedFile(qs_docPath);

    // Do it after setUrl
    m_viewerAppearancePopup->setup();

    qDebug() << Q_FUNC_INFO<< "EMITTING OPEN DONE";

    connect(m_docView,  SIGNAL(pageNumberChanged(int, int, int)),   pageWindow,                 SLOT(handlePageChange(int, int, int)), Qt::UniqueConnection);
    connect(m_docView,  SIGNAL(pageNumberChanged(int, int, int)),   pageWindowLandscape,        SLOT(handlePageChange(int, int, int)), Qt::UniqueConnection);
    connect(m_docView,  SIGNAL(pageNumberChanged(int, int, int)),   this,                       SLOT(setPageChanged(int, int, int)), Qt::UniqueConnection);
    connect(m_docView,  SIGNAL(pageNumberChanged(int, int, int)),   m_markHandler,              SLOT(handleMarksPerPage()), Qt::UniqueConnection);
    connect(m_docView,  SIGNAL(pageNumberChanged(int, int, int)),   miniature,                  SLOT(setPageChanged(int, int, int)), Qt::UniqueConnection);
    connect(m_docView,  SIGNAL(pageNumberChanged(int, int, int)),   miniatureLandscape,         SLOT(setPageChanged(int, int, int)), Qt::UniqueConnection);
    connect(this,       SIGNAL(zoomIn()),                           m_docView,                  SLOT(zoomIn()));
    connect(this,       SIGNAL(zoomOut()),                          m_docView,                  SLOT(zoomOut()));
    connect(m_docView,  SIGNAL(pdfZoomLevelChange(int)),            m_viewerAppearancePopup,    SLOT(pdfZoomLevelChange(int)));
    connect(m_docView,  SIGNAL(zoomChange(double)),                 miniature,                  SLOT(setZoomChanged(double)));
    connect(m_docView,  SIGNAL(zoomChange(double)),                 miniatureLandscape,         SLOT(setZoomChanged(double)));
    connect(m_docView,  SIGNAL(pageModeReflow()),                   this,                       SLOT(handlePageModeReflow()));

    miniature->setZoomChanged(m_docView->scaleFactor());
    miniatureLandscape->setZoomChanged(m_docView->scaleFactor());

    qDebug() << Q_FUNC_INFO<< "End";
}

void Viewer::handleLoadState(int state)
{
    qDebug() << Q_FUNC_INFO << "loadState = " << state;
    i_loadState = state;
}

void Viewer::keyReleaseEvent(QKeyEvent* event)
{
    qDebug() << Q_FUNC_INFO << event;
    if (i_loadState == QDocView::LOAD_STARTED)
    {
        QBookForm::keyReleaseEvent(event);
        return;
    }

    switch (event->key())
    {
    case QBook::QKEY_BACK:
        event->accept();

        if (!noWidgetShown()) //Any Widget shown
        {
            qDebug() << Q_FUNC_INFO << "any widget shown";
            m_markHandler->cancelMarkOp();
            hideAllElements();
        }
        else
            QBookForm::keyReleaseEvent(event);
        break;

    case QBook::QKEY_MENU:
        // Do nothing. Handled by QBookApp

    default:
        QBookForm::keyReleaseEvent(event);
        break;
    }    
}

bool Viewer::isOtherBook(const QString& newPath) const
{
    qDebug() << Q_FUNC_INFO << newPath << " != " << qs_docPath;
    return (qs_docPath != newPath);
}

/// Handle the forbiddance to use pdf tools with m_viewerDictionary showed.
void Viewer::handlePdfToolbarDuringDictioSearch()
{
    if (pdfToolsWindow->isVisible() || pdfToolsWindowLandscape->isVisible()) closePdfToolsWindow();

    pageWindow->setCurrentPageMode(false);
    pageWindowLandscape->setCurrentPageMode(false);
    checkChapterInfoAvailability();
}

void Viewer::handleLongPressStart(TouchEvent *event)
{
    qDebug() << Q_FUNC_INFO;
    m_powerLock->activate();

    if(b_hiliMode){ // New Highlight ongoing
        m_docView->removeHighlight(i_hiliId);
        hideAllElements();
        b_hiliMode = false;
    }

    if(!noWidgetShown())
        return;

    QPoint point = m_docView->mapFrom(this, event->pos());

    // Check if pointing word
    if(!m_markHandler->isPointingText(point))
    {
        i_hiliId = -1; // Set index to not valid
        b_hiliMode = false;
        qDebug() << "Aborting, no word pointed";
        return;
    }

    i_hiliId = m_markHandler->isHighlighted(point); // >= 0 if exists

#ifndef HACKERS_EDITION
    if(getCurrentDocExt() != EXT_EPUB)
    {
        m_docView->removeHighlight(i_hiliId);
        i_hiliId = -1;
        QString word = docView()->wordAt(point.x(),point.y());
        QStringList wordList;
        wordList << word << " " << " ";
        m_viewerDictionary->clearSearch();
        Screen::getInstance()->queueUpdates();

        if (isPdfBtnAllowed()) handlePdfToolbarDuringDictioSearch();

        m_viewerDictionary->dictioSearch(wordList);
        Screen::getInstance()->flushUpdates();

        b_hiliMode = false;
        m_powerLock->release();
        return;
    }
#endif

    if(i_hiliId < 0)
    { // Highlight doesn't exist. Entering higlight mode
        b_hiliMode = true;
        Screen::getInstance()->setMode(Screen::MODE_FAST,false,Q_FUNC_INFO);
        i_hiliId = m_markHandler->startHighlight(point);
        m_docView->setBlockPaintEvents(true);
        //m_markHandler->showMarkContextMenu(i_hiliId, true, point, m_docView->getHighlightBBox(i_hiliId));
    }

    else
    { // Highlight exists. Shows context edition menu
        b_hiliMode = false;
        //m_markHandler->showMarkContextMenu(i_hiliId, false, point, m_docView->getHighlightBBox(i_hiliId));
    }
}

void Viewer::processPressEvent(TouchEvent* event, bool eventFromViewerPageHandler)
{
    qDebug() << Q_FUNC_INFO;

    QPoint pressEventPos;
    if(eventFromViewerPageHandler)
    {
        pressEventPos = pageWindow->mapToGlobal(QPoint(event->pos().x(), event->pos().y()));
        int touchType = event->touchType();
        event = NULL;
        delete event;
        event = new TouchEvent(pressEventPos, touchType);
    }else
        pressEventPos = textBody->mapFromParent(QPoint(event->pos().x(), event->pos().y() - getUpperMargin()));

    if (noWidgetShown() && !m_currentMenuPopup)
    {
        if(checkPdfScroll(event)) // Check if scrolling in PDF
        {
            qDebug() << "SCROLLING";
            m_docView->scrolling();
            return;
        }

        // Check if pointing highlight. If it's not PDF
        if( getCurrentDocExt() != EXT_PDF && !eventFromViewerPageHandler)
        {
            QPoint point = m_docView->mapFrom(this, pressEventPos);
            i_hiliId = m_markHandler->isHighlighted(point); // >= 0 if exists
            if(i_hiliId >= 0) // Highlight exists. Shows context edition menu
            {
                b_hiliMode = false;
                m_markHandler->showTextActions(DEFAULT_POPUP, i_hiliId, point, m_docView->getHighlightBBox(i_hiliId));
                return;
            }
        }

        // If there is a mark going, erase it.
        if(b_hiliMode)
        {
            b_hiliMode = false;
            m_markHandler->reloadMarks();
            return;
        }

        // Link handling
        if(getCurrentDocExt() != EXT_PDF)
            pushHistory();

        if(m_docView->processEventInPoint(pressEventPos)) {
            m_screenSteps = 0;
            pageWindow->setNeedToPaint(true);
            pageWindowLandscape->setNeedToPaint(true);
            return;
        }

        if(getCurrentDocExt() != EXT_PDF) {
            if(!history.isEmpty())
                history.pop();
            if(history.isEmpty())
            {
                pageWindow->hideBackBtn();
                pageWindowLandscape->hideBackBtn();
            }
        }

        SettingsReaderPageTurning::TurnPageMode turnMode = SettingsReaderPageTurning::getCurrentTurnPageMode();

        // Action depending on pointed area and turn mode
        ScreenArea pointed = pointedArea(pressEventPos);

        if( pointed == CENTER )
        {
            if(!m_viewerMenu->isVisible())
            {
                currentChapter = false;
                Screen::getInstance()->queueUpdates();
                QBookApp::instance()->getStatusBar()->setSpinner(false);
                QBookApp::instance()->getStatusBar()->show();
                QBookApp::instance()->getStatusBar()->raise();
                Screen::getInstance()->flushUpdates();
                setCurrentChapterInfo();
                showElement(m_viewerMenu);
            }
            else
                m_viewerMenu->hide();
        }
        else if (turnMode == SettingsReaderMenu::MODE_TWOHANDS)
        {
            if(pointed == BOTTOM_RIGHT || pointed == TOP_RIGHT)
                nextScreen();
            else
                previousScreen();
        }
        else if (turnMode == SettingsReaderMenu::MODE_ONEHAND)
        {
            if(pointed == BOTTOM_RIGHT || pointed == BOTTOM_LEFT)
                nextScreen();
            else
                previousScreen();
        }
    }
    else // Any menu is shown
    {
        qDebug() << Q_FUNC_INFO << "Closing all";
        Screen::getInstance()->queueUpdates();

        hideAllElements();
        b_hiliMode = false;
        m_markHandler->reloadMarks();

        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
    }
}

void Viewer::mouseMoveEvent(QMouseEvent* event)
{
    qDebug() << Q_FUNC_INFO;
    event->accept();
    m_docView->setBlockPaintEvents(false);
    if(b_hiliMode && getCurrentDocExt() == EXT_EPUB)
    {
        QPoint movingPointInDoc = m_docView->mapFrom(this, event->pos());
        int returnedHiliId = m_markHandler->trackHighlight(movingPointInDoc);
        if((returnedHiliId >= 0) && (returnedHiliId != i_hiliId)) // valid and different of previous
        {
            qDebug() << Q_FUNC_INFO << "Updating highlight Id from " << i_hiliId << " to " << returnedHiliId;
            i_hiliId = returnedHiliId;
        }
    }
    else
    {
        qDebug() << Q_FUNC_INFO << "Movement ignored";
    }
}

void Viewer::handleLongPressEnd(TouchEvent* event)
{
    qDebug() << Q_FUNC_INFO;

    if(b_hiliMode) //New mark
    {
        // Finishing Highlight
        Screen::getInstance()->setMode(Screen::MODE_QUICK,false,Q_FUNC_INFO);

        QPoint point = m_docView->mapFrom(this, event->pos()); // end press point
        int returnedHiliId = m_markHandler->finishHighlight(point);
        if( (returnedHiliId >= 0) && (returnedHiliId != i_hiliId) && getCurrentDocExt() == EXT_EPUB) // valid and different of previous
        {
            qDebug() << "Updating highlight Id from " << i_hiliId << " to " << returnedHiliId;
            i_hiliId = returnedHiliId;

            qDebug() << Q_FUNC_INFO << "FINISHING HIGHLIGHT" << i_hiliId;

            QDocView::Location* loc = m_docView->highlightLocation(i_hiliId);
            if( loc && !loc->preview.isEmpty())
            {
                QRect bbox = m_docView->getHighlightBBox(i_hiliId);

                Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);

                QRegExp rx("\\s");

                m_docView->setBlockPaintEvents(false);

                if(loc->preview.split(rx).count() > 1)
                {
                    m_markHandler->showTextActions(DEFAULT_POPUP, i_hiliId, point, bbox);
                }
                else // first time in a word
                {
                    m_markHandler->showTextActions(INIT_SINGLE_WORD_POPUP, i_hiliId, point, bbox, loc->preview, getCurrentDocExt() == EXT_PDF);
                }
            }
            else
            {
                m_docView->removeHighlight(i_hiliId);
                i_hiliId = -1;
            }

            delete loc;
        }

        else // Invalid highlight Id
        {
            i_hiliId = -1;
            m_markHandler->reloadMarks();
            m_powerLock->release();
            return;
        }
    }
    else
    {
        m_docView->setBlockPaintEvents(false);
        qDebug() << "LONGPRESS WITHOUT HIGHLIGHT";

        if (!noWidgetShown())// Any menu is shown
        {
            qDebug() << Q_FUNC_INFO << "Closing all";
            Screen::getInstance()->queueUpdates();

            hideAllElements();
            b_hiliMode = false;
            m_markHandler->reloadMarks();

            Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
            Screen::getInstance()->flushUpdates();
        }
        else
        {
            // Check if pointing highlight. If it's not PDF
            if( getCurrentDocExt() != EXT_PDF )
            {
                QPoint point = m_docView->mapFrom(this, event->pos());
                i_hiliId = m_markHandler->isHighlighted(point); // >= 0 if exists
                if(i_hiliId >= 0) // Highlight exists. Shows context edition menu
                {
                    b_hiliMode = false;
                    m_markHandler->showTextActions(DEFAULT_POPUP, i_hiliId, point, m_docView->getHighlightBBox(i_hiliId));
                }
            }
        }
    }
    m_powerLock->release();
}


void Viewer::customEvent(QEvent* received)
{
    if (received->type() != (int)MouseFilter::TOUCH_EVENT)
    {
        qDebug() << "UNEXPECTED TYPE";
        return;
    }

#ifndef HACKERS_EDITION
    if (m_viewerDictionary->isVisible()) return;
#endif

    processTouchEvent(static_cast<TouchEvent*>(received));
}

void Viewer::processTouchEvent(TouchEvent* event)
{
    qDebug() << Q_FUNC_INFO << "Type=" << event->type() << ",POS=" << event->pos();
    if(QBookApp::instance()->isSleeping()) return;

    switch(event->touchType())
    {
    case MouseFilter::LONGPRESS_START:
        handleLongPressStart(event);
        break;

    case MouseFilter::LONGPRESS_END:
        handleLongPressEnd(event);
        break;

    case MouseFilter::TAP:
        processPressEvent(event);
        break;

    case MouseFilter::SWIPE_R2L:
        if(!QBook::settings().value("setting/enableSwipe", true).toBool() || !noWidgetShown())
            processPressEvent(event);
        else if(!checkPdfScroll(event))
        {
            if(!m_docView->isHorizontal()) // Vertical
                nextScreen();
            else if(!checkSecondaryAxisSwipe(event))
                processPressEvent(event);
        }
        else
            m_docView->scrolling();
        break;

    case MouseFilter::SWIPE_L2R:
        if(!QBook::settings().value("setting/enableSwipe", true).toBool() || !noWidgetShown())
            processPressEvent(event);
        else if(!checkPdfScroll(event))
        {
            if(!m_docView->isHorizontal()) // Vertical
                previousScreen();
            else if(!checkSecondaryAxisSwipe(event))
                processPressEvent(event);
        }
        else
            m_docView->scrolling();
        break;

    case MouseFilter::SWIPE_D2U:
        if(!QBook::settings().value("setting/enableSwipe", true).toBool() || !noWidgetShown())
            processPressEvent(event);
        else if(!checkPdfScroll(event))
        {
            if(m_docView->isHorizontal())
                nextScreen();
            else if(!checkSecondaryAxisSwipe(event))
                processPressEvent(event);
        }
        else
            m_docView->scrolling();
        break;

    case MouseFilter::SWIPE_U2D:
        if(!QBook::settings().value("setting/enableSwipe", true).toBool() || !noWidgetShown())
            processPressEvent(event);
        else if(!checkPdfScroll(event))
        {
            if(m_docView->isHorizontal())
                previousScreen();
            else if(!checkSecondaryAxisSwipe(event))
                processPressEvent(event);
        }
        else
            m_docView->scrolling();
        break;

    default:
        qWarning() << Q_FUNC_INFO << "UNEXPECTED EXIT";
    }
}

bool Viewer::checkSecondaryAxisSwipe(TouchEvent* event)
{
    qDebug() << Q_FUNC_INFO << m_docView->isHorizontal();

    int relevantMovementLength;

    if(m_docView->isHorizontal())
        relevantMovementLength = event->movement().y();
    else
        relevantMovementLength = event->movement().x();

    // Same logic for horizontal and vertical after conversion
    if(relevantMovementLength <= -i_minPxSwipeLenght)
        nextScreen();
    else if(relevantMovementLength >= i_minPxSwipeLenght)
        previousScreen();
    else // No action
        return false;

    // Event handled
    return true;
}

Viewer::ScreenArea Viewer::pointedArea(const QPoint& pos)
{
    qDebug() << Q_FUNC_INFO << "Horizontal: " << m_docView->isHorizontal();

    if(!m_docView->isHorizontal())
    {
        int x = pos.x();
        int y = pos.y();

        int thirdX = this->width()/3;
        int halfY = this->height()>>1;/*this->height()/2;*/

        if (x < thirdX)
        { // LEFT (0) <-> (1/3)
            if( y < halfY)
                return TOP_LEFT;
            else
                return BOTTOM_LEFT;
        }
        else if( (x >= thirdX) && (x < (thirdX<<1))) /*(2*thirdX)*/
        { // CENTER (1/3) <-> (2/3)
            return CENTER;
        }
        else
        { // RIGHT (2/3) <-> (1)
            if( y < halfY)
                return TOP_RIGHT;
            else
                return BOTTOM_RIGHT;
        }
    }
    else
    {
        int x = pos.x();
        int y = pos.y();

        int thirdY = this->height()/3;
        int halfX = this->width()>>1;/*this->width()/2;*/

        if( y < thirdY )
        { // LEFT (0) <-> (1/3)
            if( x < halfX )
                return BOTTOM_LEFT;
            else
                return TOP_LEFT;
        }
        else if( (y >= thirdY) && (y < (thirdY<<1))) /*(2*thirdY)*/
        { // CENTER (1/3) <-> (2/3)
            return CENTER;
        }
        else
        { // RIGHT (2/3) <-> (1)
            if( x < halfX)
                return BOTTOM_RIGHT;
            else
                return TOP_RIGHT;
        }
    }
}

void Viewer::showContents()
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->getStatusBar()->setSpinner(true);
    Screen::getInstance()->queueUpdates();
    m_docView->setBlockPaintEvents(true);

    menuPopUpShow(m_bookIndex);

    QBookApp::instance()->getStatusBar()->setSpinner(false);
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
    m_docView->setBlockPaintEvents(false);
}

void Viewer::handleSummaryReq()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();
    m_docView->setBlockPaintEvents(true);

    menuPopUpShow(m_bookSummary);

    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
    m_docView->setBlockPaintEvents(false);
}

void Viewer::handleFontBtnPress()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();
    m_docView->setBlockPaintEvents(true);

    menuPopUpShow(m_viewerAppearancePopup);

    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
    m_docView->setBlockPaintEvents(false);
}

void Viewer::handleGoToPagePress()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();
    m_docView->setBlockPaintEvents(true);

    menuPopUpShow(m_gotoPopup);

    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
    m_docView->setBlockPaintEvents(false);
}

void Viewer::handleSearchReq()
{
    qDebug() << Q_FUNC_INFO << m_searchPopup;
    menuPopUpShow(m_searchPopup);
}

void Viewer::showElement(QWidget* widget)
{
    qDebug() << Q_FUNC_INFO << widget;

    if(widget->inherits("ViewerMenu"))
    {
        // Show
        QBookApp::instance()->getStatusBar()->setSpinner(false);
        widget->show();
        widget->raise();
    }
    else if(widget->inherits("FullScreenWidget")) // Full Screen
    {
        // store current widget
        m_currentWidget = widget;

        // Show widgets
//        widget->move(0, QBookApp::instance()->getStatusBar()->height());
        QBookApp::instance()->getStatusBar()->show();
        widget->show();
        QBookApp::instance()->getStatusBar()->raise();
        widget->raise();
    }
    else if(widget->inherits("PopUp"))
    {
        // Store current popUp
        m_currentWidget = widget;

        // Show widgets
//        widget->move(0, QBookApp::instance()->getStatusBar()->height());
//        QBookApp::instance()->getStatusBar()->show();
        widget->show();
//        QBookApp::instance()->getStatusBar()->raise();
        widget->raise();
    }
    else if(widget->inherits("ViewerTextActionsPopup"))
    {
        // Store current popUp
        m_currentWidget = widget;
        widget->show();
        widget->raise();

    /** TODO: Remove FullScreenWidget structure and make each widget
    to show extra elements that need like MyNotes or Index
    using just the code below */
    }
    else // Unknown widget type
    {
        qDebug() << Q_FUNC_INFO << "OTHER";
        m_currentWidget = widget;
        widget->show();
        widget->raise();
    }
}

void Viewer::hideAllElements() // TODO: Rename properly
{
    qDebug() << Q_FUNC_INFO << "CurrentWidget: " << m_currentWidget;

    if(m_docView)
    {
        if(m_docView->isHorizontal())
        {
            if(pageWindowLandscape->shouldBeShown()) pageWindowLandscape->show();
        }
        else
        {
            if(pageWindow->shouldBeShown()) pageWindow->show();
        }
    }

    // Reset highlighting mode
    b_hiliMode = false;

    // Check whether any other action is needed. Returns if it's not the case
    if(! ( (m_viewerMenu && m_viewerMenu->isVisible())
           || (m_currentWidget && m_currentWidget->isVisible())
           || QBookApp::instance()->getStatusBar() ) )
            return;

    if(m_docView &&
            ( (m_currentWidget && m_currentWidget->isVisible())
            || (m_viewerMenu && m_viewerMenu->isVisible()) ) )
        m_docView->setBlockPaintEvents(false);

    Screen::getInstance()->queueUpdates();

    QBookApp::instance()->hideKeyboard();

    if(m_viewerMenu && m_viewerMenu->isVisible())
        m_viewerMenu->hide();

    if(m_bookSummary && m_bookSummary->isVisible() && m_bookInfo)
        m_bookInfo->update(m_bookSummary->getBookInfo());

    QBookApp::instance()->getStatusBar()->hide();

    menuPopUpHide();

    m_markHandler->hideTextDelimiters();

#ifndef HACKERS_EDITION
    // Handle the forbiddance to use pdf tools with m_viewerDictionary showed.
    if (m_viewerDictionary->isVisible())
    {
        if (isPdfBtnAllowed())
        {
            pageWindow->setCurrentPageMode(true);
            pageWindowLandscape->setCurrentPageMode(true);

             if (!m_docView->tableOfContent())
             {
                 pageWindow->hideChapterInfo();
                 pageWindowLandscape->hideChapterInfo();
             }
        }
    }
#endif

    if(m_currentWidget && m_currentWidget->isVisible())
        m_currentWidget->hide();

    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE,false);
    Screen::getInstance()->flushUpdates();
    i_refreshCounter = 1;
    Screen::getInstance()->setMode(Screen::MODE_QUICK,false,Q_FUNC_INFO);

}

Keyboard* Viewer::showKeyboard(const QString& text, bool showUp)
{
    qDebug() << Q_FUNC_INFO;
    return QBookApp::instance()->showKeyboard(text,showUp);
}

Keyboard* Viewer::hideKeyboard()
{
    qDebug() << Q_FUNC_INFO;

    return QBookApp::instance()->hideKeyboard();
}

void Viewer::backBtnHandling()
{
    m_screenSteps = 0;
    pageWindow->setNeedToPaint(true);
    pageWindowLandscape->setNeedToPaint(true);
    pushHistory();
}

void Viewer::gotoSearchWordPage(const QString &str)
{
    qDebug() << Q_FUNC_INFO << str;

    Screen::getInstance()->queueUpdates();
    b_hiliMode = true;//Set true to delete all highlights after the search.

    backBtnHandling();

    // TODO: handle the five lines bellow inside gotoBookmark function
    m_docView->setBlockPaintEvents(true);
    m_markHandler->addTempHighlight(str);
    m_docView->gotoBookmark(str);
    m_docView->setBlockPaintEvents(false);
    m_docView->update();
    if(getCurrentDocExt() == EXT_PDF) m_docView->takeMiniatureScreenshot();


    Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void Viewer::goToMark(const QString& ref)
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();
    hideAllElements();
    backBtnHandling();

    // TODO: handle the four lines bellow inside gotoBookmark function
    m_docView->setBlockPaintEvents(true);
    m_docView->gotoBookmark(ref);
    m_docView->setBlockPaintEvents(false);
    m_docView->update();
    if(getCurrentDocExt() == EXT_PDF) m_docView->takeMiniatureScreenshot();


    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void Viewer::goToMarkup(const QString &reference)
{
    qDebug() << Q_FUNC_INFO << reference;
    Screen::getInstance()->queueUpdates();

    // TODO: handle the four lines bellow inside gotoBookmark function
    m_docView->setBlockPaintEvents(true);
    m_docView->gotoBookmark(reference);
    m_docView->setBlockPaintEvents(false);
    m_docView->update();
    if(getCurrentDocExt() == EXT_PDF) m_docView->takeMiniatureScreenshot();

    setRefreshModeOnTurningPage();
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

int Viewer::getFontSize() const
{
    qDebug() << Q_FUNC_INFO;
    return m_viewerAppearancePopup->getFontSizeId();
}

int Viewer::pageMode() const
{
    return m_docView->pageMode();
}

bool Viewer::isScrollable() const
{
    return (m_docView->scaleFactor() > 0);
}

bool Viewer::isTopScrollArea(QPoint& tapPoint)
{
    return (tapPoint.y() < i_scrollAreaWidth);
}

bool Viewer::isBottomScrollArea(QPoint& tapPoint)
{
    int scrollAreaWidth = i_scrollAreaWidth;
    if (!m_docView->isHorizontal()) scrollAreaWidth += pageWindow->height();
    return (tapPoint.y() > textBody->size().height() - scrollAreaWidth);
}

bool Viewer::isLeftScrollArea(QPoint& tapPoint)
{
    return (tapPoint.x() < i_scrollAreaWidth);
}

bool Viewer::isRightScrollArea(QPoint& tapPoint)
{
    return (tapPoint.x() > textBody->size().width() - i_scrollAreaWidth);
}

bool Viewer::checkPdfScroll(TouchEvent* event)
{
    qDebug() << Q_FUNC_INFO;

    if(!b_pdfModeSupported || !isScrollable())
        return false;

    int actionX = NO_SCROLL;
    int actionY = NO_SCROLL;

    switch(event->touchType()){
    case MouseFilter::TAP:
    {
        QPoint p = m_docView->mapFrom(this, event->pos());

        if(m_docView->isHorizontal())
        {
                 if (isTopScrollArea(p))    actionX = SCROLL_LEFT;
            else if (isBottomScrollArea(p)) actionX = SCROLL_RIGHT;
                 if (isRightScrollArea(p))  actionY = SCROLL_UP;
            else if (isLeftScrollArea(p))   actionY = SCROLL_DOWN;
        }
        else
        {
                 if (isTopScrollArea(p))    actionY = SCROLL_UP;
            else if (isBottomScrollArea(p)) actionY = SCROLL_DOWN;
                 if (isRightScrollArea(p))  actionX = SCROLL_RIGHT;
            else if (isLeftScrollArea(p))   actionX = SCROLL_LEFT;

        }
        break;
    }

    case MouseFilter::SWIPE_L2R:{
        qDebug() << Q_FUNC_INFO << "SCROLL_LEFT SWIPE";
        if(m_docView->isHorizontal())
            actionY = SCROLL_DOWN;
        else
            actionX = SCROLL_LEFT;
        break;
    }

    case MouseFilter::SWIPE_R2L:{
        qDebug() << Q_FUNC_INFO << "SCROLL_RIGHT SWIPE";
        if(m_docView->isHorizontal())
            actionY = SCROLL_UP;
        else
            actionX = SCROLL_RIGHT;
        break;
    }

    case MouseFilter::SWIPE_U2D:{
        qDebug() << Q_FUNC_INFO << "SCROLL_UP SWIPE";
        if(m_docView->isHorizontal())
            actionX = SCROLL_LEFT;
        else
            actionY = SCROLL_UP;
        break;
    }

    case MouseFilter::SWIPE_D2U:{
        qDebug() << Q_FUNC_INFO << "SCROLL_DOWN SWIPE";
        if(m_docView->isHorizontal())
            actionX = SCROLL_RIGHT;
        else
            actionY = SCROLL_DOWN;
        break;
    }
    default:
        qWarning() << Q_FUNC_INFO << "UNEXPECTED EVENT";
    }

    if(actionX == NO_SCROLL && actionY == NO_SCROLL)
        return false;

    return offsetPage(actionX, actionY);
}

bool Viewer::offsetPage(int xAction, int yAction)
{
    qDebug() << Q_FUNC_INFO;

    if (!m_docView || (xAction == NO_SCROLL && yAction == NO_SCROLL)) return false;

    double xDelta = 0;
    double yDelta = 0;

    switch (xAction)
    {
        case SCROLL_LEFT:  xDelta=-0.9; break;
        case SCROLL_RIGHT: xDelta= 0.9; break;
        default:                        break;
    }

    switch (yAction)
    {
        case SCROLL_UP:   yDelta=-0.9; break;
        case SCROLL_DOWN: yDelta= 0.9; break;
        default:                       break;
    }

    // Try Scroll
    if(!m_docView->setScaleFactor(m_docView->scaleFactor(), xDelta, yDelta))
    {
        qDebug() << Q_FUNC_INFO << "End of page";

        if(!isZoomed())
            return false;

        // Not allowing one hand method for turn page in zoomed mode.
        if(xAction == SCROLL_RIGHT || yAction == SCROLL_DOWN) m_docView->nextScreen();
        else if(xAction == SCROLL_LEFT || yAction == SCROLL_UP)  m_docView->previousScreen();
        else return false; // If no scroll turn page
    }

    setRefreshModeOnTurningPage();

    return true;
}

bool Viewer::isZoomed()
{
    if (!m_docView) return false;

    return scrollArrows[0] ||
           scrollArrows[1] ||
           scrollArrows[2] ||
           scrollArrows[3];
}


bool Viewer::noWidgetShown() const
{
    bool bRet = !m_currentWidget || !m_currentWidget->isVisible();
    bRet = bRet && !m_viewerMenu->isVisible();// No menu is shown
    return bRet;
}

int Viewer::getFileExtension(const QString& path)
{
         if (path.endsWith(".pdf",  Qt::CaseInsensitive)) return EXT_PDF;
    else if (path.endsWith(".epub", Qt::CaseInsensitive)) return EXT_EPUB;
    else if (path.endsWith(".fb2",  Qt::CaseInsensitive)) return EXT_FB2;
    else if (path.endsWith(".mobi", Qt::CaseInsensitive)) return EXT_MOBI;
    else if (path.endsWith(".doc",  Qt::CaseInsensitive)) return EXT_DOC;
    else if (path.endsWith(".txt",  Qt::CaseInsensitive)) return EXT_TXT;
    else if (path.endsWith(".rtf",  Qt::CaseInsensitive)) return EXT_RTF;
    //else if (path.endsWith(".chm",  Qt::CaseInsensitive)) return EXT_CHM;
    else if (path.endsWith(".html", Qt::CaseInsensitive)) return EXT_HTML;
    else if (path.endsWith(".htm", Qt::CaseInsensitive))  return EXT_HTML;

    //else if (path.endsWith(".tcr",  Qt::CaseInsensitive)) return EXT_TCR;
    //else if (path.endsWith(".pdb",  Qt::CaseInsensitive)) return EXT_PDB;
    //else if (path.endsWith(".zip",  Qt::CaseInsensitive)) return EXT_ZIP;

    else return EXT_NO_SUPPORTED;
}

bool Viewer::isSupportedFile(const QString& path)
{
    bool result = false;

    uint extension = getFileExtension(path);

    switch(extension)
    {
#ifndef DISABLE_ADOBE_SDK
        case EXT_PDF:
#endif
        case EXT_EPUB:
            return true;
        break;

        default:
            result = ((isCR3SupportedFile(path) != EXT_NO_SUPPORTED) ? true : false);
        break;
    }

    qDebug() << Q_FUNC_INFO << " File extension is supported: " << result;
        return result;
}

Viewer::SupportedExt Viewer::isCR3SupportedFile(const QString& path)
{
    uint extension = getFileExtension(path);

    switch(extension)
    {
        case EXT_EPUB:
        case EXT_FB2:
        case EXT_DOC:        
        case EXT_MOBI:
        case EXT_TXT:
        case EXT_RTF:
        //case EXT_CHM:
        case EXT_HTML:
        //case EXT_TCR:
        //case EXT_PDB:
        //case EXT_ZIP:
            return (SupportedExt)extension;
        default:
            return EXT_NO_SUPPORTED;
    }
}

bool Viewer::isUsingCR3(const QString& extension)
{
    return isUsingCR3((SupportedExt)getFileExtension("." + extension));
}

bool Viewer::isUsingCR3(SupportedExt extension)
{
    switch(extension)
    {
#ifdef DISABLE_ADOBE_SDK
        case EXT_EPUB:
#endif
        case EXT_FB2:
        case EXT_DOC:        
        case EXT_MOBI:
        case EXT_TXT:
        case EXT_RTF:
        //case EXT_CHM:
        case EXT_HTML:
        //case EXT_TCR:
        //case EXT_PDB:
        //case EXT_ZIP:
            return true;
        default:
            return false;
    }
}

void Viewer::previousScreen()
{
    qDebug() << Q_FUNC_INFO;

    if(!m_docView) return;

    //Clear history, when pass three screen steps and not use back button.

    if(abs(m_screenSteps) == MAX_SCREEN_STEPS && !history.isEmpty())
    {
        history.clear();
        pageWindow->setNeedToPaint(true);
        pageWindow->hideBackBtn();
        pageWindowLandscape->setNeedToPaint(true);
        pageWindowLandscape->hideBackBtn();
    }
    else if(abs(m_screenSteps) < MAX_SCREEN_STEPS)
        m_screenSteps--;
    Screen::getInstance()->queueUpdates();
    m_docView->previousScreen();
    setRefreshModeOnTurningPage();
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();

}

bool Viewer::nextScreen()
{
    qDebug() << Q_FUNC_INFO;

    if(!m_docView) return false;

    //Clear history, when pass three screen steps and not use back button.

    if(abs(m_screenSteps) == MAX_SCREEN_STEPS && !history.isEmpty())
    {
        history.clear();
        pageWindow->setNeedToPaint(true);
        pageWindow->hideBackBtn();
        pageWindowLandscape->setNeedToPaint(true);
        pageWindowLandscape->hideBackBtn();
    }
    else if(abs(m_screenSteps) < MAX_SCREEN_STEPS)
        m_screenSteps++;

    Screen::getInstance()->queueUpdates();
    bool hasNext = m_docView->nextScreen();
    setRefreshModeOnTurningPage();
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();

    return hasNext;
}

void Viewer::setRefreshModeOnTurningPage()
{
    if(i_refreshCounter == i_maxQuickRefresh)
    {
        i_refreshCounter = 0;
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
    }
    else
    {
        Screen::getInstance()->setMode(Screen::MODE_QUICK,true,FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    }

    i_refreshCounter++;
}

void Viewer::resetScreen()
{
    qDebug() << Q_FUNC_INFO;

    if (bookmark->isVisible())          bookmark->updateDisplay(false);
    if (bookmarkLandscape->isVisible()) bookmarkLandscape->updateDisplay(false);
}

void Viewer::setTextBodyGeometry()
{
    qDebug() << Q_FUNC_INFO << "TextBody dimensions " << textBody->x() << textBody->y() << textBody->width() << textBody->height();
    m_docView->setGeometry(textBody->x(), textBody->y(), textBody->width(), textBody->height());
}

void Viewer::removeBook(BookInfo* book)
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->getModel()->removeBook(book);

    hideAllElements();

    QBookApp::instance()->goToLibrary();
    delete m_docView;
    m_docView = 0;

    InfoDialog* dialog = new InfoDialog(this,tr("Book correctly removed."));
    dialog->hideSpinner();
    dialog->showForSpecifiedTime();
    delete dialog;
}

void Viewer::handleFatal()
{
    qDebug() << Q_FUNC_INFO;
    m_errorMsg = tr("A fatal error occurred.");
    m_loadDocumentError = QDocView::EDVLE_FATAL_ERROR;
}

void Viewer::handleError(QString msg, int errorMask)
{
    qDebug() << Q_FUNC_INFO << "msg: " << msg;
    m_errorMsg = msg;
    m_loadDocumentError = errorMask;
}

void Viewer::handleWarnings(QStringList warnings)
{
    qDebug() << Q_FUNC_INFO;

    int size = warnings.size();
    for(int i = 0; i < size; ++i)
    {
        if(!m_warningMsgs.contains(warnings.at(i)))
            m_warningMsgs.append(warnings.at(i));
    }
}

void Viewer::handleScrollArrows(QList<bool> arrows)
{
    qDebug() << Q_FUNC_INFO;

    scrollArrows = arrows;
}

void Viewer::handleFatalAfterLoading()
{
    qDebug() << Q_FUNC_INFO;

    QString message(tr("A fatal error occurred."));

    InfoDialog* dialog = new InfoDialog(this, message, 4000);
    dialog->hideSpinner();
    dialog->showForSpecifiedTime();
    delete dialog;

    m_errorAfterLoading = true;

    emit loadDocError();
}

void Viewer::handleErrorAfterLoading(QString errorMsg, int errorMask)
{
    qDebug() << Q_FUNC_INFO << "Error Msg: " << errorMsg << ", mask:" << errorMask;

    QString message(tr("The following error occurred: "));
    message.append(errorMsg);

    InfoDialog* dialog = new InfoDialog(this, errorMsg, 4000);
    dialog->hideSpinner();
    dialog->showForSpecifiedTime();
    delete dialog;
}

void Viewer::handleWarningsAfterLoading(QStringList warnings)
{
    qDebug() << Q_FUNC_INFO << warnings;

    if(warnings.size())
    {
        QString message(tr("The following warnings occurred: "));
        message.append(warnings.join(", "));
        qWarning() << Q_FUNC_INFO << "Warnings:"  << m_warningMsgs;

        /* Not need to show the message to the user.*/
        //InfoDialog* dialog = new InfoDialog(this, message, 4000);
        //dialog->hideSpinner();
        //dialog->showForSpecifiedTime();
        //delete dialog;
    }
}

void Viewer::reloadCurrentBook()
{
    qDebug() << Q_FUNC_INFO << m_bookInfo->lastReadPage;

    disconnect(this, SIGNAL(openingFinished()), this, SLOT(reloadCurrentBook()));

    Screen::getInstance()->queueUpdates();
    loadDocument();
    gotoLastBookMark();
    m_docView->setBlockPaintEvents(false); // PaintEvents blocked during loadDocument()
    Screen::getInstance()->flushUpdates();
}

void Viewer::gotoLastBookMark()
{
    if(!m_bookInfo->lastReadLink.size())
    {
        if(m_bookInfo->readingProgress > 0)
        {
            qs_bookMark = m_docView->getBookmarkFromPosition(m_bookInfo->readingProgress);
            m_bookInfo->lastReadLink = qs_bookMark;
        }
        else
        {
            qs_bookMark = "";
        }
    }
    else
    {
        qs_bookMark = m_bookInfo->lastReadLink;
    }

    qDebug() << Q_FUNC_INFO << "Contenido de Bookmarks-->" << qs_bookMark;

    if(m_docView)
    {
        // TODO: handle the lines bellow inside gotoBookmark function
        m_docView->setBlockPaintEvents(true);
        m_docView->gotoBookmark(qs_bookMark);
        m_docView->setBlockPaintEvents(false);
        if(getCurrentDocExt() == EXT_PDF) m_docView->takeMiniatureScreenshot();

        Screen::getInstance()->setMode(Screen::MODE_AUTO,true,Q_FUNC_INFO);
    }
}

void Viewer::setPageChanged (int start, int, int count)
{
    qDebug() << Q_FUNC_INFO;
    int lastPage = m_bookInfo->lastReadPage;
    m_bookInfo->lastReadPage = start;
    m_bookInfo->pageCount = count;
    showTimeTitleLabels();

    if(m_timeOnPage.isValid())
    {
        int secsToCurrentTime = m_timeOnPage.secsTo(QDateTime::currentDateTimeUtc());
        if(secsToCurrentTime > MAX_SECS_FOR_STEP) // Only add 5 minutes for each screen.
        {
            m_bookInfo->setReadingPeriod(m_bookInfo->getReadingPeriod() + MAX_SECS_FOR_STEP);
            finalStepTimestamp = initialStepTimestamp + MAX_MSECS_FOR_STEP;
        }else
        {
            m_bookInfo->setReadingPeriod(m_bookInfo->getReadingPeriod() + secsToCurrentTime);
            finalStepTimestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
        }

        if((finalStepPos > 0) && (secsToCurrentTime > mSecsNeedToReadStep))
            registerStepRead();
    }
    initialStepPos = m_docView->getInitialPosFromRenderer();
    finalStepPos = m_docView->getFinalPosFromRenderer();
    updateTimeOnPage();
    calculateTimeForStep();
    if (lastPage != start+1 && lastPage != start-1)
    {
        setCurrentChapterInfo();
        return;
    }
    else if(lastPage == start-1)
        m_pagesLeft--;
    else
        m_pagesLeft++;
    pageWindow->setChapterPage(m_pagesLeft);
    pageWindowLandscape->setChapterPage(m_pagesLeft);
    if(m_pagesLeft <= 0 || m_pagesLeft > m_totalChapterPages)
        setCurrentChapterInfo();
}

void Viewer::setMark(BookLocation* location)
{
    m_markHandler->setCurrentMark(location);
}

bool Viewer::checkChapterInfoAvailability()
{
    if (m_docView->tableOfContent()) return true;

    pageWindow->hideChapterInfo();
    pageWindowLandscape->hideChapterInfo();
    m_viewerMenu->hideBar();
    return false;
}

void Viewer::setCurrentChapterInfo()
{
    qDebug() << Q_FUNC_INFO;

    if (!checkChapterInfoAvailability()) return;

    if(isUsingCR3((SupportedExt)getCurrentDocExt()))
        m_bookIndex->calculatePageForChapters();
    const QList<QDocView::Location*>& contentList = m_bookIndex->getContentList();
    int count = contentList.size();
    currentChapter = false;
    for(int i = 0; i < count; ++i)
    {
        if(!currentChapter) {
            QDocView::Location *location = contentList.at(i);
            if(location)
            {
                if(i == 0 && location->page > m_bookInfo->lastReadPage) {
                    m_chapterTitle = location->preview;
                    m_pagesLeft = location->page - m_bookInfo->lastReadPage;
                    m_pageStart = location->page;
                    m_totalChapterPages = location->page;
                    m_viewerMenu->setCurrentChapter(m_chapterTitle, m_pagesLeft);
                    pageWindow->setChapterPage(m_pagesLeft);
                    pageWindowLandscape->setChapterPage(m_pagesLeft);
                    currentChapter = true;
                    break;
                }
                //We found a location to the last page of the book but is not the last item in the index, so we ignore it.
                if(location->page >= m_bookInfo->pageCount && i < count - 1) {
                    qDebug() << Q_FUNC_INFO << "**************" << "ITEM apunta al final del libro y no es el último.";
                    continue;
                }

                if (location->page > m_bookInfo->lastReadPage && i > 0) {
                    m_pagesLeft = 0;
                    for(int j = i -1; j >= 0; j--){
                        if(contentList.at(j)) {
                            m_chapterTitle = contentList.at(j)->preview;
                            m_pagesLeft = location->page - m_bookInfo->lastReadPage;
                            m_totalChapterPages = location->page - contentList.at(j)->page;
                            m_pageStart = contentList.at(j)->page;
                            break;
                        }
                        else if(j==0) {
                            m_chapterTitle = " ";
                            m_totalChapterPages = location->page;
                            m_pagesLeft = location->page - m_bookInfo->lastReadPage;;
                        }
                    }
                    pageWindow->setChapterPage(m_pagesLeft);
                    pageWindowLandscape->setChapterPage(m_pagesLeft);
                    m_viewerMenu->setCurrentChapter(m_chapterTitle, m_pagesLeft);
                    currentChapter = true;
                    break;
                }
                else if (i == count - 1) {
                    m_chapterTitle = location->preview;
                    m_totalChapterPages = m_bookInfo->pageCount - location->page;
                    m_pagesLeft = m_bookInfo->pageCount - m_bookInfo->lastReadPage;
                    m_pageStart = location->page;
                    pageWindow->setChapterPage(m_pagesLeft);
                    pageWindowLandscape->setChapterPage(m_pagesLeft);
                    m_viewerMenu->setCurrentChapter(m_chapterTitle, m_pagesLeft);
                    currentChapter = true;
                    break;
                }
            }
        }
    }
    // There is a toc but the toc is empty so dont do anything in the loop.
    // In this case we need to hide the info bar.
    if(!currentChapter)
    {
        m_viewerMenu->hideBar();
        pageWindow->hideChapterInfo();
        pageWindowLandscape->hideChapterInfo();
    }
}

void Viewer::goToPage(int pageNumber)
{
    m_powerLock->activate();
    backBtnHandling();
    docView()->gotoPage(pageNumber);
    m_powerLock->release();
}

void Viewer::goPageBack()
{
    if(!history.isEmpty()) {
        qDebug() << Q_FUNC_INFO;
        QString last = history.pop();
        goToMarkup(last);
        if(history.isEmpty())
        {
            pageWindow->setNeedToPaint(true);
            pageWindow->hideBackBtn();
            pageWindowLandscape->setNeedToPaint(true);
            pageWindowLandscape->hideBackBtn();
        }
        else
            m_screenSteps = 0;
    }
}

void Viewer::pdfMenuBtnClicked()
{
    pageWindow->setPdfToolbarState(true);
    pageWindowLandscape->setPdfToolbarState(true);

    if(m_docView->isHorizontal())
    {
        pageWindowLandscape->hide();
        pdfToolsWindowLandscape->show();
        miniatureLandscape->show();
    }
    else
    {        
        pageWindow->hide();
        pdfToolsWindow->show();
        miniature->show();
    }
}

void Viewer::handlePageModeReflow()
{
    pdfToolsWindowLandscape->hide();
    pageWindowLandscape->hide();
    miniatureLandscape->hide();
    viewerHeaderLandscape->hide();
    ViewerContLandscape->hide();
    viewerHeader->show();
    ViewerCont->show();
    pageWindowLandscape->setPdfToolbarState(false);
    pageWindowLandscape->setNeedToPaint(true);
    pageWindowLandscape->updateDisplay();
    pageWindow->setPdfToolbarState(false);
    pageWindow->setNeedToPaint(true);
    pageWindow->updateDisplay();
    setTextBodyMargins(m_bookInfo);
}

void Viewer::handleLandscapeMode()
{
    qDebug() << Q_FUNC_INFO;

    if(!m_docView) return;

    Screen::getInstance()->queueUpdates();

    if (m_docView->sizeLevel() > 0) m_docView->setBlockPaintEvents(true);

    m_viewerMenu->hide();

    bool goingToLandscapeMode = false;

    if(m_docView->isHorizontal())
    {
        viewerHeader->show();
        bookMarkCont->show();
        viewerHeaderLandscape->hide();

        if(pdfToolsWindowLandscape->isVisible())
        {
            pdfToolsWindow->show();
            miniature->show();
            miniatureLandscape->hide();
            pageWindow->hide();
            pageWindow->setPdfToolbarState(true);
        }
        else
        {
            pdfToolsWindow->hide();
            miniature->hide();
            miniatureLandscape->hide();
            pageWindow->setNeedToPaint(true);
            pageWindow->show();
            pageWindow->setPdfToolbarState(false);
        }

        ViewerCont->show();
        ViewerContLandscape->hide();
    }
    else // LANDSCAPE MODE
    {
        goingToLandscapeMode = true;
        viewerHeader->hide();
        bookMarkCont->hide();
        viewerHeaderLandscape->show();

        if(pdfToolsWindow->isVisible())
        {
            miniature->hide();
            miniatureLandscape->show();
            pdfToolsWindow->hide();
            pdfToolsWindowLandscape->show();
            pageWindowLandscape->hide();
            pageWindowLandscape->setPdfToolbarState(true);
        }
        else
        {
            pageWindow->hide();
            pdfToolsWindowLandscape->hide();
            pageWindowLandscape->setNeedToPaint(true);
            pageWindowLandscape->show();
            pageWindowLandscape->setPdfToolbarState(false);
        }

        ViewerCont->hide();
        ViewerContLandscape->show();
    }

    setTextBodyMargins(goingToLandscapeMode);
    setHorizontal(goingToLandscapeMode);

    QBookApp::instance()->getStatusBar()->hide();    
    m_docView->takeMiniatureScreenshot();    
    updateZoom();
    m_docView->update(); // FIXME: allow refresh in SD.
    m_docView->positioningMiniature();

    m_docView->setBlockPaintEvents(false);
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void Viewer::closePdfToolsWindow()
{
    qDebug() << Q_FUNC_INFO;

    pageWindow->setPdfToolbarState(false);
    pageWindowLandscape->setPdfToolbarState(false);

    if(m_docView->isHorizontal())
    {        
        pdfToolsWindowLandscape->hide();
        miniatureLandscape->hide();
        pageWindowLandscape->setNeedToPaint(true);
        pageWindowLandscape->updateDisplay();
    }
    else
    {        
        pdfToolsWindow->hide();
        miniature->hide();
        pageWindow->setNeedToPaint(true);
        pageWindow->updateDisplay();
    }
}

void Viewer::screenAdjust()
{
    m_powerLock->activate();
    m_docView->displayFit(QDocView::AUTO_FIT_BEST);
    m_docView->update(); // FIXME: allow refresh in SD.
    m_powerLock->release();
}

void Viewer::widthAdjust()
{
    m_powerLock->activate();
    m_docView->displayFit(QDocView::AUTO_FIT_WIDTH);
    m_docView->update(); // FIXME: allow refresh in SD.
    m_powerLock->release();
}

void Viewer::heightAdjust()
{
    m_powerLock->activate();
    m_docView->displayFit(QDocView::AUTO_FIT_HEIGHT);
    m_docView->update(); // FIXME: allow refresh in SD.
    m_powerLock->release();
}

void Viewer::setHorizontal(bool on)
{
    if (!m_docView->isHardModePDF()) return;

    m_docView->setHorizontal(on);
    if (on) m_bookInfo->orientation = BookInfo::ORI_LANDSCAPE;
    else    m_bookInfo->orientation = BookInfo::ORI_PORTRAIT;
}

void Viewer::numPageMarks( int /*notesCount*/, int /*highlightsCount*/ )
{
    qDebug() << Q_FUNC_INFO;

    // IMPORTANT: We are setting all the notes in the doc. The docView, has to handle which ones are shown.

    qDebug() << "highlightCount: " << m_docView->highlightCount();
    QStringList highlists = m_docView->highlightList();

    QList<int> docViewNotes;

    QStringList::const_iterator itH = highlists.constBegin();
    QStringList::const_iterator itHEnd = highlists.constEnd();
    for(int i = 0; itH != itHEnd; ++itH, ++i)
    {
        BookLocation* loc = m_bookInfo->getLocation(*itH);
        if( loc && loc->type == BookLocation::NOTE )
        {
            docViewNotes.append(i);
        }
    }

    // Set the list
    m_docView->setNoteList(docViewNotes);
}

#ifndef HACKERS_EDITION
void Viewer::buyBook()
{
    qDebug() << Q_FUNC_INFO;
    QString shopBookUrl = QBook::settings().value("serviceURLs/book").toString() + m_bookInfo->isbn;
    QBookApp::instance()->openStoreWithUrl(shopBookUrl);
}
#endif

void Viewer::resumeWifi()
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->resumeWifi();
}

void Viewer::handleBookmark()
{
    if (noWidgetShown()) m_markHandler->handleBookmark();
}

void Viewer::resetDocView()
{
    qDebug() << Q_FUNC_INFO;
    if(m_docView){
        delete m_docView;
        m_docView = NULL;
    }
    qs_docPath.clear();
}

void Viewer::setMargin( int topPercentage, int rightPercentage, int bottomPercentage, int leftPercentage )
{
    qDebug() << Q_FUNC_INFO;

    double h = textBody->height();
    double w = textBody->width();

    double tm = (h * topPercentage) / 100.0;
    double rm = (w * rightPercentage) / 100.0;

    double bm ;

    //TODO: check this piece of code
    /* WorkAround to fix problem scaling images higher than 1000px*/
    switch(QBook::getInstance()->getResolution())
    {
        case QBook::RES1072x1448:
            bm = (h * bottomPercentage) / 100.0; // -15;
            break;
        case QBook::RES758x1024:
            bm = (h * bottomPercentage) / 100.0; // -15;
            break;
        case QBook::RES600x800: default:
            bm = (h * bottomPercentage) / 100.0;
            break;
    }

    double lm = (w * leftPercentage) / 100.0;

    qDebug() << Q_FUNC_INFO << "TextBody dimensions " << textBody->x() << textBody->y() << textBody->width() << textBody->height();
    qDebug() << Q_FUNC_INFO << "Setting margins to " << "t: " << tm << ", b: " << bm << ", l:" << lm << ", r: " << rm;

    // NOTE: The margins are set in pixels
    m_docView->setMargin(tm, rm, bm, lm);
}

void Viewer::disconnectWifi()
{
    qDebug() << Q_FUNC_INFO;

    if(QBookApp::instance()->getCurrentForm() != this) return;

    disconnect(QBookApp::instance(), SIGNAL(periodicSyncFinished()), this, SLOT(disconnectWifi()));
    disconnect(QBookApp::instance(), SIGNAL(resumingWifiFailed()),   this, SLOT(disconnectWifi()));

    if (!QBook::settings().value("wifi/disabled",false).toBool() || ConnectionManager::getInstance()->isConnected())
        QBookApp::instance()->powerOffWifiAsync();/* Wifi behaviour with viewer (Wifi off entering, wifi on leaving)*/
    else
        QBookApp::instance()->getStatusBar()->hideWifiCont();
}

void Viewer::showBookmark(bool show)
{    
    ViewerBookmark* mark = 0;

    if (m_docView->isHorizontal()) mark = bookmarkLandscape;
    else                           mark = bookmark;

    if (!mark) return;

    if (show) mark->raise();
    mark->updateDisplay(show);
}

void Viewer::menuPopUpShow( ViewerMenuPopUp* popup, bool showStatusBar )
{
    qDebug() << Q_FUNC_INFO;
    if(popup != m_currentMenuPopup)
    {
        Screen::getInstance()->queueUpdates();

        // Stop previous popup
        if(m_currentMenuPopup)
        {
            qDebug() << Q_FUNC_INFO << "Hiding current popup: " << m_currentMenuPopup;
            m_currentMenuPopup->stop();
            m_currentMenuPopup->hide();
        }

        m_currentMenuPopup = popup;

        m_docView->setBlockPaintEvents(false);
        if(showStatusBar)
        {
            QBookApp::instance()->getStatusBar()->show();
            QBookApp::instance()->getStatusBar()->raise();
        }
        m_currentMenuPopup->show();
        m_currentMenuPopup->raise();

        m_currentMenuPopup->start();

        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
    }
}

void Viewer::menuPopUpHide()
{
    qDebug() << Q_FUNC_INFO;

    if(m_currentMenuPopup)
    {
        Screen::getInstance()->queueUpdates();

        m_currentMenuPopup->stop();
        m_currentMenuPopup->hide();

        if(m_docView)
            m_docView->setBlockPaintEvents(false);

        m_currentMenuPopup = NULL;

        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
    }
}

void Viewer::paintEvent( QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void Viewer::performSearchRequest( const QString& text )
{
    qDebug() << Q_FUNC_INFO << m_searchPopup;
    menuPopUpShow(m_searchPopup, false);
    Screen::getInstance()->flushUpdates();
    m_searchPopup->searchRequest(text);
}

void Viewer::setTimestamp(qint64 timestamp)
{
    qDebug() <<Q_FUNC_INFO;
    if(m_bookInfo)
        m_bookInfo->timestamp = timestamp;
}

void Viewer::prepareViewerBeforeSleep()
{
    qDebug() << Q_FUNC_INFO;

    if(!m_docView) return;

    m_docView->setBlockPaintEvents(false);
    m_searchPopup->stopSearch();
    m_markHandler->cancelMarkOp();
    m_powerLock->release();

    if(m_timeOnPage.isValid())
    {
        int secsToCurrentTime = m_timeOnPage.secsTo(QDateTime::currentDateTimeUtc());
        if(secsToCurrentTime > MAX_SECS_FOR_STEP) // Only add 5 minutes for each screen.
            finalStepTimestamp = initialStepTimestamp + MAX_MSECS_FOR_STEP;
        else
            finalStepTimestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();

        if(secsToCurrentTime > mSecsNeedToReadStep)
            registerStepRead();
    }

    m_bookInfo->appendPercent(m_viewerStepsManager->getStringFromRanges());
}

void Viewer::calculateTimeForStep()
{
    qDebug() << Q_FUNC_INFO;
    mSecsNeedToReadStep = m_viewerStepsManager->getNeededMsecsToBeRead(initialStepPos,finalStepPos);
    qDebug() << Q_FUNC_INFO << "mSecsNeedToReadStep: " << mSecsNeedToReadStep;
}

void Viewer::updateTimeOnPage()
{
    qDebug() <<Q_FUNC_INFO;
    initialStepTimestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    m_timeOnPage = QDateTime::currentDateTimeUtc();
}

void Viewer::pushHistory()
{
    qDebug() << Q_FUNC_INFO;

    if(history.isEmpty())
    {
        pageWindow->showBackBtn();
        pageWindowLandscape->showBackBtn();
    }
    QDocView::Location* loc = m_docView->bookmark();
    if(loc)
    {
        history.push(loc->ref);
        delete loc;
    }
}

void Viewer::calculateLocationsChapter()
{
    qDebug() << Q_FUNC_INFO;
    QHash<QString, BookLocation*> locations = m_bookInfo->getLocations();
    QHash<QString, BookLocation*>::const_iterator it = locations.begin();
    QHash<QString, BookLocation*>::const_iterator itEnd = locations.end();

    while(it != itEnd)
    {
        if(!(*it)->chapterName.size())
            (*it)->chapterName = setChapterForLocation((*it)->pos);

        if((*it)->page <= 0)
            it.value()->page = m_docView->getPageFromMark(it.value()->bookmark);

        it++;
    }
}

QString Viewer::setChapterForLocation(double locationPos)
{
    qDebug() << Q_FUNC_INFO;
    const QList<QDocView::Location*>& contentList = m_bookIndex->getContentList();
    int count = contentList.size();

    for(int i = 0; i < count; ++i)
    {
        if(i < count - 1)
        {
            if((locationPos > contentList.at(i)->pos) && (locationPos < contentList.at(i+1)->pos))
                return contentList.at(i)->preview;
        }else
            return contentList.at(i)->preview;
    }
    return "";
}

void Viewer::deleteCurrentBookInfo()
{
    delete m_bookInfo;
    m_bookInfo = NULL;
}

void Viewer::checkAndExtractCover()
{
    qDebug() << Q_FUNC_INFO;

    QFileInfo thumbnailFile(m_bookInfo->thumbnail);
    bool coverToDownscale = !thumbnailFile.completeBaseName().endsWith(THUMBNAIL_SUFIX)
                            && !m_bookInfo->thumbnail.endsWith(NO_COVER_RESOURCE)
                            && !m_bookInfo->path.contains(Storage::getInstance()->getPrivatePartition()->getMountPoint())
                            && !m_bookInfo->corrupted;

    // Extract thumbnail if empty yet or old (big size)
    if(m_bookInfo->thumbnail.isEmpty() || coverToDownscale)
        QBookApp::instance()->generateBookCover(m_bookInfo);
}

void Viewer::repaintContentList()
{
    hideKeyboard();
    m_bookIndex->repaint();
}

#ifndef HACKERS_EDITION
void Viewer::setInitialDefinitionSearch(const QString& word, const QString& context)
{
    m_viewerDictionary->setInitialSearch(word, context);
}

void Viewer::resetInitialDefinitionSearch()
{
    m_viewerDictionary->resetInitialSearch();
}
#endif

void Viewer::resetViewerBookInfo()
{
    qDebug() << Q_FUNC_INFO;
    resetDocView();
    delete m_bookInfo;
    m_bookInfo = NULL;
}
void Viewer::clearStepsProcess()
{
    qDebug() << Q_FUNC_INFO;
    initialStepTimestamp=0;
    initialStepPos=0;
    m_timeOnPage=QDateTime();
    finalStepTimestamp=0;
    finalStepPos=0;
}

void Viewer::showTimeTitleLabels()
{
    if(m_bookInfo->lastReadPage == 0)
    {
        titleLbl->hide();
        pageWindowLandscape->hideTitle();
        dateTimeLbl->hide();
        pageWindowLandscape->hideDateTime();
        m_shownYet = false;
    }
    else //if(!m_shownYet) // TODO: control only one refresh.
    {
        if(m_showTitle)
        {
            if(m_docView->isHorizontal())
            {
                pageWindowLandscape->updateBookTitle(m_bookInfo->title);
                pageWindowLandscape->showTitle();
            }
            else
            {
                titleLbl->setText(bqUtils::truncateStringToLength(m_bookInfo->title, STRING_MAX_LENGTH));
                titleLbl->show();
            }
        }
        else
        {
            titleLbl->hide();
            pageWindowLandscape->hideTitle();
        }

        if(m_showDateTime)
        {
            QString time = QTime::currentTime().toString("hh:mm");
            if(m_docView->isHorizontal())
            {
                pageWindowLandscape->updateDateTime(time);
                pageWindowLandscape->showDateTime();
            }
            else
            {
                dateTimeLbl->setText(time);
                dateTimeLbl->show();
            }
        }
        else
        {
            dateTimeLbl->hide();
            pageWindowLandscape->hideDateTime();
        }
        m_shownYet = true;
    }
}
void Viewer::updateTime()
{
    qDebug() << Q_FUNC_INFO;

    if(dateTimeLbl->isVisible() || pageWindowLandscape->dateTimeisVisible())
    {
        m_updateTimeTimer.start();
        Screen::getInstance()->queueUpdates();
        QString time = QTime::currentTime().toString("hh:mm");

        if(m_docView->isHorizontal())
        {
            pageWindowLandscape->updateDateTime(time);
            pageWindowLandscape->showDateTime();
        }
        else
        {
            dateTimeLbl->setText(time);
            dateTimeLbl->show();
        }

        Screen::getInstance()->flushUpdates();
    }
}

bool Viewer::titleShouldBeShown()
{
    return QBook::settings().value("setting/showBookTitle", QVariant(false)).toBool();
}

bool Viewer::dateTimeShouldBeShown()
{
    return QBook::settings().value("setting/showDateTime", QVariant(false)).toBool();
}

bool Viewer::hasChangedOptions()
{
    if(!m_showTitle && !m_showDateTime)
        return (titleShouldBeShown() || dateTimeShouldBeShown());
    else
        return (!titleShouldBeShown() && !dateTimeShouldBeShown());

}

bool Viewer::headerShouldBeShown()
{
    return (QBook::settings().value("setting/showBookTitle", QVariant(false)).toBool() ||
            QBook::settings().value("setting/showDateTime", QVariant(false)).toBool() ||
            m_bookInfo->m_type == BookInfo::BOOKINFO_TYPE_DEMO);
}

void Viewer::setTextBodyMargins(BookInfo* bookInfo)
{
    setTextBodyMargins(i_docExtension == EXT_PDF && bookInfo->orientation == BookInfo::ORI_LANDSCAPE);
}

void Viewer::setTextBodyMargins(bool isPdfModeLandscape)
{
    QMargins margins(0, 0, 0, 0);

    /// LEFT
    if(isPdfModeLandscape)
    {
        margins.setLeft(pageWindowLandscape->width());
    }
    else
    {
        /// TOP
        if (headerShouldBeShown())
        {
            if (i_docExtension == EXT_PDF) margins.setTop(viewerHeader->height());
            else                           margins.setTop(viewerHeader->height()/2);


        }

        /// BOTTOM
        if (isUsingCR3((SupportedExt) i_docExtension)) margins.setBottom(pageWindow->height()*0.85);
        else                                           margins.setBottom(pageWindow->height());
    }

    emit upperMarginUpdate(margins.top());
    textBody->layout()->setContentsMargins(margins);
    if (m_docView) m_docView->setFormActivated(true);
}

int Viewer::getUpperMargin()
{
    qDebug() << Q_FUNC_INFO;
    QMargins margins = textBody->layout()->contentsMargins();
    return margins.top();
}

void Viewer::updatePdfMiniatureScreenshot(QPixmap& screenshot)
{
    miniature->updatePdfMiniatureScreenshot(screenshot);
    miniatureLandscape->updatePdfMiniatureScreenshot(screenshot);
}

void Viewer::updatePdfMiniatureLocation(double xoPercent, double yoPercent, double xfPerdcent, double yfPercent)
{
    miniature->setFrameGeometry(xoPercent, yoPercent, xfPerdcent, yfPercent);
    miniatureLandscape->setFrameGeometry(xoPercent, yoPercent, xfPerdcent, yfPercent);
}

void Viewer::enablePdfMode(bool enable)
{
    // Update pageHandler
    enable = enable && getCurrentDocExt() == Viewer::EXT_PDF;

    if (!enable)
    {
        if (pdfToolsWindow->isVisible())
        {
            miniature->hide();
            miniatureLandscape->hide();
            pdfToolsWindow->hide();
            pdfToolsWindowLandscape->hide();
            pageWindow->setPdfToolbarState(false);
            pageWindowLandscape->setPdfToolbarState(false);
        }

        if (m_docView->isHorizontal())
        {
            // Back to non-landscape mode.            
            setHorizontal(false);
            QBookApp::instance()->getStatusBar()->hide();
            resetScreen();
        }
    }

    pageWindow->setCurrentPageMode(enable);
    pageWindowLandscape->setCurrentPageMode(enable);
}

bool Viewer::isPdfBtnAllowed()
{
    if (m_docView && m_bookInfo) return m_bookInfo->pageMode != QDocView::MODE_REFLOW_PAGES && getCurrentDocExt() == Viewer::EXT_PDF;
    else return false;
}

void Viewer::updateZoom()
{
    m_docView->updateScaleByLevel();
}

void Viewer::setChapterPosInfo()
{
    qDebug() << Q_FUNC_INFO;
    const QList<QDocView::Location*>& contentList = m_bookIndex->getContentList();
    int count = contentList.size();
    QList<int> chaptersPos;
    for(int i = 0; i < count; i++)
    {
        QDocView::Location *location = contentList.at(i);
        if(location)
        {
            int pos = location->page;
            chaptersPos << pos;
        }
    }
    pageWindow->setChapterPos(chaptersPos);
    pageWindowLandscape->setChapterPos(chaptersPos);
}

