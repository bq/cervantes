/*************************************************************************

bq Cervantes e-book reader application
Copyright (C) 2011-2013  Mundoreader, S.L

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

#include "Browser.h"

#include "BrowserNetworkAccessManager.h"
#include "BrowserInputContext.h"
#include "BrowserWebPage.h"
#include "BrowserFavo.h"
#ifndef DISABLE_ADOBE_SDK
#include "BrowserAdobeDrm.h"
#include "AdobeDRM.h"
#endif
#include "ConfirmDialog.h"
#include "InfoDialog.h"
#include "SelectionDialog.h"
#include "Keyboard.h"
#include "QBook.h"
#include "QBookApp.h"
#include "Storage.h"
#include "Screen.h"
#include "PowerManager.h"
#include "Viewer.h"
#include "Model.h"
#include "BookInfo.h"
#include "MetaDataExtractor.h"
#include "FullScreenWidget.h"
#include "ProgressDialog.h"
#include "bqUtils.h"


#ifndef HACKERS_EDITION
#include "bqDeviceServices.h"
#endif

#include <QWebElement>
#include <QRect>
#include <QWebFrame>
#include <QWebHistory>
#include <QWebHitTestResult>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QList>
#include <QKeyEvent>

#define ZOOM_MAX            3
#define ZOOM_MIN            1
#define ZOOM_STEP           1.5
#define SCROLL_CURSOR_THICKNESS 15
#define GETDOWNLOAD_TIMEOUT 15000

Browser::Browser(QWidget *parent)
        : QBookForm(parent)
        , mScrollMoveTimes(0)
        , progress(0)
        , mDisableBigBarMode(true)
        , bContentDisposition(false)
        , b_alreadyVisitedBrowser(false)
        , mSelectWebEdit(false)
        , m_downloadDialog(NULL)
        , replyNew(NULL)
        , newKeyboard(0)
        , m_favo(NULL)
        , m_waitingDialog(NULL)
        , m_infoDialogAccessFailed(NULL)
        , m_connectionDialog(NULL)
        , bytesToDownload(0)
        , bytesDownloaded(0)
{
    qDebug() << Q_FUNC_INFO;
    mDefaultURL = QBook::etcDirPath() + QDir::separator() + QString("loading.html");
    mHomeURL =  QString("http://www.google.com/?hl=" + QBook::settings().value("setting/language", QVariant("es")).toString());

    m_powerLock = PowerManager::getNewLock(this);
    setupUi(this);
    initBrowserParams();

    connect(webview, SIGNAL(wheel(bool,int)), SLOT(doWheel(bool,int)));

    BrowserWebPage *pPage = new BrowserWebPage(this, false);
    webview->setPage(pPage);
    connect(pPage, SIGNAL(tryDownload(const QUrl &)), SLOT(download(const QUrl &)));

    m_namgr = new BrowserNetworkAccessManager(this);
    connect(m_namgr,SIGNAL(sslErrors(QNetworkReply*,const QList<QSslError>&)),this,SLOT(errorListener(QNetworkReply*,const QList<QSslError>&)));
    webview->page()->setNetworkAccessManager(m_namgr);

    mInputContext = new BrowserInputContext(this);
    webview->setInputContext(mInputContext);

    progressTimer = new QTimer();

    m_favo = new BrowserFavo(this);
#ifndef DISABLE_ADOBE_SDK
    browserDrm = new BrowserAdobeDrm();
#endif
    m_favo->hide();
    connect(m_favo, SIGNAL(myfavoclick(QString)), this, SLOT(handleMyfavoPress(QString)));
    connect(m_favo, SIGNAL(hideMe()), this, SLOT(hideElement()));

    connect(webview, SIGNAL(titleChanged(const QString&)), SLOT(adjustTitle()));
    connect(webview, SIGNAL(loadProgress(int)), SLOT(setProgress(int)));
    connect(webview, SIGNAL(loadFinished (bool)), this, SLOT(setTextCursor(bool)));

    connect(url, SIGNAL(clicked()),this, SLOT(showKeyboard()));
    url->setStyleSheet("QPushButton{text-align : left; background-image: url(:/QBook/white)}");
    connect(clearSearchBtn, SIGNAL(clicked()), this, SLOT(clearText()));

    webview->page()->setForwardUnsupportedContent(true);
    connect(webview->page(), SIGNAL(unsupportedContent(QNetworkReply *)), SLOT(unsupportedContentProcess(QNetworkReply *)));

    connect(forwardbtn, SIGNAL(pressed()),  webview, SLOT(forward()));
    connect(reloadbtn,  SIGNAL(pressed()),  webview, SLOT(reload()));
    connect(stopbtn,    SIGNAL(pressed()),  webview, SLOT(stop()));

    updateZoomBtnStates();
    mScale = webview->zoomFactor();
    initialWebViewBars();
    resetScrolls();

    connect(webview, SIGNAL(loadStarted()),            this, SLOT(handleLoadStart()));
    connect(webview, SIGNAL(newPressPos(QPoint)),      this, SLOT(handleViewPressPos(QPoint)));
    connect(webview, SIGNAL(newReleasePos(QPoint)),    this, SLOT(handleViewReleasePos( )));
    connect(webview, SIGNAL(urlChanged(const QUrl &)), this, SLOT(handleURLChanged(const QUrl &)));

#ifdef Q_WS_QWS
        path = Storage::getInstance()->getDataPartition()->getMountPoint() + "/webfavo.sqlite";
#endif

    QFile fileSpecific(":/res/browser_styles.qss");
    QFile fileCommons(":/res/browser_styles_generic.qss");
    fileSpecific.open(QFile::ReadOnly);
    fileCommons.open(QFile::ReadOnly);

    QString styles = QLatin1String(fileSpecific.readAll() + fileCommons.readAll());
    setStyleSheet(styles);
}

Browser::~Browser()
{
    qDebug() << Q_FUNC_INFO;
    delete m_powerLock;
    m_powerLock = NULL;

    delete m_favo;
    m_favo = NULL;
}

void Browser::initBrowserParams()
{
    qDebug() << Q_FUNC_INFO;
    QWebSettings::setMaximumPagesInCache(3);
    QWebSettings::setObjectCacheCapacities(32 * 1024, 32 * 1024, 32 * 1024);
    QWebSettings::setOfflineStorageDefaultQuota(0);
    QWebSettings* settings = QWebSettings::globalSettings();

    settings->setFontSize(QWebSettings::DefaultFontSize, 14);
    settings->setFontFamily(QWebSettings::StandardFont, "Lato");
    settings->setFontFamily(QWebSettings::FixedFont, "Lato");
    settings->setFontFamily(QWebSettings::SerifFont, "Lato");
    settings->setFontFamily(QWebSettings::SansSerifFont, "Lato");

    settings->setAttribute(QWebSettings::JavaEnabled, true);
    settings->setAttribute(QWebSettings::PluginsEnabled, true);
    settings->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);
    settings->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
    settings->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    settings->setAttribute(QWebSettings::LinksIncludedInFocusChain, true);
    settings->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
    settings->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, true);
    settings->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, true);
    settings->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);

    connect(backbtn,    SIGNAL(pressed()),  this,       SLOT(goBack()));
    connect(addfavobtn, SIGNAL(clicked()),  this,       SLOT(addtofavo()));
    connect(favobtn,    SIGNAL(clicked()),  this,       SLOT(showfavo()));
    connect(zoomInBtn,  SIGNAL(pressed()),  this,       SLOT(handleZoomIn()));
    connect(zoomOutBtn, SIGNAL(pressed()),  this,       SLOT(handleZoomOut()));
    connect(homeBtn,    SIGNAL(pressed()),  this,       SLOT(handleHomeBtnPress()));
    connect(Go,         SIGNAL(clicked()),  this,       SLOT(processUrl()));

    connect(verticalScrollBar,      SIGNAL(valueChanged(int)),  this,   SLOT(handleVScrollBarValueChanged(int)));
    connect(horizontalScrollBar,    SIGNAL(valueChanged(int)),  this,   SLOT(handleHScrollBarValueChanged(int)));
    horizontalScrollBar->setMinimum(0);
    horizontalScrollBar->setMaximum(0);
    verticalScrollBar->setMinimum(0);
    verticalScrollBar->setMaximum(0);
    verticalScrollFrame->hide();
    horizontalScrollFrame->hide();
    spacerButton->hide();
    backbtn->setEnabled(false);
    forwardbtn->setEnabled(false);

    connect(vScrollUpButton,    SIGNAL(pressed()), SLOT(handleScrollUpBtnPress()));
    connect(vScrollDownButton,  SIGNAL(pressed()), SLOT(handleScrollDownBtnPress()));
    connect(hScrollLeftButton,  SIGNAL(pressed()), SLOT(handleScrollLeftBtnPress()));
    connect(hScrollRightButton, SIGNAL(pressed()), SLOT(handleScrollRightBtnPress()));
}

void Browser::showfavo()
{
    qDebug() << Q_FUNC_INFO;
    if(m_favo)
        hideElement();

    m_favo->setup();
    showElement(m_favo);
}

void Browser::handleMyfavoPress(QString clickurl)
{
    qDebug() << Q_FUNC_INFO << clickurl;
    QString sURL = clickurl;
    if (sURL=="http://local/index.html")
    {
        qDebug() << Q_FUNC_INFO << " clickurl: "<< clickurl << ", mDefaultURL: " << mDefaultURL;
        handleHomeBtnPress();
    }
    else
    {
        qDebug() << Q_FUNC_INFO << " clickurl: "<< clickurl << ", mDefaultURL: " << mDefaultURL;
        webview->load(QUrl(sURL));
        setURLHint(sURL);
        adjustTitle();
    }
}

void Browser::addtofavo()
{
    qDebug() << Q_FUNC_INFO;

    QUrl qUrl  = webview->url();
    QString url = qUrl.toEncoded();
    url.replace(QRegExp("[(][0-9]+%[)]$"), "");
    url = url.trimmed();
    QString s = webview->title();

    m_favo->addFavo(url, s);
}

void Browser::handleURLChanged(const QUrl & url )
{
    qDebug() << Q_FUNC_INFO << " Url: " << url;
    QBookApp::instance()->hideKeyboard();
    setURLHint(url.toString());
    updateButtonsState();
    adjustTitle();
}

void Browser::setURLHint(const QString & url)
{
    qDebug() << Q_FUNC_INFO << url;
    mCurrectURL = url;
    primitiveUrl = url;
}

void Browser::resetScrolls()
{
    qDebug() << Q_FUNC_INFO;
    verticalScrollBar->setValue(0);
    verticalScrollBar->setSingleStep(15);
    verticalScrollFrame->hide();

    horizontalScrollBar->setValue(0);
    horizontalScrollBar->setSingleStep(15);
    horizontalScrollFrame->hide();
    spacerButton->hide();
    mLTViewPos = QPoint(0,0);
    mVPageStep = 0;
    mVMax = 0;
    mHPageStep = 0;
    mHMax = 0;
    mHShow = false;
    mVShow = false;
    mPrevScrollPos =  QPoint(0,0);
    mNewScrollPos =  QPoint(0,0);
}

void Browser::handleLoadStart()
{
    qDebug() << Q_FUNC_INFO;

    if(!ConnectionManager::getInstance()->isConnected())
        QBookApp::instance()->requestConnection();

    initialWebViewBars();
    mScrollMoveTimes = 0;
    resetScrolls();
}

void Browser::handleZoomIn()
{
    qDebug() << Q_FUNC_INFO;
    qreal newScale = mScale* ZOOM_STEP;
    if( newScale > ZOOM_MAX)
        return;

    mScale = newScale;
    updateZoomBtnStates();
    webview->setZoomFactor( mScale);
    updateBars(webview->size());
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
}

void Browser::handleZoomOut()
{
    qDebug() << Q_FUNC_INFO;
    qreal newScale = mScale / ZOOM_STEP;

    if( newScale < ZOOM_MIN)
        return;

    mScale = newScale;
    updateZoomBtnStates();
    webview->setZoomFactor( newScale);
    updateBars(webview->size());
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
}

void Browser::updateZoomBtnStates()
{
    qDebug() << Q_FUNC_INFO;
    if( (mScale/ZOOM_STEP) < ZOOM_MIN)
        zoomOutBtn->setEnabled(false);
    else
        zoomOutBtn->setEnabled(true);

    if( (mScale*ZOOM_STEP) > ZOOM_MAX)
        zoomInBtn->setEnabled(false);
    else
        zoomInBtn->setEnabled(true);
}

void Browser::handleVScrollBarValueChanged(int nNewScrollValue)
{
    qDebug() << Q_FUNC_INFO;
    mScrollMoveTimes++;

    mPrevScrollPos = webview->page()->currentFrame()->scrollPosition();
    int nDifY = verticalScrollBar->value() - mLTViewPos.y();
    nDifY = nNewScrollValue - mLTViewPos.y();
    webview->page()->currentFrame()->scroll(0,nDifY);
    mNewScrollPos = webview->page()->currentFrame()->scrollPosition();
    mLTViewPos.setY( nNewScrollValue);
}

void Browser::handleHScrollBarValueChanged(int nNewScrollValue)
{
    qDebug() << Q_FUNC_INFO;
    mScrollMoveTimes++;

    mPrevScrollPos = webview->page()->currentFrame()->scrollPosition();
    int nDifX = horizontalScrollBar->value() - mLTViewPos.x();
    nDifX = nNewScrollValue - mLTViewPos.x();
    webview->page()->currentFrame()->scroll(nDifX,0);
    mNewScrollPos = webview->page()->currentFrame()->scrollPosition();
    mLTViewPos.setX( nNewScrollValue);
}

void Browser::showKeyboard()
{
    qDebug() << Q_FUNC_INFO;
    newKeyboard = QBookApp::instance()->showKeyboard(tr("Ir"));
    newKeyboard->disconnect();
    url->setText("http://");
    newKeyboard->handleMyQLineEdit(url);
    connect(newKeyboard, SIGNAL(actionRequested()), this, SLOT(processUrl()));
}

void Browser::clearText()
{
    qDebug() << Q_FUNC_INFO;
    url->clear();
    url->setText("http://");
}

void Browser::processUrl()
{
    QString sURL = url->text();
    sURL.replace(QRegExp(" \\(\\d*%\\)"), "");

    QUrl url = QUrl::fromEncoded(sURL.toUtf8());
    webview->load(url);
    setURLHint(sURL);
    adjustTitle();
}

void Browser::unsupportedContentProcess(QNetworkReply *reply)
{
    qDebug() << "--->" << Q_FUNC_INFO;
    bContentDisposition = false;
    if (reply->hasRawHeader("Content-Disposition"))
    {
        cd_fileName = QString();
        const QString value = QLatin1String(reply->rawHeader("Content-Disposition"));
        const int pos = value.indexOf(QLatin1String("filename="));

        if (pos != -1)
        {
             QString name = value.mid(pos + 9);
             if(name.startsWith(QLatin1Char('"')) && name.endsWith(QLatin1Char('"')))
                name = name.mid(1, name.size() - 2);
             else if(name.startsWith(QLatin1Char('"')) && name.endsWith(QLatin1Char(';')))
                name = name.mid(1, name.size() - 3);

             if(name.split(";").size() > 0 )
                 name = name.split(";")[0];

             name = name.replace("\"", "");
             cd_fileName = name;
        }

        if(!cd_fileName.isEmpty())
            bContentDisposition = true;
    }
    download(reply->url());
}

void Browser::getFile(const QString& fileName)
{
    qDebug() << Q_FUNC_INFO << "fileName: " << fileName;
    m_powerLock->activate();

    if(!m_downloadDialog){
        m_downloadDialog = new ProgressDialog(this,tr("Downloading file..."));
        m_downloadDialog->setHideBtn(false);
        m_downloadDialog->setTextValue(false);
        connect(this, SIGNAL(downloadProgress(int)), m_downloadDialog, SLOT(setProgressBar(int)));
        connect(m_downloadDialog, SIGNAL(cancel()),  this,             SLOT(downloadCanceled()));
    }

    m_downloadDialog->setProgressBar(0);
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_PARTIALSCREEN_UPDATE,Q_FUNC_INFO);

    QString filePath = Storage::getInstance()->getPublicPartition()->getMountPoint() + QDir::separator() + fileName;
    m_file.setFileName(filePath);
    if (!m_file.open(QIODevice::WriteOnly))
    {
        delete m_downloadDialog;
        m_downloadDialog = NULL;
        return;
    }

    m_downloadDialog->show();
    QNetworkRequest request;
    request.setUrl(m_url);
    replyNew = webview->page()->networkAccessManager()->get(request);
    connect(replyNew, SIGNAL(finished()), this, SLOT(downloadComplete()), Qt::UniqueConnection);
    connect(replyNew, SIGNAL(readyRead()), this, SLOT(readData()), Qt::UniqueConnection);
    connect(replyNew, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(handleDownloadProgress(qint64, qint64)), Qt::UniqueConnection);
    connect(replyNew, SIGNAL(error(QNetworkReply::NetworkError)),this, SLOT(handleDownLoadError(QNetworkReply::NetworkError)), Qt::UniqueConnection);
    timer.start();
    disconnect(progressTimer, SIGNAL(timeout()), this, SLOT(checkConnectionToExit()));
    disconnect(progressTimer, SIGNAL(timeout()), progressTimer, SLOT(stop()));
}

void Browser::downloadComplete()
{
    qDebug() << Q_FUNC_INFO;
    delete m_downloadDialog;
    m_downloadDialog = NULL;
    m_file.close();
    processDownload();
    m_powerLock->release();
}

void Browser::readData()
{
    qDebug() << Q_FUNC_INFO;

    qint64 availableBytes = replyNew->bytesAvailable();
    timer.start();
    bytesDownloaded += availableBytes;

    if(bytesToDownload <= 0)
    {
        bytesToDownload = replyNew->header(QNetworkRequest::ContentLengthHeader).toULongLong();
        if(bytesToDownload <= 0)
            m_downloadDialog->hideProgressBar();
    }

    qDebug() << "bbytesToDownload: " << bytesToDownload;
    if(Storage::getInstance()->getFreeInternalMemory() < bytesToDownload)
    {
        downloadCanceled();
        m_waitingDialog = new InfoDialog(this,tr("Not enough space to download."));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_PARTIALSCREEN_UPDATE,Q_FUNC_INFO);
        m_waitingDialog->showForSpecifiedTime();
        delete m_waitingDialog;
        m_waitingDialog = NULL;
    }

    while(availableBytes > 0 && timer.elapsed() < GETDOWNLOAD_TIMEOUT)
    {
        m_file.write(replyNew->read(128));
        availableBytes = availableBytes - 128;
    }

    if(timer.elapsed() > GETDOWNLOAD_TIMEOUT)
    {
        downloadCanceled();
        m_waitingDialog = new InfoDialog(this,tr("Download file failed"));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_PARTIALSCREEN_UPDATE,Q_FUNC_INFO);
        m_waitingDialog->showForSpecifiedTime();
        delete m_waitingDialog;
        m_waitingDialog = NULL;
    }
}

void Browser::handleDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    qDebug() << Q_FUNC_INFO << "received:" << bytesReceived << "total:"<< bytesTotal;
    int dialogProgress = int((100 * float(bytesReceived)) / bytesTotal);
    if(m_downloadDialog)
        m_downloadDialog->setProgressBar(dialogProgress);

    /* restart the progress timer */
    timer.start();
}

void Browser::handleDownLoadError(QNetworkReply::NetworkError code)
{
    qDebug() << Q_FUNC_INFO << "code: " << (int)code;
    m_file.close();
    m_file.remove();

    m_waitingDialog = new InfoDialog(this,tr("Download file failed"));
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_PARTIALSCREEN_UPDATE,Q_FUNC_INFO);
    m_waitingDialog->showForSpecifiedTime();
    delete m_waitingDialog;
    m_waitingDialog = NULL;
    m_powerLock->release();
}

void Browser::stopDownload()
{
    qDebug() << Q_FUNC_INFO;
    if(replyNew)
    {
        disconnect(replyNew, SIGNAL(readyRead()), this, SLOT(readData()));
        disconnect(replyNew, SIGNAL(finished()), this, SLOT(downloadComplete()));
        disconnect(replyNew, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(handleDownloadProgress(qint64, qint64)));
        disconnect(replyNew, SIGNAL(error(QNetworkReply::NetworkError)),this, SLOT(handleDownLoadError(QNetworkReply::NetworkError)));
        replyNew->abort();
    }

    if(m_downloadDialog)
    {
        Screen::getInstance()->queueUpdates();
        delete m_downloadDialog;
        m_downloadDialog = NULL;
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
    }
}

void Browser::downloadCanceled()
{
    qDebug() << Q_FUNC_INFO;
    stopDownload();
    m_file.close();
    m_file.remove();
}

void Browser::processDownload()
{
    qDebug() << Q_FUNC_INFO << "name: " << m_file.fileName();

#ifndef DISABLE_ADOBE_SDK
    if(m_file.fileName().toLower().endsWith(".acsm"))
    {
        if(AdobeDRM::getInstance()->isLinked())
        {
            m_powerLock->activate();
            browserDrm->processFulFillment(m_file);
            m_powerLock->release();
        }
        else
        {
            if(!m_waitingDialog)
            {
                m_waitingDialog = new InfoDialog(this,tr("No hay credenciales de Adobe DRM asociados a este dispositivo."));
                Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_PARTIALSCREEN_UPDATE,Q_FUNC_INFO);
                m_waitingDialog->showForSpecifiedTime();
            }
        }
    }else 
#endif
     if(QBookApp::instance()->getViewer()->isSupportedFile(m_file.fileName()))
    {

        QString filePath = m_file.fileName();

        SelectionDialog* dialogSelect = new SelectionDialog(this,tr("Open downloaded book?"));
        connect(QBookApp::instance(),SIGNAL(askingPowerOff()),dialogSelect,SLOT(doReject()));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        Screen::getInstance()->resetQueue();
        dialogSelect->exec();

        bool openBookReq = dialogSelect->result();
        delete dialogSelect;

        const BookInfo* bookInfo = QBookApp::instance()->getModel()->getBookInfo(filePath);
        if(bookInfo)
        {
            if(openBookReq){
                emit openBook(bookInfo);
                return;
            }
        }else{
            QBookApp::instance()->stopThumbnailGeneration();
            BookInfo* book = new BookInfo(filePath);
            populateBookInfo(book);

            if(openBookReq)
                book->lastTimeRead = QDateTime::currentDateTime();

            QBookApp::instance()->getModel()->addBook(book);
            if(openBookReq)
            {
                qDebug() << Q_FUNC_INFO << "opening book";
                emit openBook(book);
            }

            QBookApp::instance()->syncModel();
            QBookApp::instance()->resumeThumbnailGeneration();
        }
    }else{
        m_waitingDialog = new InfoDialog(this,tr("File downloaded correctly"));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_PARTIALSCREEN_UPDATE,Q_FUNC_INFO);
        m_waitingDialog->showForSpecifiedTime();
    }

    if(m_waitingDialog)
    {
        delete m_waitingDialog;
        m_waitingDialog = NULL;
    }
}

void Browser::adjustTitle()
{
    qDebug() << Q_FUNC_INFO << primitiveUrl;
    QString sTitle = primitiveUrl;
    qDebug() << Q_FUNC_INFO << "sTitle: " << sTitle;
    url->setText(sTitle);

    if(progress < 100)
        url->setText(QString("%1 (%2%)").arg(sTitle).arg(progress));
    else
        url->setText(QString("%1").arg(sTitle));
}

void Browser::setProgress(int p)
{
    qDebug() << Q_FUNC_INFO << p <<"% loaded " << webview->url().toEncoded();
    m_powerLock->activate();
    if( p == 100 )
    {
        m_powerLock->release();
        setURLHint(webview->url().toEncoded());

        QBookApp::instance()->getStatusBar()->setSpinner(false);

        if(progressTimer->isActive())
            progressTimer->stop();
    }
    else
        QBookApp::instance()->getStatusBar()->setSpinner(true);

    qDebug() << Q_FUNC_INFO << "Progress is " << progress << ", p is " << p;
    progress = p;
    adjustTitle();
}

void Browser::activateForm()
{
    qDebug() << Q_FUNC_INFO;

    if(!ConnectionManager::getInstance()->isConnected())
        return;

    activateFormWithConnection();
}

void Browser::activateFormWithConnection()
{
    qDebug() << Q_FUNC_INFO;
    m_powerLock->activate();
    connect(progressTimer, SIGNAL(timeout()), this, SLOT(checkConnectionToExit()));
    connect(progressTimer, SIGNAL(timeout()), progressTimer, SLOT(stop()));
    progressTimer->start(10 * 1000);

    if(!s_forced.isEmpty())
    {
        qDebug() << Q_FUNC_INFO << "forced url";
        loadURL(s_forced);
        s_forced.clear();
    }else
    {
        if(!b_alreadyVisitedBrowser)  // First visit in session, loads home page
        {
            loadURL(mHomeURL);
            b_alreadyVisitedBrowser = true;
        }
    }
}

void Browser::checkConnectionToExit()
{
    qDebug() << Q_FUNC_INFO << "progress:"  << progress;
    disconnect(progressTimer, SIGNAL(timeout()), this, SLOT(checkConnectionToExit()));   

    if(progressTimer->isActive())
        progressTimer->stop();

    if(m_powerLock->isActive())
        m_powerLock->release();

    if(!m_connectionDialog)
        m_connectionDialog = new InfoDialog(this, tr("Not connected. Please try it again in a few minutes"));

    if(progress <= 10 && m_connectionDialog)
    {
        m_connectionDialog->hideSpinner();
        m_connectionDialog->showForSpecifiedTime();

        Screen::getInstance()->queueUpdates();
        stopDownload();
        emit hideMe();

        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
        return;
    }
}

void Browser::showElement(QWidget * widget)
{
    qDebug() << "--->" << Q_FUNC_INFO << ": " << widget;
    Screen::getInstance()->queueUpdates();

    // Show widgets
    widget->show();
    widget->raise();

    widget->setFocus();

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void Browser::hideElement()
{
    qDebug() << "--->" << Q_FUNC_INFO;


    QBookApp::instance()->hideKeyboard();
    if( m_favo && m_favo->isVisible() )
    {
        Screen::getInstance()->queueUpdates();
        m_favo->hide();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
    }

}

void Browser::deactivateForm()
{
    qDebug() << Q_FUNC_INFO << "QWebView deactivated";
    delete m_infoDialogAccessFailed;
    m_infoDialogAccessFailed = NULL;
    delete m_connectionDialog;
    m_connectionDialog = NULL;

    if(m_powerLock->isActive())
        m_powerLock->release();

    if(progressTimer->isActive())
        progressTimer->stop();

    stopDownload();

    hideElement();

    QBookApp::instance()->getStatusBar()->setSpinner(false);
}

void Browser::errorListener( QNetworkReply * reply, const QList<QSslError> & errors )
{
    qDebug() << Q_FUNC_INFO;
    reply->ignoreSslErrors();

    for(int i=0 ; i < errors.count() ; i++)
        qDebug()<<"SSL Error Code = "<< errors[i].error();
}

void Browser::updateStatus()
{
    if(mCurrectURL.length() <= 0 )
        mCurrectURL = mDefaultURL;

    if(mCurrectURL.length() <= 0)
    {
        // Still no URL, we load the local one
        qDebug() << Q_FUNC_INFO << "mCurrectURL is empty, loading " << QBook::etcDirPath() + QString("/loading.html");
        webview->load(QUrl::fromLocalFile(QBook::etcDirPath() + QString("/loading.html")));
        return;
    }
    qDebug() << Q_FUNC_INFO << "ok";
}

void Browser::updateBars(QSize sWebViewSize)
{
    qDebug() << Q_FUNC_INFO;

    if(mDisableBigBarMode)
        return;

    webview->page()->setViewportSize(sWebViewSize);
    QSize newViewport = webview->page()->viewportSize();
    QSize sFrame = webview->page()->mainFrame()->contentsSize();
    spacerButton->hide();
    if(newViewport.width() < sFrame.width()){
        horizontalScrollFrame->show();
        mHShow = true;
    }
    else{
        horizontalScrollFrame->hide();
        mHShow =false;
    }
    newViewport = webview->page()->viewportSize();
    sFrame = webview->page()->mainFrame()->contentsSize();

    if(newViewport.height() < sFrame.height()){
        verticalScrollFrame->show();
        mVShow = true;
    }
    else{
        verticalScrollFrame->hide();
        mVShow = false;
    }
    newViewport = webview->page()->viewportSize();
    sFrame = webview->page()->mainFrame()->contentsSize();

    if(mHShow && mVShow)
        spacerButton->show();
    else
        spacerButton->hide();

    newViewport = webview->page()->viewportSize();
    sFrame = webview->page()->mainFrame()->contentsSize();
    adjustScrollItemPos();
    QSize sViewPort = webview->page()->viewportSize();
    if(mHShow){
        horizontalScrollBar->setMaximum(sFrame.width()- sViewPort.width());
        horizontalScrollBar->setPageStep(sViewPort.width());
        horizontalScrollBar->setValue(webview->page()->mainFrame()->scrollPosition().x());
        mLTViewPos.setX( webview->page()->mainFrame()->scrollPosition().x());
        mHPageStep = sViewPort.width();
        mHMax = sFrame.width();
    }
    else{
        mHPageStep = 0;
        mHMax = 0;
    }
    if(mVShow){
        verticalScrollBar->setMaximum(sFrame.height() - sViewPort.height());
        verticalScrollBar->setPageStep(sViewPort.height());
        verticalScrollBar->setValue(webview->page()->mainFrame()->scrollPosition().y());
        mLTViewPos.setY( webview->page()->mainFrame()->scrollPosition().y());

        mVPageStep = sViewPort.height();
        mVMax = sFrame.height();
    }
    else{
        mVPageStep = 0;
        mVMax = 0;
    }
}

void Browser::adjustScrollItemPos()
{
    if(!horizontalScrollFrame->isHidden())
        adjustHorizontalScrollItemPos();

    if(!verticalScrollFrame->isHidden())
        adjustVerticalScrollItemPos();
}

void Browser::adjustHorizontalScrollItemPos()
{
    qDebug() << Q_FUNC_INFO;
    QRect sRect = horizontalScrollFrame->rect();
    QPoint oriPos(0,0);
    QPoint newPos(0,0);
    QSize oriSize(0,0);
    QSize newSize(0,0);
    int newX = 0;
    int newWidth = 0;
    int scrollCursorBtnWidth = SCROLL_CURSOR_THICKNESS;

    horizontalScrollBar->stackUnder(hScrollRightButton);
    horizontalScrollBar->stackUnder(hScrollLeftButton);
    oriPos = horizontalScrollBar->pos();
    newX = ( hScrollLeftButton->minimumWidth()) - scrollCursorBtnWidth;
    newPos = oriPos;
    newPos.setX(newX);

    if( newPos != oriPos)
        horizontalScrollBar->move(newPos);

    oriSize = horizontalScrollBar->size();
    newSize = horizontalScrollFrame->size();
    newWidth = newSize.width();
    newWidth -= (hScrollLeftButton->minimumWidth() + hScrollRightButton->minimumWidth());
    newWidth += (2*scrollCursorBtnWidth);
    newSize.setWidth(newWidth);

    if(newSize.width() != oriSize.width())
        horizontalScrollBar->resize(newSize);

    oriPos = hScrollLeftButton->pos();
    newX =  0;
    newPos = oriPos;
    newPos.setX(newX);

    if( newPos != oriPos)
        hScrollLeftButton->move(newPos);

    oriPos = hScrollRightButton->pos();
    newX = sRect.width()- hScrollRightButton->minimumWidth();
    newPos = oriPos;
    newPos.setX(newX);

    if( newPos != oriPos)
        hScrollRightButton->move(newPos);
}

void Browser::adjustVerticalScrollItemPos()
{
    qDebug() << Q_FUNC_INFO;
    QRect sRect = verticalScrollFrame->rect();
    QPoint oriPos(0,0);
    QPoint newPos(0,0);
    QSize oriSize(0,0);
    QSize newSize(0,0);
    int newY = 0;
    int newHeight = 0;
    int scrollCursorBtnHeight = SCROLL_CURSOR_THICKNESS;

    verticalScrollBar->stackUnder(vScrollUpButton);
    verticalScrollBar->stackUnder(vScrollDownButton);
    oriPos = verticalScrollBar->pos();
    newY = ( vScrollUpButton->minimumHeight()) - scrollCursorBtnHeight;
    newPos = oriPos;
    newPos.setY(newY);

    if( newPos != oriPos)
        verticalScrollBar->move(newPos);

    oriSize = verticalScrollBar->size();
    newSize = verticalScrollFrame->size();
    newHeight = newSize.height();
    newHeight -= (vScrollUpButton->minimumHeight() + vScrollDownButton->minimumHeight());
    newHeight += (2*scrollCursorBtnHeight);
    newSize.setHeight(newHeight);

    if(newSize.height() != oriSize.height())
        verticalScrollBar->resize(newSize);

    oriPos = vScrollUpButton->pos();
    newY =  0;
    newPos = oriPos;
    newPos.setY(newY);

    if( newPos != oriPos)
        vScrollUpButton->move(newPos);

    oriPos = vScrollDownButton->pos();
    newY = sRect.height()- vScrollDownButton->minimumHeight();
    newPos = oriPos;
    newPos.setY(newY);

    if( newPos != oriPos)
        vScrollDownButton->move(newPos);
}

void Browser::handleViewPressPos(QPoint pt)
{
    QWebHitTestResult aResult = webview->page()->currentFrame()->hitTestContent(pt);
    qDebug() << Q_FUNC_INFO;
    if(aResult.isContentEditable())
    {
        qDebug()<< Q_FUNC_INFO << "true";
        mSelectWebEdit= true;
        m_selectedElementPos = pt;
        //TODO: Workaround
        aResult.element().setStyleProperty("font-family", "noexiste");
    }
    else
        closeKeyBoardDialog();
}

void Browser::handleViewReleasePos()
{
    qDebug() << Q_FUNC_INFO;
    if(mSelectWebEdit){
        qDebug() << Q_FUNC_INFO << "true";
        mSelectWebEdit = false;
        QTimer::singleShot(100, this, SLOT(showKeyBoardDialog()));
    }
}

void Browser::showKeyBoardDialog()
{
    qDebug() << Q_FUNC_INFO;

    bool showKeyboardUp = m_selectedElementPos.y() > 450;
    newKeyboard = QBookApp::instance()->showKeyboard(tr("Go"),showKeyboardUp);
    newKeyboard->justSendSignals();
    connect(newKeyboard,    SIGNAL(pressedChar(const QString &)),   this, SLOT(sendKey(const QString &)),Qt::UniqueConnection);
    connect(newKeyboard,    SIGNAL(backSpacePressed()),             this, SLOT(sendBackSpace()),Qt::UniqueConnection);
    connect(newKeyboard,    SIGNAL(cursorLeftPressed()),            this, SLOT(sendCursorLeft()), Qt::UniqueConnection);
    connect(newKeyboard,    SIGNAL(cursorRightPressed()),           this, SLOT(sendCursorRight()), Qt::UniqueConnection);
    connect(newKeyboard,    SIGNAL(newLinePressed()),               this, SLOT(sendReturn()),Qt::UniqueConnection);
    connect(newKeyboard,    SIGNAL(newLinePressed()),               this, SLOT(closeKeyBoardDialog()),Qt::UniqueConnection);
    connect(newKeyboard,    SIGNAL(actionRequested()),              this, SLOT(sendReturn()),Qt::UniqueConnection);
    connect(newKeyboard,    SIGNAL(actionRequested()),              this, SLOT(closeKeyBoardDialog()));
    Screen::getInstance()->refreshScreen();
}

void Browser::sendKey(const QString &key)
{
    qDebug() << Q_FUNC_INFO;
    QKeyEvent press(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, key);
    QApplication::sendEvent(webview, &press);
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_A, Qt::NoModifier, key);
    QApplication::sendEvent(webview, &release);
}

void Browser::sendBackSpace()
{
    qDebug() << Q_FUNC_INFO;
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
    QApplication::sendEvent(webview, &press);
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Backspace, Qt::NoModifier);
    QApplication::sendEvent(webview, &release);
}

void Browser::sendCursorLeft()
{
    qDebug() << Q_FUNC_INFO;
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QApplication::sendEvent(webview, &press);
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Left, Qt::NoModifier);
    QApplication::sendEvent(webview, &release);
}

void Browser::sendCursorRight()
{
    qDebug() << Q_FUNC_INFO;
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QApplication::sendEvent(webview, &press);
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Right, Qt::NoModifier);
    QApplication::sendEvent(webview, &release);
}

void Browser::sendReturn()
{
    qDebug() << Q_FUNC_INFO;
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, "\n");
    QApplication::sendEvent(webview, &press);
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Return, Qt::NoModifier, "\n");
    QApplication::sendEvent(webview, &release);
}

void Browser::closeKeyBoardDialog()
{
    qDebug() << Q_FUNC_INFO;
    QBookApp::instance()->hideKeyboard();
}

void Browser::handleScrollUpBtnPress()
{
    qDebug() << Q_FUNC_INFO;
    verticalScrollBar->triggerAction(QAbstractSlider::SliderSingleStepSub);
}

void Browser::handleScrollDownBtnPress()
{
    qDebug() << Q_FUNC_INFO;
    verticalScrollBar->triggerAction(QAbstractSlider::SliderSingleStepAdd);
}

void Browser::handleScrollLeftBtnPress()
{
    qDebug() << Q_FUNC_INFO;
    horizontalScrollBar->triggerAction(QAbstractSlider::SliderSingleStepSub);
}

void Browser::handleScrollRightBtnPress()
{
    qDebug() << Q_FUNC_INFO;
    horizontalScrollBar->triggerAction(QAbstractSlider::SliderSingleStepAdd);
}

void Browser::updateButtonsState()
{
    qDebug() << Q_FUNC_INFO;

    if( webview->history()->backItems(1).count() >0)
        backbtn->setEnabled(true);
    else
        backbtn->setEnabled(false);

    if( webview->history()->forwardItems(1).count() > 0)
        forwardbtn->setEnabled(true);
    else
        forwardbtn->setEnabled(false);
}

void Browser::download(const QUrl & url)
{
    qDebug() << Q_FUNC_INFO << "url: " << url;
    m_url = url;
    QString localFileName = QFileInfo(m_url.path()).fileName();
    qDebug() << Q_FUNC_INFO << "localFileName: " << localFileName;

    if(bContentDisposition)
        localFileName = cd_fileName;

    if(QBookApp::instance()->getViewer()->isSupportedFile(localFileName)
        || localFileName.contains(".acsm",Qt::CaseInsensitive))
    {
        qDebug() << Q_FUNC_INFO << "localFileName: " << localFileName;
        getFile(localFileName.replace("?", ""));
    }
    else
    {
        qDebug() << Q_FUNC_INFO << "url: " << url;
        m_waitingDialog = new InfoDialog(this,tr("Unsupported download link"));
        m_waitingDialog->showForSpecifiedTime();
        delete m_waitingDialog;
        m_waitingDialog = NULL;
        bContentDisposition = false;
    }
}

void Browser::doWheel(bool bVertical,int nDis)
{
    qDebug()<< Q_FUNC_INFO << " nDic: " << nDis<<",bVertical="<<bVertical;
    int nPos = 0;
    if(bVertical)
    {
        nPos = verticalScrollBar->sliderPosition();
        nPos += nDis;
        verticalScrollBar->setSliderPosition(nPos);
    }else
    {
        nPos = horizontalScrollBar->sliderPosition();
        nPos += nDis;
        horizontalScrollBar->setSliderPosition(nPos);
    }
}

void Browser::handleHomeBtnPress()
{
    qDebug() << Q_FUNC_INFO << mHomeURL;
    QString sURL = mHomeURL;
    webview->load(QUrl(sURL));
    setURLHint(sURL);
    adjustTitle();
}

void Browser::initialWebViewBars()
{
    qDebug()<<Q_FUNC_INFO;
    mDisableBigBarMode = true;
    QWebFrame *pMainFrame = webview->page()->mainFrame();
    QList<QWebFrame*> mainChildFrames = pMainFrame->childFrames();
    int nCount = mainChildFrames.size();
    for(int i=0;i<nCount;i++){
        mainChildFrames[i]->setScrollBarPolicy(Qt::Horizontal,Qt::ScrollBarAsNeeded);
        mainChildFrames[i]->setScrollBarPolicy(Qt::Vertical,Qt::ScrollBarAsNeeded);
    }
    pMainFrame->setScrollBarPolicy(Qt::Horizontal,Qt::ScrollBarAsNeeded);
    pMainFrame->setScrollBarPolicy(Qt::Vertical,Qt::ScrollBarAsNeeded);
    spacerButton->hide();
    verticalScrollFrame->hide();
    horizontalScrollFrame->hide();
}

void Browser::loadURL(QString sURL )
{
      qDebug() << "--->" << Q_FUNC_INFO << sURL;
      webview->load(QUrl(sURL));
      setURLHint(sURL);
      adjustTitle();
}

void Browser::setSimpleView()
{
     browserBar->hide();
     webViewFooterCont->hide();
     horizontalScrollFrame->hide();
     spacerButton->hide();
     return;
}

void Browser::forceUrl(const QString& url)
{
    s_forced = QString(url);
    mCurrectURL = QString(url);
}

void Browser::setTextCursor(bool ok)
{
    qDebug() << Q_FUNC_INFO;
    url->setCursorPosition ( ok );
    Screen::getInstance()->refreshScreen();
}

void Browser::keyReleaseEvent(QKeyEvent* event)
{
     qDebug() << Q_FUNC_INFO << event;

     if(event->key() == QBook::QKEY_MENU) {

         // Hides keyboard ignoring its refresh. The menu refresh will do
         Screen::getInstance()->queueUpdates();
         QBookApp::instance()->hideKeyboard();
         hideElement();
         Screen::getInstance()->setMode(Screen::MODE_BLOCK,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
         Screen::getInstance()->flushUpdates();
     }

     if(event->key() == QBook::QKEY_BACK){

         Screen::getInstance()->queueUpdates();

         if(newKeyboard && newKeyboard->isVisible())
             QBookApp::instance()->hideKeyboard();
         else
             goBack();

         Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
         Screen::getInstance()->flushUpdates();

         event->accept();
     }
     else
         QBookForm::keyReleaseEvent(event);
}

void Browser::goBack()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    if(webview->history()->canGoBack())
    {
        qDebug() << Q_FUNC_INFO << "elements: " << webview->history()->count();//WORKAROUND
        webview->back();
    }
    else
        emit hideMe();

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
 }

bool Browser::eventFilter(QObject* watched, QEvent* event)
{
     Q_UNUSED(watched);

     if (event->type() == QEvent::KeyPress || event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease)	{
         qDebug() << Q_FUNC_INFO << "type " << event->type();
         if(b_blocked){
             qDebug() << Q_FUNC_INFO << "Blocked";
             return true;
         }
     }
     return false;
}

void Browser::populateBookInfo(BookInfo* book)
{
     qDebug() << Q_FUNC_INFO;
     if(!book)
        return;
     QString collection;
     MetaDataExtractor::getMetaData(book->path, book->title, book->author, book->publisher, book->publishTime, book->synopsis, book->format, book->isDRMFile, collection, book->language);
     book->corrupted = false;
     if(!collection.isEmpty())
         book->addCollection(collection);
     if (book->format != "pdf")
          book->fontSize = 2;
}
