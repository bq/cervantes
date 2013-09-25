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

#include "WebStore.h"
#include "BrowserNetworkAccessManager.h"
#include "BrowserInputContext.h"
#include "BrowserWebPage.h"

#include "PowerManager.h"
#include "Screen.h"
#include "Keyboard.h"
#include "QBook.h"
#include "QBookApp.h"
#include "Storage.h"
#include "bqDeviceServices.h"

#include "ConfirmDialog.h"
#include "InfoDialog.h"
#include "SelectionDialog.h"
#include "ProgressDialog.h"
#include "BookInfo.h"
#include "Model.h"

#include <QWebElement>
#include <QWebHistory>
#include <QWebHitTestResult>
#include <QTimer>
#include <QList>
#include <QKeyEvent>
#include <QTimer>

#define REFRESH_COUNTER     2
#define TIME_TO_CONNECT_WIFI 15000

WebStore::WebStore(QWidget *parent) : QBookForm(parent)
        , dirty(REFRESH_COUNTER)
        , progress(0)
        , b_alreadyVisitedStore(false)
        , mSelectWebEdit(false)
        , newKeyboard(NULL)
        , m_infoDialogAccessFailed(NULL)
        , m_connectionDialog(NULL)
        , pDialog(NULL)
{
    m_powerLock = PowerManager::getNewLock(this);

    QWebSettings::setMaximumPagesInCache(3);
    QWebSettings::setObjectCacheCapacities(32 * 1024, 32 * 1024, 32 * 1024);
    QWebSettings::setOfflineStorageDefaultQuota(0);
    QWebSettings* settings = QWebSettings::globalSettings();

    settings->setAttribute(QWebSettings::JavaEnabled, false);
    settings->setAttribute(QWebSettings::PluginsEnabled, false);
    settings->setAttribute(QWebSettings::JavascriptCanOpenWindows, false);
    settings->setAttribute(QWebSettings::JavascriptCanAccessClipboard, false);
    settings->setAttribute(QWebSettings::DeveloperExtrasEnabled, false);
    settings->setAttribute(QWebSettings::LinksIncludedInFocusChain, false);
    settings->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, false);
    settings->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, false);
    settings->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, false);
    setupUi(this);

    BrowserWebPage *pPage = new BrowserWebPage(this, true);
    webview->setPage(pPage);

    m_namgr = new BrowserNetworkAccessManager(this);
    webview->page()->setNetworkAccessManager(m_namgr);
    connect(m_namgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(processHeaders(QNetworkReply*)));
    connect(m_namgr, SIGNAL( sslErrors ( QNetworkReply*, const QList<QSslError>&)), this, SLOT(errorListener(QNetworkReply*, const QList<QSslError>&)));

    url->setStyleSheet("QPushButton{text-align : left; background-image: url(:/QBook/white)}");
    webview->page()->setForwardUnsupportedContent(true);

    backbtn->setEnabled(false);
    forwardbtn->setEnabled(false);
    spacerButton->hide();
    browserBar->hide();
    webViewFooterCont->hide();

    connect(webview, SIGNAL(loadStarted()),                 SLOT(handleLoadStart()));
    connect(webview, SIGNAL(newPressPos(QPoint)),           SLOT(handleViewPressPos(QPoint)));
    connect(webview, SIGNAL(newReleasePos(QPoint)),         SLOT(handleViewReleasePos()));
    connect(webview, SIGNAL(urlChanged(const QUrl &)),      SLOT(handleURLChanged(const QUrl &)));
    connect(webview, SIGNAL(loadProgress(int)),             SLOT(setProgress(int)));
    connect(webview, SIGNAL(loadFinished (bool)),           SLOT(setTextCursor()));
    connect(webview->page(), SIGNAL(loadFinished(bool)),    SLOT(loadFinished(bool)));

    verticalScrollFrame->hide();
    horizontalScrollFrame->hide();

    mInputContext = new BrowserInputContext(this);
    webview->setInputContext(mInputContext);

    progressTimer = new QTimer();

    QFile fileSpecific(":/res/bqwebstore_styles.qss");
    QFile fileCommons(":/res/bqwebstore_styles_generic.qss");
    qDebug() << fileSpecific.open(QFile::ReadOnly);
    qDebug() << fileCommons.open(QFile::ReadOnly);

    QString styles = QLatin1String(fileSpecific.readAll() + fileCommons.readAll());
    setStyleSheet(styles);
}

void WebStore::loadFinished(bool)
{
    qDebug() << Q_FUNC_INFO;
    if(--dirty == 0)
    {
        dirty = REFRESH_COUNTER;
        Screen::getInstance()->refreshScreen();
    }
}

void WebStore::handleURLChanged(const QUrl & url )
{
    qDebug() << Q_FUNC_INFO << " Url: " << url;
    QString header= url.toString();
    qDebug() << Q_FUNC_INFO << " header: " << header;
    mCurrectURL = url.toString();
}

void WebStore::paintEvent(QPaintEvent* e)
{
    qDebug() << Q_FUNC_INFO;
    QWidget::paintEvent(e);
}

void WebStore::handleLoadStart()
{
    qDebug() << Q_FUNC_INFO;

    if(!ConnectionManager::getInstance()->isConnected())
    {
        connect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(connectionCanceled()), Qt::UniqueConnection);
        connectWifiObserverNavigateToUrl();
        QBookApp::instance()->requestConnection();
    }
}

void WebStore::connectWifiObserverNavigateToUrl()
{
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(loadURL()), Qt::UniqueConnection);
}


void WebStore::disconnectWifiObserverNavigateToUrl()
{
    disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(loadURL()));
}

WebStore::~WebStore()
{
    qDebug() << Q_FUNC_INFO;

    delete m_powerLock;
    m_powerLock = NULL;

    delete pDialog;
    pDialog = NULL;
}

void WebStore::setProgress(int p)
{
    qDebug() << Q_FUNC_INFO << p <<"% loaded " << webview->url().toEncoded();
    m_powerLock->activate();

    if (m_wifiConnectionCanceled)
        return;

    if( p == 100 )
    {
        m_powerLock->release();
        mCurrectURL = webview->url().toEncoded();
        QBookApp::instance()->getStatusBar()->setSpinner(false);

        if(progressTimer->isActive())
            progressTimer->stop();
    }
    else
        QBookApp::instance()->getStatusBar()->setSpinner(true);

    qDebug() << Q_FUNC_INFO << "Progress is " << progress << ", p is " << p;
    progress = p;
}

void WebStore::activateForm()
{
    qDebug() << Q_FUNC_INFO;
    QBookApp::instance()->setBuying(true);
    QBookApp::instance()->getStatusBar()->setMenuTitle(tr("Tienda"));

    connect(progressTimer, SIGNAL(timeout()), this, SLOT(checkConnectionToExit()), Qt::UniqueConnection);
    progressTimer->start(TIME_TO_CONNECT_WIFI);

}

void WebStore::activateFormWithConnection()
{
    qDebug() << Q_FUNC_INFO;
    if(progressTimer->isActive())
        progressTimer->stop();

    disconnect(progressTimer, SIGNAL(timeout()), this, SLOT(checkConnectionToExit()));

    QString url;
    QString* token = new QString();
    services = QBookApp::instance()->getDeviceServices();

    if(!m_infoDialogAccessFailed)
        m_infoDialogAccessFailed = new InfoDialog(this,tr("The access to the store failed. Please try again in a few minutes"));

    m_powerLock->activate();
    bool bRet = services->getToken(token);
    m_powerLock->release();

    if(!bRet)
    {
        if(m_infoDialogAccessFailed)
        {
            m_infoDialogAccessFailed->showForSpecifiedTime();
            delete token;
            emit hideMe();
        }
        return;
    }

    dirty--;
    if(!s_forced.isEmpty())
    {
        qDebug() << Q_FUNC_INFO << "forced url";
        if(!s_forced.contains("token"))
            s_forced.append(QString("&token=") + *token);

        loadURL(s_forced);
        s_forced.clear();
    }

    if(!b_alreadyVisitedStore) // First visit in session, loads index
    {
        qDebug() << Q_FUNC_INFO << "first visit in session";
        b_alreadyVisitedStore = true;
        url = QBook::settings().value("shopUrl").toString();

        if(!url.contains("token"))
            url = url.append(QString("?token=") + *token);

        loadURL(url);
    }

    delete token;
}

void WebStore::checkConnectionToExit()
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
        emit hideMe();
    }
}

void WebStore::hideKeyboard()
{
    qDebug() << Q_FUNC_INFO;
    QBookApp::instance()->hideKeyboard();
}

void WebStore::deactivateForm()
{
    qDebug() << Q_FUNC_INFO;

    disconnect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(connectionCanceled()));

    delete m_infoDialogAccessFailed;
    m_infoDialogAccessFailed = NULL;
    m_wifiConnectionCanceled = false;

    delete m_connectionDialog;
    m_connectionDialog = NULL;

    if(m_powerLock->isActive())
        m_powerLock->release();

    if(progressTimer->isActive())
        progressTimer->stop();

    if(newKeyboard && newKeyboard->isVisible())
        hideKeyboard();

    QBookApp::instance()->getStatusBar()->setSpinner(false);
    QBookApp::instance()->setBuying(false);
}

void WebStore::handleViewPressPos(QPoint pt)
{
    qDebug() << Q_FUNC_INFO;
    QWebHitTestResult aResult = webview->page()->currentFrame()->hitTestContent(pt);

    if(aResult.isContentEditable())
    {
        qDebug() << Q_FUNC_INFO << "true";
        mSelectWebEdit= true;
        m_selectedElementPos = pt;
        aResult.element().setStyleProperty("font-family", "noexiste");
    }
    else
        hideKeyboard();
}

void WebStore::handleViewReleasePos()
{
    qDebug() << Q_FUNC_INFO;
    if(mSelectWebEdit)
    {
        qDebug() << Q_FUNC_INFO << "true";
        mSelectWebEdit = false;
        QTimer::singleShot(100, this, SLOT(showKeyBoardDialog()));
    }
}
void WebStore::showKeyBoardDialog()
{
    qDebug() << Q_FUNC_INFO;

    bool showKeyboardUp = m_selectedElementPos.y() > 450;
    newKeyboard = QBookApp::instance()->showKeyboard(tr("Accept"),showKeyboardUp);
    newKeyboard->justSendSignals();
    connect(newKeyboard,    SIGNAL(pressedChar(const QString &)),   this, SLOT(sendKey(const QString&)),Qt::UniqueConnection);
    connect(newKeyboard,    SIGNAL(backSpacePressed()),             this, SLOT(sendBackSpace()),Qt::UniqueConnection);
    connect(newKeyboard,    SIGNAL(cursorLeftPressed()),            this, SLOT(sendCursorLeft()), Qt::UniqueConnection);
    connect(newKeyboard,    SIGNAL(cursorRightPressed()),           this, SLOT(sendCursorRight()), Qt::UniqueConnection);
    connect(newKeyboard,    SIGNAL(newLinePressed()),               this, SLOT(sendReturn()),Qt::UniqueConnection);
    connect(newKeyboard,    SIGNAL(newLinePressed()),               this, SLOT(hideKeyboard()),Qt::UniqueConnection);
    connect(newKeyboard,    SIGNAL(actionRequested()),              this, SLOT(sendReturn()),Qt::UniqueConnection);
    connect(newKeyboard,    SIGNAL(actionRequested()),              this, SLOT(hideKeyboard()),Qt::UniqueConnection);
    Screen::getInstance()->refreshScreen();
}

void WebStore::sendKey(const QString & key)
{
    qDebug() << Q_FUNC_INFO;
    QKeyEvent press(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, key);
    QApplication::sendEvent(webview, &press);
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_A, Qt::NoModifier, key);
    QApplication::sendEvent(webview, &release);
}

void WebStore::sendBackSpace()
{
    qDebug() << Q_FUNC_INFO;
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
    QApplication::sendEvent(webview, &press);
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Backspace, Qt::NoModifier);
    QApplication::sendEvent(webview, &release);
}

void WebStore::sendCursorLeft()
{
    qDebug() << Q_FUNC_INFO;
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QApplication::sendEvent(webview, &press);
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Left, Qt::NoModifier);
    QApplication::sendEvent(webview, &release);
}

void WebStore::sendCursorRight()
{
    qDebug() << Q_FUNC_INFO;
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QApplication::sendEvent(webview, &press);
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Right, Qt::NoModifier);
    QApplication::sendEvent(webview, &release);
}

void WebStore::sendReturn()
{
    qDebug() << Q_FUNC_INFO;
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, "\n");
    QApplication::sendEvent(webview, &press);
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Return, Qt::NoModifier, "\n");
    QApplication::sendEvent(webview, &release);
}

void WebStore::loadURL(QString sURL )
{
      qDebug() << "--->" << Q_FUNC_INFO << sURL;
      disconnectWifiObserverNavigateToUrl();
      disconnect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(connectionCanceled()));
      m_wifiConnectionCanceled = false;
      if(!sURL.isEmpty())
      {
          webview->load(QUrl(sURL));
          mCurrectURL = sURL;
      }
      else
          mCurrectURL = webview->url().toEncoded();
}

void WebStore::forceUrl(const QString& url)
 {
     s_forced = QString(url);
     mCurrectURL = QString(url);
 }

void WebStore::processHeaders(QNetworkReply* reply)
{
    qDebug() << "--->" << Q_FUNC_INFO << "for: " << reply->request().url();

    if (!reply)
        return;

    if(reply->error() == QNetworkReply::OperationCanceledError)
        return;

    if(reply->request().url().isEmpty())
        return;

    if(reply->error() != 0 )
    {
        qDebug() << "ERROR CODE IS: " << reply->error();
        webview->stop();

        if(!m_connectionDialog)
            m_connectionDialog = new InfoDialog(this, tr("Not connected. Please try it again in a few minutes"));

        connect(progressTimer, SIGNAL(timeout()), this, SLOT(checkConnectionToExit()));
        connect(progressTimer, SIGNAL(timeout()), progressTimer, SLOT(stop()));
        progressTimer->start( 10 * 1000);
    }

    if(hasPurchaseHeaders(reply))
        processPurchaseHeaders(reply);
    if(hasDoReloginHeader(reply))
        processDoReloginHeader();
    if(hasOpenBookHeader(reply))
        processOpenBookHeader(reply);
}

bool WebStore::hasPurchaseHeaders(QNetworkReply* reply)
{
    qDebug() << "--->" << Q_FUNC_INFO;
    return reply->hasRawHeader(QByteArray("isbn")) && reply->hasRawHeader(QByteArray("purchaseOk"));
}

bool WebStore::hasOpenBookHeader(QNetworkReply* reply)
{
    qDebug() << "--->" << Q_FUNC_INFO;
    return (reply->hasRawHeader(QByteArray("openBook")) && reply->hasRawHeader(QByteArray("isbn")));
}

void WebStore::processOpenBookHeader(QNetworkReply* reply)
{
    qDebug() << "--->" << Q_FUNC_INFO;
    QString openBookHeader = QString(reply->rawHeader(QByteArray("openBook")));
    QString isbn = QString(reply->rawHeader(QByteArray("isbn")));

    if(openBookHeader == "1" && !isbn.isEmpty())
    {
        QString path = Storage::getInstance()->getPrivatePartition()->getMountPoint();
        const BookInfo* bookModel = QBookApp::instance()->getModel()->bookInPath(isbn, QDir::cleanPath(path));

        if(bookModel && !bookModel->path.isEmpty())
            emit openBook(bookModel);
    }
}

void WebStore::errorListener( QNetworkReply * reply, const QList<QSslError> & errors )
{
    qDebug() << Q_FUNC_INFO;
    reply->ignoreSslErrors();
    int count = errors.count();
    for(int i=0 ; i < count ; i++)
    {
        int nErrorCode = errors[i].error();
        qDebug()<<"SSL Error Code = "<< nErrorCode;
    }
}

void WebStore::processPurchaseHeaders(QNetworkReply* reply)
{
    qDebug() << Q_FUNC_INFO;
    QString isbn = QString(reply->rawHeader(QByteArray("isbn")));

    if(!isbn.isEmpty())
    {
        qDebug() << Q_FUNC_INFO << ": Purchase ok!";
        m_powerLock->activate();
        hideKeyboard();
        b_blocked = true;

        createProgressDialog();
        pDialog->setProgressBar(0);
        pDialog->show();


        ElfLibraryReply reply = SUCCESS;
        QList<BookInfo*>* bookInfoList = services->getUserLibrary(isbn, reply);
        qDebug() << "--->" << Q_FUNC_INFO << "after download book. bookInfo.size(): " << bookInfoList->size();

        if(!bookInfoList->size() || reply != SUCCESS)
        {
            destroyDialog();
            delete bookInfoList;
            return;
        }

        m_powerLock->activate();
        BookInfo* book = bookInfoList->first();
        QString path = Storage::getInstance()->getPrivatePartition()->getMountPoint() + QDir::separator() + book->isbn+ "." + book->format;
        const BookInfo* bookModel = QBookApp::instance()->getModel()->getBookInfo(QDir::cleanPath(path));

        qDebug() << "--->" << Q_FUNC_INFO << "isbn" << book->isbn << " archived: " << book->m_archived << " type: " << book->m_type;
        bool isSusbcriptionBook = false;
        bool downloadSuccess = false;
        if(bookModel)
        {
            if(bookModel->m_type == BookInfo::BOOKINFO_TYPE_DEMO)
            {
                QString bookPath = Storage::getInstance()->getPrivatePartition()->getMountPoint() + QDir::separator() + bookModel->isbn + "." + book->format;
                QString cmd = "mv " +  bookPath + " " + bookPath + ".old";
                system(cmd.toAscii().constData());
            }
            else if(bookModel->m_type == BookInfo::BOOKINFO_TYPE_SUBSCRIPTION)
            {
                book = new BookInfo(*bookModel);
                book->m_type = BookInfo::BOOKINFO_TYPE_PURCHASE;
                book->corrupted = false;
                isSusbcriptionBook = true;
                downloadSuccess = true;
                destroyDialog();
            }
        }

        if(!isSusbcriptionBook)
        {
            downloadSuccess = services->downloadBook(book);
            destroyDialog();
            m_powerLock->release();

            b_blocked = false;
        }

        if(!downloadSuccess)
        {
            if(bookModel && bookModel->m_type == BookInfo::BOOKINFO_TYPE_DEMO)
            {
                QString cmd = "mv " + path + ".old" + " " + path;
                system(cmd.toAscii().constData());
            }else
                services->deleteBookAndMeta(book->isbn);

            ConfirmDialog *dialog = new ConfirmDialog(this,tr("Ha habido un problema descargando el libro."));
            dialog->exec();
            delete dialog;
            services->processSyncCanceled();
        }
        else if(downloadSuccess) // Successful book download
        {
            qDebug() << "--->" << Q_FUNC_INFO << "download success";
            QString cmd = "rm " + path + ".old";
            if(!isSusbcriptionBook)
            {
                QString bookPath = Storage::getInstance()->getPrivatePartition()->getMountPoint() + QDir::separator() + book->isbn + ".*";
                QString cmd = "rm " + bookPath + ".old";
                system(cmd.toAscii().constData());

                QList<bqDeviceServicesClientBookMetadata> metadata = services->getMetaData(book->isbn);
                if(metadata.size() > 0){
                    qDebug() << "--->" << Q_FUNC_INFO << "downloading metadata";
                    services->insertMetaData(book, metadata.first());
                }
            }

            SelectionDialog* dialogSelect;
            if(book->m_type == BookInfo::BOOKINFO_TYPE_DEMO)
                dialogSelect = new SelectionDialog(this,tr("Open downloaded sample?"));
            else
                dialogSelect = new SelectionDialog(this,tr("Open downloaded book?"));

            Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_PARTIALSCREEN_UPDATE,Q_FUNC_INFO);
            dialogSelect->exec();

            bool openBookReq = dialogSelect->result();
            delete dialogSelect;

            QBookApp::instance()->getModel()->addBook(book);
            QtConcurrent::run(QBookApp::instance(), &QBookApp::syncModel);

            emit addedBook(book->path);

            if(openBookReq)
                emit openBook(book);
        }
    }
}

bool WebStore::hasDoReloginHeader(QNetworkReply* reply)
{
    qDebug() << Q_FUNC_INFO;
    return (reply->hasRawHeader(QByteArray("doRelogin")) && QString(reply->rawHeader(QByteArray("doRelogin"))) == "1");
}

void WebStore::processDoReloginHeader()
{
    qDebug() << Q_FUNC_INFO;
    QString *token = new QString();

    m_powerLock->activate();
    bool bRet = services->getToken(token);
    m_powerLock->release();
    if(!bRet)
    {
        InfoDialog *infoDialogExpiredSession = new InfoDialog(this,tr("Your session has expired. Please enter again."));
        infoDialogExpiredSession->hideSpinner();
        infoDialogExpiredSession->showForSpecifiedTime();
        delete infoDialogExpiredSession;
        delete token;
        emit hideMe();
        return;
    }

    QString currentUrl = webview->url().toString();
    if(currentUrl.size() > 0 && currentUrl.contains("token=")){
        currentUrl = currentUrl.split("token=")[0];
        currentUrl += "token=" + *token;
    }else
        currentUrl += "&token=" + *token;

    qDebug() << Q_FUNC_INFO << "currentUrl: " << currentUrl;
    loadURL(currentUrl);
}

 void WebStore::keyReleaseEvent(QKeyEvent* event)
 {
     qDebug() << Q_FUNC_INFO << event;

     if(event->key() == QBook::QKEY_BACK)
     {
         event->accept();
         if(QBook::settings().value("setting/initial",true).toBool()) // Block Back in Wizard
             return;

         if(newKeyboard && newKeyboard->isVisible())
             QBookForm::keyReleaseEvent(event);
         else
             goBack();
     }
     else     
         QBookForm::keyReleaseEvent(event);
 }

 void WebStore::goBack()
 {
     qDebug() << Q_FUNC_INFO;

     if(webview->history()->canGoBack())
     {
         qDebug() << Q_FUNC_INFO << "elements: " << webview->history()->count();//WORKAROUND

         if(webview->history()->count() > 1)
             webview->back();
         else
         {
             b_alreadyVisitedStore = false;
             emit hideMe();
         }
     }
     else{
         b_alreadyVisitedStore = false;
         emit hideMe();
     }
}

bool WebStore::eventFilter(QObject* watched, QEvent* event)
{
     Q_UNUSED(watched);

     if (event->type() == QEvent::KeyPress || event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease)
     {
         if(b_blocked)
         {
             qDebug() << Q_FUNC_INFO << "Blocked";
             return true;
         }
     }
     return false;
}

void WebStore::createProgressDialog()
{
    qDebug() << Q_FUNC_INFO;
    if(!pDialog)
    {
        pDialog = new ProgressDialog(this,tr("Downloading book..."));
        pDialog->hideCancelButton();
        pDialog->setHideBtn(false);
        pDialog->setModal(true);
        pDialog->setTextValue(false);
        connect(services, SIGNAL(downloadProgress(int)), pDialog, SLOT(setProgressBar(int)));
        connect(services, SIGNAL(hideProgressBar()), pDialog, SLOT(hideProgressBar()));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_PARTIALSCREEN_UPDATE,Q_FUNC_INFO);
    }
}

void WebStore::destroyDialog()
{
    qDebug() << Q_FUNC_INFO;
    if(pDialog)
    {
        pDialog->hide();
        delete pDialog;
        pDialog = NULL;
    }
}

void WebStore::setTextCursor(int position)
{
    url->setCursorPosition ( position );
}

void WebStore::connectionCanceled()
{
    disconnect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(connectionCanceled()));
    disconnectWifiObserverNavigateToUrl();
    QBookApp::instance()->getStatusBar()->setSpinner(false);
    QBookApp::instance()->goToHome();
    m_wifiConnectionCanceled = true;
}

void WebStore::resetVisitedStore()
{
    b_alreadyVisitedStore = false;
}
