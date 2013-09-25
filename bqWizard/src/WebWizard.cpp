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

#include "WebWizard.h"
#include <QWebElement>
#include <QWebFrame>
#include <QWebHistory>
#include <QWebHitTestResult>
#include <QTimer>
#include <QList>
#include <QKeyEvent>
#include <QTimer>

#include "InfoDialog.h"
#include "Keyboard.h"
#include "QBook.h"
#include "QBookApp.h"
#include "Storage.h"
#include "Screen.h"
#include "BrowserWebPage.h"
#include "BrowserInputContext.h"
#include "BrowserNetworkAccessManager.h"
#include "PowerManager.h"
#include "bqDeviceServices.h"
#include "version.h"
#include "DeviceInfo.h"

#define REFRESH_COUNTER     2

WebWizard::WebWizard(QWidget *parent) : QBookForm(parent)
        , dirty(REFRESH_COUNTER)
        , progress(0)
        , mSelectWebEdit(false)
        , m_connectionDialog(NULL)
        , newKeyboard(NULL)
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
    setFixedSize(Screen::getInstance()->screenWidth(), Screen::getInstance()->screenHeight());

    pPage = new BrowserWebPage(this, true);
    webview->setPage(pPage);

    m_namgr = new BrowserNetworkAccessManager(this);
    webview->page()->setNetworkAccessManager(m_namgr);
    connect(m_namgr,SIGNAL(sslErrors(QNetworkReply*,  const QList<QSslError>&)),this,SLOT(errorListener(QNetworkReply*,const QList<QSslError>&)));
    connect(m_namgr,SIGNAL(finished(QNetworkReply*)), this,SLOT(processHeaders(QNetworkReply*)));

    url->setStyleSheet("QPushButton{text-align : left; background-image: url(:/QBook/white)}");
    webview->page()->setForwardUnsupportedContent(true);

    backbtn->setEnabled(false);
    forwardbtn->setEnabled(false);
    spacerButton->hide();
    browserBar->hide();
    webViewFooterCont->hide();

    connect(webview, SIGNAL(loadProgress(int)),         SLOT(setProgress(int)));
    connect(webview, SIGNAL(loadFinished (bool)),       SLOT(setTextCursor()));
    connect(webview, SIGNAL(loadStarted()),             SLOT(handleLoadStart()));
    connect(webview, SIGNAL(newPressPos(QPoint)),       SLOT(handleViewPressPos(QPoint)));
    connect(webview, SIGNAL(newReleasePos(QPoint)),     SLOT(handleViewReleasePos()));
    connect(webview, SIGNAL(urlChanged(const QUrl &)),  SLOT(handleURLChanged(const QUrl &)));
    connect(webview->page(),SIGNAL(loadFinished(bool)), SLOT(loadFinished(bool)));

    verticalScrollFrame->hide();
    horizontalScrollFrame->hide();

    mInputContext = new BrowserInputContext(this);
    webview->setInputContext(mInputContext);

    progressTimer = new QTimer();

    QFile file(":/res/elf_web_styles.qss");
    file.open(QFile::ReadOnly);
    QString styles = QLatin1String(file.readAll());
    setStyleSheet(styles);
}

void WebWizard::loadFinished(bool){
    qDebug() << Q_FUNC_INFO;
    if(--dirty == 0)
    {
        dirty = REFRESH_COUNTER;
        Screen::getInstance()->refreshScreen();
    }
}

void WebWizard::handleURLChanged(const QUrl & url )
{
    qDebug() << Q_FUNC_INFO << " Url: " << url;

    QString header= url.toString();
    qDebug() << Q_FUNC_INFO << " header: " << header;
    if(header.contains("finish.no.card") || header.contains("action=finishKo") || header.contains("card-add-demo"))
    {
        qDebug() << Q_FUNC_INFO << "FinishNoCard";
        emit registerOK();
        return;
    }

    mCurrectURL = url.toString();
}

void WebWizard::paintEvent(QPaintEvent* e)
{
    qDebug() << Q_FUNC_INFO;
    QWidget::paintEvent(e);
}

void WebWizard::handleLoadStart()
{
    qDebug() << Q_FUNC_INFO;
    if(!ConnectionManager::getInstance()->isConnected())
        QBookApp::instance()->requestConnection();
}

WebWizard::~WebWizard()
{
    qDebug() << Q_FUNC_INFO;
    delete m_powerLock;
    m_powerLock = NULL;
}

void WebWizard::setProgress(int p)
{
    qDebug() << Q_FUNC_INFO << p <<"% loaded " << webview->url().toEncoded();
    m_powerLock->activate();
    if( p == 100 )
    {
        m_powerLock->release();
        mCurrectURL = webview->url().toEncoded();

        if(progressTimer->isActive())
            progressTimer->stop();
    }

    qDebug() << Q_FUNC_INFO << "Progress is " << progress << ", p is " << p;
    progress = p;
}

void WebWizard::activateForm()
{
    qDebug() << Q_FUNC_INFO;

    if(!ConnectionManager::getInstance()->isConnected())
        return;

    activateFormWithConnection();
}

void WebWizard::activateFormWithConnection()
{
    qDebug() << Q_FUNC_INFO;
    QBookApp::instance()->getStatusBar()->hide();

    //Timeout for the landingPage
    m_powerLock->activate();
    connect(progressTimer, SIGNAL(timeout()), this, SLOT(checkConnectionToExit()));
    connect(progressTimer, SIGNAL(timeout()), progressTimer, SLOT(stop()));
    progressTimer->start( 10 * 1000);

    if(!s_forced.isEmpty())
    {
        qDebug() << Q_FUNC_INFO << "forced url";
        loadURL(s_forced);
        s_forced.clear();
    }
}

void WebWizard::checkConnectionToExit()
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
        emit goToWifi();
    }
}

void WebWizard::hideKeyboard()
{
    qDebug() << Q_FUNC_INFO;
    QBookApp::instance()->hideKeyboard();
}

void WebWizard::deactivateForm()
{
    qDebug() << Q_FUNC_INFO;

    delete m_connectionDialog;
    m_connectionDialog = NULL;

    if(m_powerLock->isActive())
        m_powerLock->release();

    if(progressTimer->isActive())
        progressTimer->stop();

    if(newKeyboard && newKeyboard->isVisible())
        hideKeyboard();

    QBookApp::instance()->getStatusBar()->show();
}

void WebWizard::errorListener( QNetworkReply * reply, const QList<QSslError> & errors )
{
    qDebug() << Q_FUNC_INFO;
    reply->ignoreSslErrors();
    for(int i=0 ; i < errors.count() ; i++){
        int nErrorCode = errors[i].error();
        qDebug()<<"SSL Error Code = "<< nErrorCode;
    }
}

void WebWizard::handleViewPressPos(QPoint pt)
{
    QWebHitTestResult aResult = webview->page()->currentFrame()->hitTestContent(pt);
    qDebug() << Q_FUNC_INFO;
    if(aResult.isContentEditable())
    {
        qDebug()<<"aResult.isContentEditable() == true";    
        mSelectWebEdit = true;
        m_selectedElementPos = pt;
        aResult.element().setStyleProperty("font-family", "noexiste");
    }
    else
        hideKeyboard();
}

void WebWizard::handleViewReleasePos()
{
    qDebug() << Q_FUNC_INFO;
    if(mSelectWebEdit)
    {
        qDebug() << Q_FUNC_INFO << "true";
        mSelectWebEdit = false;
        QTimer::singleShot(100, this, SLOT(showKeyBoardDialog()));
    }
}

void WebWizard::showKeyBoardDialog()
{
    qDebug() << Q_FUNC_INFO;
    bool showKeyboardUp = m_selectedElementPos.y() > 450;
    newKeyboard = QBookApp::instance()->showKeyboard(tr("Send"),showKeyboardUp, Keyboard::EMAIL);
    newKeyboard->justSendSignals();
    connect(newKeyboard,SIGNAL(pressedChar(const QString &)),this,SLOT(sendKey(const QString &)),Qt::UniqueConnection);
    connect(newKeyboard,SIGNAL(backSpacePressed()),this,SLOT(sendBackSpace()),Qt::UniqueConnection);
    connect(newKeyboard, SIGNAL(cursorLeftPressed()), this, SLOT(sendCursorLeft()), Qt::UniqueConnection);
    connect(newKeyboard, SIGNAL(cursorRightPressed()), this, SLOT(sendCursorRight()), Qt::UniqueConnection);
    connect(newKeyboard,SIGNAL(newLinePressed()),this,SLOT(sendReturn()),Qt::UniqueConnection);
    connect(newKeyboard,SIGNAL(newLinePressed()),this,SLOT(closeKeyBoardDialog()),Qt::UniqueConnection);
    connect(newKeyboard,SIGNAL(actionRequested()),this,SLOT(sendReturn()),Qt::UniqueConnection);
    connect(newKeyboard, SIGNAL(actionRequested()), this, SLOT(closeKeyBoardDialog()));
    Screen::getInstance()->refreshScreen();
}

void WebWizard::sendKey(const QString & key)
{
    qDebug() << Q_FUNC_INFO;
    QKeyEvent press(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, key);
    QApplication::sendEvent(webview, &press);
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_A, Qt::NoModifier, key);
    QApplication::sendEvent(webview, &release);
}

void WebWizard::sendBackSpace()
{
    qDebug() << Q_FUNC_INFO;
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
    QApplication::sendEvent(webview, &press);
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Backspace, Qt::NoModifier);
    QApplication::sendEvent(webview, &release);
}

void WebWizard::sendCursorLeft()
{
    qDebug() << Q_FUNC_INFO;
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QApplication::sendEvent(webview, &press);
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Left, Qt::NoModifier);
    QApplication::sendEvent(webview, &release);
}

void WebWizard::sendCursorRight()
{
    qDebug() << Q_FUNC_INFO;
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QApplication::sendEvent(webview, &press);
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Right, Qt::NoModifier);
    QApplication::sendEvent(webview, &release);
}

void WebWizard::sendReturn()
{
    qDebug() << Q_FUNC_INFO;
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, "\n");
    QApplication::sendEvent(webview, &press);
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Return, Qt::NoModifier, "\n");
    QApplication::sendEvent(webview, &release);
}

void WebWizard::closeKeyBoardDialog()
{
    qDebug() << Q_FUNC_INFO;
    QBookApp::instance()->hideKeyboard();
}

void WebWizard::loadURL(const QString& sURL )
{
    qDebug() << Q_FUNC_INFO << sURL;
    webview->load(QUrl(sURL));
    mCurrectURL = sURL;
}

void WebWizard::forceUrl(const QString& url)
 {
     s_forced = QString(url);
     mCurrectURL = QString(url);
 }

void WebWizard::processHeaders(QNetworkReply* reply)
{
    qDebug() << Q_FUNC_INFO << "for: " << reply->request().url();

    if (!reply)
        return;

    if(reply->error() == QNetworkReply::OperationCanceledError)
        return;

    if(reply->request().url().isEmpty())
        return;


    if(reply->error() != 0 )
    {
        qDebug() << "ERROR CODE IS: " << reply->error();
        return;
    }

    if(hasLandingHeaders(reply))
        processLandingHeaders(reply);

    checkLandingErrors(reply);
}

void WebWizard::checkLandingErrors(QNetworkReply* reply)
{
    qDebug() << Q_FUNC_INFO;
    if(reply->hasRawHeader(QByteArray("errorCode")) && reply->rawHeader(QByteArray("errorCode")) == "SN-Invalid")
        emit serialInvalid();
}

void WebWizard::setTextCursor(int position)
{
    url->setCursorPosition ( position );
}

bool WebWizard::hasLandingHeaders(QNetworkReply* reply)
{
    qDebug() << "--->" << Q_FUNC_INFO;
    if(reply->hasRawHeader(QByteArray("shopIcon")) && reply->hasRawHeader(QByteArray("shopName")) && reply->hasRawHeader(QByteArray("deviceModelName"))
        && reply->hasRawHeader(QByteArray("eMail")) && reply->hasRawHeader(QByteArray("wsServicesURL")) && reply->hasRawHeader(QByteArray("dictionaryToken"))
        && reply->hasRawHeader(QByteArray("readerPartitionName")) && reply->hasRawHeader(QByteArray("offDeviceImageUrl"))
        && reply->hasRawHeader(QByteArray("restDeviceImageUrl")) && reply->hasRawHeader(QByteArray("startDeviceImageUrl")) && reply->hasRawHeader(QByteArray("updateDeviceImageUrl")))
    {
        return true;
    }else
        return false;
}

void WebWizard::processLandingHeaders(QNetworkReply* reply)
{
    qDebug() << "--->" << Q_FUNC_INFO << "Processing welcomePage values" << reply->readAll();

    QString shopName = QString(reply->rawHeader(QByteArray("shopName")));
    QString deviceModelName = QString(reply->rawHeader(QByteArray("deviceModelName")));
    QString readerPartitionName = QString(reply->rawHeader(QByteArray("readerPartitionName")));
    QString wsServicesURL = QString(reply->rawHeader(QByteArray("wsServicesURL")));
    QString shopUrl = QString(reply->rawHeader(QByteArray("shopUrl")));
    QString shopBookUrl = QString(reply->rawHeader(QByteArray("shopBookUrl")));
    QString shopSearchUrl = QString(reply->rawHeader(QByteArray("shopSearchUrl")));
    QString eMail = QString(reply->rawHeader(QByteArray("eMail")));
    QString dictionaryToken = QString(reply->rawHeader(QByteArray("dictionaryToken")));
    QString shopIcon = QString(reply->rawHeader(QByteArray("shopIcon")));
    QString offDeviceImageUrl = QString(reply->rawHeader(QByteArray("offDeviceImageUrl")));
    QString restDeviceImageUrl = QString(reply->rawHeader(QByteArray("restDeviceImageUrl")));
    QString startDeviceImageUrl = QString(reply->rawHeader(QByteArray("startDeviceImageUrl")));
    QString updateDeviceImageUrl = QString(reply->rawHeader(QByteArray("updateDeviceImageUrl")));
    QString shopUserAccountUrl = QString(reply->rawHeader(QByteArray("shopUserAccountUrl")));
    QString lowBatteryDeviceImageUrl = QString(reply->rawHeader(QByteArray("lowBatteryDeviceImageUrl")));
    QString activated = QString(reply->rawHeader(QByteArray("activated")));

    if(!shopName.isEmpty())
       QBook::settings().setValue("shopName", shopName);

    if(!deviceModelName.isEmpty())
       QBook::settings().setValue("deviceModelName", deviceModelName);

    if(!readerPartitionName.isEmpty()) {
       QBook::settings().setValue("readerPartitionName", readerPartitionName);
       Storage::getInstance()->getPublicPartition()->setVolumeLabel(readerPartitionName);
    }

    if(!wsServicesURL.isEmpty())
       QBook::settings().setValue("wsServicesURL", wsServicesURL);

    if(!shopUrl.isEmpty())
       QBook::settings().setValue("shopUrl", shopUrl);

    if(!shopBookUrl.isEmpty())
       QBook::settings().setValue("serviceURLs/book", shopBookUrl);

    if(!shopSearchUrl.isEmpty())
       QBook::settings().setValue("serviceURLs/search", shopSearchUrl);

    if(!eMail.isEmpty())
       QBook::settings().setValue("eMail", eMail);

    if(!dictionaryToken.isEmpty())
       QBook::settings().setValue("dictionaryToken", dictionaryToken);

    if(!shopIcon.isEmpty())
       QBook::settings().setValue("shopIcon", shopIcon);

    if(!offDeviceImageUrl.isEmpty())
       QBook::settings().setValue("offDeviceImageUrl", offDeviceImageUrl);

    if(!restDeviceImageUrl.isEmpty())
       QBook::settings().setValue("restDeviceImageUrl", restDeviceImageUrl);

    if(!startDeviceImageUrl.isEmpty())
       QBook::settings().setValue("startDeviceImageUrl", startDeviceImageUrl);

    if(!updateDeviceImageUrl.isEmpty())
       QBook::settings().setValue("updateDeviceImageUrl", updateDeviceImageUrl);

    if(!shopUserAccountUrl.isEmpty())
       QBook::settings().setValue("shopUserAccountUrl", shopUserAccountUrl);

    if(!lowBatteryDeviceImageUrl.isEmpty())
       QBook::settings().setValue("lowBatteryDeviceImageUrl", lowBatteryDeviceImageUrl);

    if(!activated.isEmpty())
       QBook::settings().setValue("Activated", activated);

    QBook::settings().sync();
    emit welcomePageLoaded();
    return;
}
void WebWizard::keyReleaseEvent(QKeyEvent* event)
{
     qDebug() << Q_FUNC_INFO << event;

     if(event->key() == QBook::QKEY_BACK)
     {
         hideKeyboard();
         event->accept();
         goBack();
         return;
     }
     QBookForm::keyReleaseEvent(event);
}

 void WebWizard::goBack()
 {
     qDebug() << Q_FUNC_INFO;
     QString landingUrl = QBook::settings().value("serviceURLs/landing","http://landing.mundoreader.com/?").toString();
     QString version = QBOOKAPP_VERSION;
     QString lang = QBook::settings().value("setting/language", QVariant("es")).toString();
     QString email = QBook::settings().value("eMail").toString();
     QString activated = QString::number(QBook::settings().value("setting/activated", false).toInt());

     if(webview->history()->count() > 1)
     {
         forceUrl(QBookApp::instance()->getDeviceServices()->generateWelcomePageUrl(DeviceInfo::getInstance()->getSerialNumber(), landingUrl, version, lang, email, activated));
         activateFormWithConnection();
         webview->history()->clear();
     }
}

 void WebWizard::setWebPageEmail(QString email)
{
    qDebug() << Q_FUNC_INFO;
    pPage->setEmail(email);
}
