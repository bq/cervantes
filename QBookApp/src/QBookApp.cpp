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

#include "QBookApp.h"
#include <QKeyEvent>
#include <QtNetwork>
#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <QPixmap>
#include <QtCore>
#include <QDBusArgument>
#include <QWSServer>
#include <QSize>
#include <QFileInfo>


#include "ModelBackendOneFile.h"
#include "DeviceInfo.h"
#include "DeviceInfoDialog.h"
#include "QBook.h"
#include "Browser.h"
#ifndef DISABLE_ADOBE_SDK
#include "QAdobeDocView.h"
#include "AdobeDRM.h"
#endif
#include "Storage.h"
#include "QSettingLinkPc.h"

#include "BookInfo.h"

#include "InfoDialog.h"
#include "SelectionDialog.h"
#include "ProgressDialog.h"
#include "Wifi.h"
#include "SettingsWiFiList.h"
#include "QBookDevel.h"

#ifndef HACKERS_EDITION
#include "QBookDebugDialog.h"
#include "bqDeviceServices.h"
#include "bqDeviceKey.h"
#include "WebStore.h"
#include "WebWizard.h"
#include "WizardWelcome.h"
#endif

#include "bqPublicServices.h"
#include "ProgressDialog.h"
#include "SettingsQuickSettingsPopup.h"
#include "RTCManager.h"
#include "Search.h"
#include "QFwUpgradeOTA.h"
#include "SettingsDeviceInfo.h"
#include "PasswordRequester.h"
#include "Settings.h"
#include "FrontLight.h"
#include "QBookScreenSaver.h"
#include "EpubMetaDataExtractor.h"
#include "Fb2MetaDataExtractor.h"
#include "MobiMetaDataExtractor.h"

#ifdef Q_WS_QWS
#include <ntx.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <syslog.h>
#endif

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


#ifdef __QBOOK_ARCHIVE__
#include "QRAR.h"
#include "QArchiveBrowser.h"
#endif

#include "Screen.h"

#include "Battery.h"
#include "DeleteLaterFile.h"
#include "SleeperThread.h"
#include "version.h"

#define POWEROFF_BATTERY_LOW_TIMER      30000
#define POWERBUTTON_LONGPRESS_TIMER     1500
#define LIGHT_LONGPRESS_TIMER           1200
#define USB_CHANGE_SUSPEND_WAIT_TIME    10 // 10 Seconds
#define POWER_KEY_WATCHER_PERIOD        100
#define BOOKS_LOADED_TO_SHOW_DIALOG 10

#ifdef MX508
#define STORE_CERTS_PATH	"/etc/ssl/certs/mundoreader/*.pem"
#define AGENT_PROGRAM		"/app/bin/ConnectionManagerAgent"
#define MANUALS_PATH            "/usr/manuals/"
#else
#ifndef Q_WS_QWS
#define STORE_CERTS_PATH	ROOTFS"/etc/ssl/certs/mundoreader/*.pem"
#define AGENT_PROGRAM		"./ConnectionManager/agent/x86-bin/ConnectionManagerAgent"
#define MANUALS_PATH            ROOTFS"/usr/manuals/"
#endif
#endif

#ifdef Q_WS_QWS
#define OFFLINE_HELPER "/app/bin/Offline"
#else
#define OFFLINE_HELPER "./ConnectionManager/offline/x86-bin/Offline"
#endif

#include <QtNetwork/QSslConfiguration>
#include <QThread>

#include "Viewer.h"
#include "Keyboard.h"
#include "FullScreenWidget.h"

#include "Model.h"
#include "Home.h"
#include "StatusBar.h"
#include "Library.h"

#include <QWidget>
#include "Dictionary.h"
#include "ConfirmDialog.h"
#include "Power.h"
#include "PowerManager.h"
#include "PowerManagerDefs.h"
#include "ADConverter.h"
#include "CorruptedDialog.h"
#include "SettingsUnLinkStoreDevice.h"
#include "SettingsDeviceOptionsMenu.h"
#include "SyncHelper.h"

#define TIME_TO_AUTOCONNECT_LOCK             25
#define TIME_SHOW_LOWBATTERY_DIALOG       30000 // Miliseconds
#define TIME_SHOW_CRITICALBATTERY_DIALOG  30000 // Miliseconds
#define TIME_SHOW_SUBSCRIPTION_DIALOG     60000 // Miliseconds
#define TIME_SHOW_ACTIVATION_DIALOG       7200  // 2 hours
#define HACKERS_UPDATE_CHECK_FILE_Q22     "/hackers_e60q22_ok"
#define HACKERS_UPDATE_CHECK_FILE_672     "/hackers_ok"

// Singleton instance
QBookApp* QBookApp::m_spInstance = NULL;

#define MAX_NUM_HWCLOCK_REATTS  100

static int i_setHwClockAttempts = 0;

void QBookApp::staticInit()
{
    if(!m_spInstance)
    {
        m_spInstance = new QBookApp(NULL, Qt::Window);
    }
}


void QBookApp::staticDone()
{
    qDebug() << Q_FUNC_INFO;
    // Deleting QBookApp singleton
    if(m_spInstance)
    {
        delete m_spInstance;
        m_spInstance = NULL;
    }
}

QBookApp::QBookApp(QWidget* parent, Qt::WFlags flags)
    : QWidget(parent, flags)
    , link(NULL)
    , deviceInfoDialog(NULL)
    , quickSettingsPopup(NULL)
    , m_wifiSelection(NULL)
    , m_home(NULL)
    , mass_storage(false)
    , b_charger_state(false)
    , b_synchronizing(false)
    , isPoweringOff(false)
    , b_keyBlocked(false)
    , b_lazyDelete(false)
    , m_library(NULL)
    , m_settingsMenu(NULL)
    , m_search(NULL)
    , m_webView(NULL)
#ifndef HACKERS_EDITION
    , m_welcomeWizard(NULL)
    , m_pServices(NULL)
    , m_pPublicServices(NULL)
    , m_bqDeviceKey(NULL)
    , m_webStore(NULL)
    , m_webWizard(NULL)
#endif
    , m_book(NULL)
    , m_connectManager(NULL)
    , m_pKeyboard(NULL)
    , m_viewer(NULL)
    , b_powerOffOngoing(false)
    , i_powerKeySamplesCounter(0)
    , fwDwld(NULL)
    , m_OTAdownloadingDialog(NULL)
    , m_syncDialog(NULL)
    , readingFilesdialog(NULL)
    , b_batteryLevel(NULL)
    , loadingBooksDialog(NULL)
    , m_debugDialog(NULL)
    , m_autoconnect_lock(NULL)
    , m_connected_lock(NULL)
    , m_usbChangeLock(NULL)
    , m_powerKeyLock(NULL)
    , m_homeKeyLock(NULL)
    , m_syncLock(NULL)
    , m_connectingNetwork(NULL)
    , m_sync(NULL)
    , m_sleepScreen(NULL)
    , bLoadWizard(false)
    , m_openDocError(false)
    , networkStartTimerFlag(false)
    , m_isDialogShow(false)
    , m_mmcState(false)
    , b_userEventsBlocked(false)
    , b_isOTAUpdating(false)
    , b_blockBtns(false)
    , b_wifiWasConnected(false)
    , b_cancelSleep(false)
    , b_isSleeping(false)
    , b_isBuying(false)
    , b_wizardFromStore(false)
    , offlineHelper(NULL)
{
#ifndef HACKERS_EDITION
    checkOTAFile();
#endif

    m_lastOTACheck.setTime_t(0);

    QString lastLinkCheck = QBook::settings().value("setting/lastLinkCheck", "").toString();
    if(!lastLinkCheck.isEmpty())
        m_lastLinkCheck = QDateTime::fromString(lastLinkCheck, Qt::ISODate);
    else
    {
        m_lastLinkCheck = QDateTime::currentDateTime();
        QBook::settings().setValue("setting/lastLinkCheck", QVariant(QDateTime::currentDateTime().toString(Qt::ISODate)));
    }

    QString lastActivationCheck = QBook::settings().value("setting/lastActivatedCheck", "").toString();
    if(!lastActivationCheck.isEmpty())
        m_lastActivatedCheck = QDateTime::fromString(lastActivationCheck, Qt::ISODate);
    else
    {
        QBook::settings().setValue("setting/lastActivatedCheck", QVariant(QDateTime::currentDateTime().toString(Qt::ISODate)));
        m_lastActivatedCheck = QDateTime::currentDateTime();
    }

    m_spInstance = this;
    setFixedSize(Screen::getInstance()->screenWidth(),Screen::getInstance()->screenHeight());
    setupUi(this);

#ifndef Q_WS_QWS
    setCursor(QCursor(Qt::ArrowCursor));
#endif

    PowerManager::initialize();

    Storage* storage = Storage::getInstance();
    storage->init();

    PowerManagerLock *constructorLock = PowerManager::getNewLock();
    constructorLock->activate();

    // Create PowerLocks
    m_powerKeyLock = PowerManager::getNewLock();
    m_homeKeyLock = PowerManager::getNewLock();
    m_syncLock = PowerManager::getNewLock();

    // Connect signals/slots that may show dialogs just after loading home
    QTimer::singleShot(100, this, SLOT(connectDialogSlots()));

    connect(storage, SIGNAL(partitionMounted(StoragePartition*)), this, SLOT(handlePartitionMounted(StoragePartition*)), Qt::QueuedConnection);
    connect(storage, SIGNAL(partitionUmounted(StoragePartition*)), this, SLOT(handlePartitionUmounted(StoragePartition*)));
    connect(storage, SIGNAL(partitionCorrupted(StoragePartition*)), this, SLOT(handlePartitionCorrupted(StoragePartition *)));
    connect(storage, SIGNAL(partitionUmountFailed(StoragePartition*)), this, SLOT(partitionUmountFailed(StoragePartition*)));
    connect(storage, SIGNAL(deviceUnknown(StorageDevice*)), this, SLOT(handleUnknownStorage(StorageDevice *)));
    connect(storage,SIGNAL(deviceRemoved(StorageDevice*)), this, SLOT(deviceRemoved(StorageDevice*)),Qt::QueuedConnection);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    m_pModel = new Model(new ModelBackendOneFile(Storage::getInstance()->getPrivatePartition()->getMountPoint()));

    foreach(StoragePartition *partition, storage->getPartitions())
    {
        if (partition->getPartitionType() == partition->PUBLIC
                || partition->getPartitionType() == partition->PRIVATE
                    || partition->getPartitionType() == partition->REMOVABLE)
        {

                if(partition->isMounted())
                {
                    qDebug() << Q_FUNC_INFO << "Adding partition " << partition->getMountPoint() << " to book model";
                    m_pModel->addDir(partition->getMountPoint()+ QDir::separator());
                }else{
                    qDebug() << Q_FUNC_INFO << "partition " << partition->getPartitionType()  << "not monted->getMountPoint()" ;
                }
        }
    }

    init();

    // if first power on or restore
    firseInit();

    checkRebootFromRestore();

    qApp->installEventFilter(this);

    m_pKeyboard = Keyboard::getInstance();

#ifdef SHOWCASE
    int brightness = FrontLight::getInstance()->getBrightness();
    FrontLight::getInstance()->setBrightness(brightness)
    FrontLight::getInstance()->setFrontLightActive(true);
    FrontLight::getInstance()->setBrightness(100)
#endif

#ifndef Q_WS_QWS
    setCursor(QCursor(Qt::PointingHandCursor));
#endif

#ifndef HACKERS_EDITION
    // Load wizard in the end just for first run
    if(bLoadWizard)
    {
        m_welcomeWizard = new WizardWelcome(this);
        m_welcomeWizard->setFormFlags(QBookForm::FLAG_SHARED);
        connect(m_welcomeWizard, SIGNAL(closeWizard()), this, SLOT(closeWizard()));
        connect(m_welcomeWizard, SIGNAL(closeWizard()), this,SLOT(checkNewDictionaries()), Qt::UniqueConnection);
        pushTopForm(m_welcomeWizard);
        Screen::getInstance()->releaseScreen();
        // Hide the title bar. TODO: Corregir esta llamada en el activateForm del Wizard.
        getStatusBar()->hide();
    }else{
#endif
        Screen::getInstance()->releaseScreen();
        goToHome();
#ifndef HACKERS_EDITION
    }
#endif

    bLoadWizard = false;

    PowerManager::start();

    QTimer::singleShot(0, Battery::getInstance(), SLOT(getLevel()));

    // Check status after start up
    QTimer::singleShot(1500, this, SLOT(firstChecksAfterStartup()));

    connect(m_connectManager,SIGNAL(connected()),this,SLOT(checkFwVersion()), Qt::UniqueConnection);
    connect(&m_timer_powerButton, SIGNAL(timeout()), this, SLOT(askPowerOffDevice()));
    connect(&m_timer_light, SIGNAL(timeout()), this, SLOT(powerLight()));
    connect(&m_timer_light, SIGNAL(timeout()), &m_timer_light, SLOT(stop()));
    connect(&m_powerKeyWatcher, SIGNAL(timeout()),this, SLOT(checkLongPressPowerOff()));

    // Init timers
    m_powerKeyWatcher.setSingleShot(false); // Periodic
    m_powerKeyWatcher.setInterval(POWER_KEY_WATCHER_PERIOD);
    m_timer_powerButton.setSingleShot(true);
    m_timer_powerButton.setInterval(POWERBUTTON_LONGPRESS_TIMER);
    m_timer_light.setSingleShot(true);
    m_timer_light.setInterval(LIGHT_LONGPRESS_TIMER);

    if(DeviceInfo::getInstance()->hasFrontLight() && FrontLight::getInstance()->isFrontLightActive())
    {
        int brightness = FrontLight::getInstance()->getBrightness();
        FrontLight::getInstance()->setBrightness(brightness);
        FrontLight::getInstance()->switchFrontLight(true);
    }

    connect(&m_afterWifiOnTimer, SIGNAL(timeout()), this, SLOT(releaseAutoconnectPowerLock()));

    /* switch off any power-on indicator led */
    Power::getInstance()->setLed(false);

    checkStartupDebugOpts();

    constructorLock->release();
    delete constructorLock;
}

SyncHelper* QBookApp::getSyncHelper()
{
    if(!m_sync){
        createSyncHelper();
    }
    return m_sync;
}

void QBookApp::createSyncHelper()
{
    if(m_sync){
        delete m_sync;
        m_sync = NULL;
    }
    m_sync = new SyncHelper("com.bqreaders.SyncHelperService", "/SyncHelperService", QDBusConnection::systemBus(), this);

    connect(m_sync, SIGNAL(UpdateBook(const QVariantMap&, const QVariantMap&)), this, SLOT(doUpdateBook(const QVariantMap&, const QVariantMap&)), Qt::UniqueConnection);
    connect(m_sync, SIGNAL(AddBook(const QVariantMap&, const QVariantMap&)), this, SLOT(doAddBook(const QVariantMap&, const QVariantMap&)), Qt::UniqueConnection);
    connect(m_sync, SIGNAL(RemoveBook(const QString&)), this, SLOT(doRemoveBook(const QString&)), Qt::UniqueConnection);
    connect(m_sync, SIGNAL(unArchivedBook(int,bool)), m_library, SLOT(unArchivedBook(int,bool)), Qt::UniqueConnection);
    connect(m_sync, SIGNAL(Finished(int, bool)), this, SLOT(synchronizationEnd(int, bool)), Qt::UniqueConnection);
    connect(m_sync, SIGNAL(AddHomeBook(const QVariantMap&, int, int, const QString&)), m_home, SLOT(doAddHomeBook(const QVariantMap&, int, int, const QString&)), Qt::UniqueConnection);
    connect(m_sync, SIGNAL(homeBooksFinished()), m_home, SLOT(getHomeBooksFinished()), Qt::UniqueConnection);
    connect(QDBusConnection::systemBus().interface(), SIGNAL(serviceOwnerChanged ( const QString &, const QString &, const QString &)), this, SLOT(dbusServicesChanged(const QString &, const QString &, const QString &)));

    if (QDBusConnection::systemBus().interface()->isServiceRegistered("com.bqreaders.SyncHelperService")) {
        initSyncHelper();
    } else {
        QDBusReply<void> reply;
        reply = QDBusConnection::systemBus().interface()->startService("com.bqreaders.SyncHelperService");
        if (!reply.isValid()) {
            qDebug() << "Starting com.bqreaders.SyncHelperService failed:" << reply.error().message();
        } else {
            qDebug() << "Starting com.bqreaders.SyncHelperService ok:";
        }
    }
}

void QBookApp::initSyncHelper()
{
    const QString& privatePartition = Storage::getInstance()->getPrivatePartition()->getMountPoint() + QDir::separator();
    const QString& serialNumber = DeviceInfo::getInstance()->getSerialNumber();
    const QString& dataPartition = Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator();
    const QString& servicesUrl = QBook::settings().value("wsServicesURL", "").toString();
    const QString& shopName = QBook::settings().value("shopName", "").toString();
    const QString deviceModel = QBook::settings().value("deviceModelName", "").toString();
    const QString version = QBOOKAPP_VERSION;
    const QString lang = QBook::settings().value("setting/language", QVariant("es")).toString();

    m_sync->init(privatePartition,serialNumber,dataPartition,servicesUrl,shopName,deviceModel, version, lang);
    getSyncHelper()->SetServerTimestamp(0);
    emit syncHelperCreated();
}

void QBookApp::fillBookInfoWithDataMap(BookInfo *bookinfo, const QVariantMap &map)
{
        bookinfo->title = map["title"].toString();
        bookinfo->isbn = map["isbn"].toString();
        bookinfo->path = map["path"].toString();
        bookinfo->author = map["author"].toString();
        bookinfo->thumbnail = map["thumbnail"].toString();
        bookinfo->coverUrl = map["coverUrl"].toString();

        if (map.contains("publishTime"))
        	bookinfo->publishTime = QDateTime::fromTime_t(map["publishTime"].toUInt());

        bookinfo->downloadTime= QDateTime::currentDateTimeUtc();
        bookinfo->pageCount = map["pageCount"].toInt();
        bookinfo->lastReadLink = map["lastReadLink"].toString();
        bookinfo->readingProgress = map["readingProgress"].toDouble();
        bookinfo->readingPercentage = map["readingPercentage"].toDouble();

        if(map["timestamp"].toLongLong() > 0){
            long long unixTime = (map["timestamp"].toLongLong() / 1000);
            QDateTime time;
            time.setTime_t(unixTime);
            bookinfo->lastTimeRead = time;
            bookinfo->timestamp = map["timestamp"].toLongLong();
        }

        bookinfo->publisher = map["publisher"].toString();
        bookinfo->synopsis = map["synopsis"].toString();
        bookinfo->fileSize = map["fileSize"].toDouble();
        bookinfo->syncDate = QDateTime::currentDateTimeUtc();
        bookinfo->storePrice = map["storePrice"].toString();
        bookinfo->format = map["format"].toString();
        bookinfo->fontSize = map["fontSize"].toDouble();
        bookinfo->pageMode = map["pageMode"].toInt();
        bookinfo->totalReadingTime = map["totalReadingTime"].toInt();

        if (map.contains("m_expirationDate"))
            bookinfo->m_expirationDate = QDateTime::fromMSecsSinceEpoch(map["m_expirationDate"].toLongLong()).toUTC();

        bookinfo->m_type = (BookInfo::bookTypeEnum)map["m_type"].toInt();
        if(bookinfo->m_type == BookInfo::BOOKINFO_TYPE_SUBSCRIPTION)
            bookinfo->corrupted = true;
        bookinfo->m_archived = map["m_archived"].toBool();
        bookinfo->isDRMFile = map["isDRMFile"].toBool();
        bookinfo->readingStatus = (BookInfo::readStateEnum)map["readingStatus"].toInt();
        bookinfo->resetReadingPeriod();
        bookinfo->clearPercentageList();
}

void QBookApp::updateBookInfoWithDataMap(BookInfo *bookinfo, const QVariantMap &map)
{
    qDebug() << Q_FUNC_INFO << "isbn:" << bookinfo->isbn << "type: " << bookinfo->m_type;

    if(!bookinfo->format.contains("pdf")){
        bookinfo->lastReadLink = map["lastReadLink"].toString();
        bookinfo->readingProgress = map["readingProgress"].toDouble();
        bookinfo->readingPercentage = map["readingPercentage"].toDouble();
    }

    if(map["timestamp"].toLongLong() > 0)
    {
        long long unixTime = (map["timestamp"].toLongLong() / 1000);
        QDateTime time;
        time.setTime_t(unixTime);
        bookinfo->lastTimeRead = time;
        bookinfo->timestamp = map["timestamp"].toLongLong();
    }

    bookinfo->m_archived = map["m_archived"].toBool();
    bookinfo->syncDate = QDateTime::currentDateTimeUtc();

    BookInfo::bookTypeEnum type = (BookInfo::bookTypeEnum)map["m_type"].toInt();
    if(bookinfo->m_type == BookInfo::BOOKINFO_TYPE_SUBSCRIPTION && type == BookInfo::BOOKINFO_TYPE_PURCHASE)
        bookinfo->corrupted = false;
    bookinfo->m_type = type;
    if(bookinfo->m_type == BookInfo::BOOKINFO_TYPE_SUBSCRIPTION)
        bookinfo->corrupted = true;
    bookinfo->isDRMFile = map["isDRMFile"].toBool();
    bookinfo->fontSize = map["fontSize"].toDouble();
    bookinfo->pageMode = map["pageMode"].toInt();
    bookinfo->totalReadingTime = map["totalReadingTime"].toInt();
    bookinfo->readingStatus = (BookInfo::readStateEnum)map["readingStatus"].toInt();
    bookinfo->setReadingPeriod(0);
    bookinfo->clearPercentageList();

    qDebug() << Q_FUNC_INFO << "isbn:" << bookinfo->isbn << "type: " << bookinfo->m_type << " archived: " << bookinfo->m_archived;

    if(bookinfo->m_archived){
        QFile file(bookinfo->path);
        if(file.exists())
            QFile::remove(bookinfo->path);
    }

    if (map.contains("m_expirationDate"))
        bookinfo->m_expirationDate = QDateTime::fromMSecsSinceEpoch(map["m_expirationDate"].toLongLong()).toUTC();
}

void QBookApp::fillBookInfoWithLocationMap(BookInfo *bookinfo, const QVariantMap &locations)
{
    qDebug() << Q_FUNC_INFO;
	QVariantMap::const_iterator it = locations.begin();
    bookinfo->clearLocations();

    while(it != locations.end())
    {
        // And yes, kids, this is real black magic. I should sit down a nice
        // evening and write a proper BookInfo dbus param adaptor.
        QVariantMap map;
        qvariant_cast<QDBusArgument>(it.value()) >> map;

		BookLocation *location = new BookLocation();

        location->type = (BookLocation::Type) map["type"].toInt();
        location->operation = BookLocation::EMPTY;
        location->text = map["text"].toString();
        location->note = map["note"].toString();
        location->bookmark = map["bookmark"].toString();
        location->page = map["page"].toInt();
        location->lastUpdated = map["lastUpdated"].toLongLong();

		bookinfo->addLocation(it.key(), location);
		++it;
    }
}

void QBookApp::doUpdateBook(const QVariantMap &data, const QVariantMap &locations)
{
    qDebug() << Q_FUNC_INFO << data;
    QString isbn = data["isbn"].toString();
    QString format = data["format"].toString();
    QString filePath = Storage::getInstance()->getPrivatePartition()->getMountPoint() + QDir::separator() + isbn + "." + format;

    const BookInfo* modelBook = m_pModel->getBookInfo(QDir::cleanPath(filePath));

    if(!modelBook) return;

    BookInfo *bookinfo = new BookInfo(*modelBook);
    updateBookInfoWithDataMap(bookinfo, data);
	fillBookInfoWithLocationMap(bookinfo, locations);

    m_pModel->updateBook(bookinfo);

    if(m_viewer && m_viewer->getCurrentBookInfo())
    {
        if(bookinfo->path == m_viewer->getCurrentBookInfo()->path)
        {
            qDebug() << Q_FUNC_INFO << "Reset reading period";
            qDebug() << Q_FUNC_INFO << "ReadingPeriod: "<<  m_viewer->getCurrentBookInfo()->getReadingPeriod();
            m_viewer->getCurrentBookInfo()->resetReadingPeriod();
            m_viewer->getCurrentBookInfo()->clearPercentageList();
            qDebug() << Q_FUNC_INFO << "ReadingPeriod: "<<  m_viewer->getCurrentBookInfo()->getReadingPeriod();
        }
    }
    qDebug() << Q_FUNC_INFO << "bookinfo: "<<  bookinfo->getReadingPeriod();
    delete bookinfo;
}

void QBookApp::doAddBook(const QVariantMap &data, const QVariantMap &locations)
{
	qDebug() << "*** Called doAddBook with data: " << data;
	BookInfo *bookinfo = new BookInfo();
	fillBookInfoWithDataMap(bookinfo, data);
    fillBookInfoWithLocationMap(bookinfo, locations);
    m_pModel->addBook(bookinfo);
}

void QBookApp::doRemoveBook(const QString& filePath)
{
    qDebug() << Q_FUNC_INFO;
    const BookInfo* modelBook = m_pModel->getBookInfo(filePath);

    if(!modelBook)
        return;

    if(modelBook->m_archived && modelBook->m_type != BookInfo::BOOKINFO_TYPE_SUBSCRIPTION)
        return;

    m_pModel->removeBook(modelBook);
}

QBookApp::~QBookApp()
{
    qDebug() << Q_FUNC_INFO;

    // Deleting app singletons
    release();

    ConnectionManager::removeInstance();
    m_connectManager = NULL;
    delete m_passwordRequester;

    delete fwDwld;
    fwDwld = NULL;
    delete m_OTAdownloadingDialog;
    m_OTAdownloadingDialog = NULL;
    delete b_batteryLevel;
    b_batteryLevel = NULL;

    delete m_autoconnect_lock;
    m_autoconnect_lock = NULL;

    delete m_connected_lock;
    m_connected_lock = NULL;

    delete m_usbChangeLock;
    m_usbChangeLock = NULL;

    delete m_powerKeyLock;
    m_powerKeyLock = NULL;

    delete m_homeKeyLock;
    m_homeKeyLock = NULL;

    delete m_syncLock;
    m_syncLock = NULL;
}


/*-------------------------------------------------------------------------*/

void QBookApp::checkRebootFromRestore()
{
    if(QBook::settings().value("setting/pwr_on_from_restore", false).toBool()){
        ConfirmDialog* confirmDialog = new ConfirmDialog(this,tr("Restore Default Success"));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        confirmDialog->exec();
        delete confirmDialog;
        QBook::settings().setValue("setting/pwr_on_from_restore", false);
   }
}

void QBookApp::checkLanguageChanged()
{
    if (QBook::settings().value("setting/changeLanguage", false).toBool())
    {
        ConfirmDialog* confirmDialog = new ConfirmDialog(this,tr("Language changed successfully"));
        confirmDialog->exec();
        delete confirmDialog;
        Screen::getInstance()->refreshScreen();
        QBook::settings().remove("setting/changeLanguage");
        QBook::settings().sync();
    }
}

void QBookApp::init()
{
    qDebug() << Q_FUNC_INFO;
//    release();
    setupSSLconfig(STORE_CERTS_PATH);
    qDebug() << "AFTER RELEASE";

    connect(Storage::getInstance(),SIGNAL(deviceInserted(StorageDevice*)),this,SIGNAL(userEvent()));
    connect(Storage::getInstance(),SIGNAL(deviceRemoved(StorageDevice*)),this,SIGNAL(userEvent()));
    connect(this, SIGNAL(userEvent()),PowerManager::getInstance(),SLOT(setTimeLatestUserEvent()),Qt::QueuedConnection);
    connect(PowerManager::getInstance(),SIGNAL(backFromSuspend()),getStatusBar(),SLOT(updateTime()));
    connect(PowerManager::getInstance(),SIGNAL(goingToSleep()),this,SLOT(prepareToSleep()));
    connect(PowerManager::getInstance(),SIGNAL(backFromSleep()),getStatusBar(),SLOT(updateTime()));
    connect(PowerManager::getInstance(),SIGNAL(backFromSleep()),this,SLOT(resumeAfterSleep()));
    connect(PowerManager::getInstance(), SIGNAL(releaseConnectedPowerLock()), this, SLOT(releaseConnectedPowerLock()));
    connect(PowerManager::getInstance(), SIGNAL(shuttingDown()), this, SLOT(prepareToShutdown()));
    connect(this, SIGNAL(periodicSyncFinished()), this, SLOT(checkScreensaverCoverAfterSync()));

//    m_stack = new QStackedWidget(body);
//    m_stack->setLineWidth(0);
//    m_stack->setFrameShape(QFrame::NoFrame);

//    bodyArea->addWidget(m_stack);
    fwDwld = new QFwUpgradeOTA();
    m_home = new Home(this);
    m_home->setFormFlags(QBookForm::FLAG_SHARED);
    connect(m_pModel,  SIGNAL(modelChanged(QString, int)), m_home, SLOT(modelChanged(QString, int)), Qt::UniqueConnection);
    connect(m_home,    SIGNAL(viewRecentBooks()), this, SLOT(viewRecentBooks()), Qt::UniqueConnection);
    m_home->move(0, getStatusBar()->height());// Until this form has not the valid height...
    m_home->hide();

    QString serialNumber = DeviceInfo::getInstance()->getSerialNumber();
    QString privatePartition = Storage::getInstance()->getPrivatePartition()->getMountPoint() + QDir::separator();
    QString dataPartition = Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator();
    QString servicesUrl = QBook::settings().value("wsServicesURL", "").toString();
    QString shopName = QBook::settings().value("shopName", "").toString();
    QString deviceModel = QBook::settings().value("deviceModelName", "").toString();
    QString lang = QBook::settings().value("setting/language", QVariant("es")).toString();

    m_pPublicServices = new bqPublicServices();

#ifndef HACKERS_EDITION
    m_pServices = new bqDeviceServices(serialNumber, privatePartition, dataPartition, servicesUrl, shopName, deviceModel, QBOOKAPP_VERSION, lang);
    m_pServices->setServerTimestamp(m_pModel->getServerTimestamp());

    m_bqDeviceKey = new bqDeviceKey();
    m_bqDeviceKey->setDataPartition(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator());
    connect(m_pServices, SIGNAL(unlinkForced()), this, SLOT(unlinkForced()));
    Dictionary::staticInit();
#endif

    connect(getStatusBar(), SIGNAL(openLastContent()),        this, SLOT(openLastContent()));
    connect(getStatusBar(), SIGNAL(wifiConnectionCanceled()), this, SIGNAL(wifiConnectionCanceled()));

    initLibrary();// NOTE: We create the library because we need it for the sync periodyc to start
    initViewer();

    /// Clear Downloading flag at restart for robustness
    QBook::settings().setValue("upgrade/Downloading", false);
#ifndef HACKERS_EDITION
    if(isLinked())
        createSyncHelper();
#endif
    if(!m_connectManager)
        initConnectManager();

    hackersInit();

    qDebug() << Q_FUNC_INFO << "end init";
}

void QBookApp::initConnectManager()
{
    qDebug() << Q_FUNC_INFO;

    m_connectManager = ConnectionManager::getInstance();
    // FIXME: FER dissable this until we write down a sane way of doing it.

    m_autoconnect_lock = PowerManager::getNewLock(this);
    m_autoconnect_lock->setTimeOut(TIME_TO_AUTOCONNECT_LOCK);
    m_connected_lock = PowerManager::getNewLock(this);

    connect(m_autoconnect_lock, SIGNAL(lockTimedOut()), this, SIGNAL(resumingWifiFailed()));
    connect(m_autoconnect_lock, SIGNAL(lockTimedOut()), this, SLOT(finishedResumingWifi()));
    connect(&m_wifiSleepTimer, SIGNAL(timeout()), this, SLOT(checkConnectedPowerLock()));

    connect(m_connectManager, SIGNAL(connected()), this, SLOT(releaseAutoconnectPowerLock()), Qt::DirectConnection);
    connect(m_connectManager, SIGNAL(connected()), this, SLOT(activateConnectedPowerLock()));
    connect(m_connectManager, SIGNAL(connected()), this, SLOT(finishedResumingWifi()));
    connect(m_connectManager, SIGNAL(connected()), this, SLOT(setWifiStatusAsConnected()));
    connect(m_connectManager, SIGNAL(connected()), this, SLOT(askForActivationDevice()), Qt::UniqueConnection);
    connect(m_connectManager, SIGNAL(connected()), this, SLOT(synchronization()), Qt::UniqueConnection);
    connect(m_connectManager, SIGNAL(connected()), this, SLOT(checkNewDictionaries()), Qt::UniqueConnection);

    connect(m_connectManager, SIGNAL(timeUpdated()), this, SIGNAL(userEvent())); // To update sleep time
    connect(m_connectManager, SIGNAL(timeUpdated()), this, SLOT(setHwClock()));
    connect(m_connectManager, SIGNAL(disconnected()), this, SLOT(releaseConnectedPowerLock()));
    connect(m_connectManager, SIGNAL(disconnected()), this, SLOT(lostConnection()),Qt::UniqueConnection);

    m_passwordRequester = new PasswordRequester(m_connectManager, this);
    connect(m_passwordRequester, SIGNAL(passwordRequested()), this, SIGNAL(passwordRequested()));
    connect(m_passwordRequester, SIGNAL(passwordSent()), this, SIGNAL(passwordSent()));
    connect(m_passwordRequester, SIGNAL(wrongPassword(QString)), this, SIGNAL(wrongPassword(QString)));
    connect(m_passwordRequester, SIGNAL(cancelPassword()), this, SIGNAL(cancelPassword()));
    connect(m_passwordRequester, SIGNAL(connectionFailed(QString, QString)), this, SIGNAL(connectionFailed(QString, QString)));

    /* Set powerlock initial state, because qbookapp could be restarted with wifi online */
    if (m_connectManager->isConnected())
        activateConnectedPowerLock();

    requestConnection(false);


    connect(m_connectManager,SIGNAL(currentNetworkLevelChanged(int)),this,SLOT(WifiLevelChanged(int)));

    /************************************************************************************************/
    // We need a look after connman online state. Connman try to autoconnect after gone online.
    connect(m_connectManager, SIGNAL(onLineWifi(bool)), this, SLOT(lockAutoconnectOnWifiPower(bool)));
    connect(m_connectManager, SIGNAL(wifiProblem()), this, SLOT(wifiProblem()));
    /************************************************************************************************/
}

void QBookApp::setHwClock() {
    qDebug() << Q_FUNC_INFO << i_setHwClockAttempts;


    // Check power key to avoid collision in MSP430 of I2C poweroff system
    if(!Power::getInstance()->isPowerKeyPressed()
         && RTCManager::setHardwareDateFromSystem())
    {
        i_setHwClockAttempts = 0;
        return;
    }
    // Re-schedule if not sucessful
    qDebug() << Q_FUNC_INFO << "HW clock set FAILURE";
    i_setHwClockAttempts++;
    if(i_setHwClockAttempts < MAX_NUM_HWCLOCK_REATTS)
        QTimer::singleShot(100,this,SLOT(setHwClock()));
    else{
        qWarning() << Q_FUNC_INFO << "TOTAL HW CLOCK SET FAILURE";
        return;
    }
}

void QBookApp::setWifiStatusAsConnected()
{
    qDebug() << Q_FUNC_INFO;
    getStatusBar()->setWifiStatus(StatusBar::WIFI_LEVEL_3);
}

void QBookApp::initViewer()
{
    qDebug() << Q_FUNC_INFO;

    m_viewer = new Viewer(this);
    connect(m_viewer, SIGNAL(hideMe()), this, SLOT(goToHome()));
    m_viewer->setFormFlags(QBookForm::FLAG_SHARED);
    m_viewer->hide();

    connect(m_viewer, SIGNAL(loadDocError()),                this, SLOT(loadDocError()));
    connect(m_viewer, SIGNAL(viewerConf()),                  this, SLOT(handleViewerConf()));
    connect(m_viewer, SIGNAL(createNewCollection(const BookInfo*)), this, SLOT(createNewCollection(const BookInfo*)));
}

void QBookApp::initLibrary()
{
    qDebug() << Q_FUNC_INFO;

    m_library = new Library(this);
    m_library->setFormFlags(QBookForm::FLAG_SHARED);
    m_library->move(0, getStatusBar()->height());// Until this form has not the valid height...
    m_library->hide();

    connect(m_library, SIGNAL(hideMe()), this, SLOT(goToHome()));
    connect(m_library, SIGNAL(returnToViewer()), this, SLOT(handleGoToViewer()));
    connect(this, SIGNAL(reportStoreLink()), m_library, SLOT(reportedStoreLink()));
}

void QBookApp::initWebView()
{
    qDebug() << Q_FUNC_INFO;

    m_webView = new Browser(this);
    m_webView->setFormFlags(QBookForm::FLAG_SHARED);
    m_webView->move(0, getStatusBar()->height());// Until this form has not the valid height...
    m_webView->hide();

    connect(m_webView,SIGNAL(openBook(const BookInfo*)),this,SLOT(openContent(const BookInfo*)));
    connect(m_webView, SIGNAL(hideMe()), this, SLOT(goToHome()));
}

#ifndef HACKERS_EDITION
void QBookApp::initWebStore()
{
    qDebug() << Q_FUNC_INFO;

    m_webStore = new WebStore(this);
    m_webStore->setFormFlags(QBookForm::FLAG_SHARED);
    m_webStore->move(0, getStatusBar()->height());// Until this form has not the valid height...
    m_webStore->hide();

    connect(m_webStore, SIGNAL(openBook(const BookInfo*)),      this,      SLOT(openContent(const BookInfo*)));
    connect(m_webStore, SIGNAL(hideMe()),                       this,      SLOT(goToHome()));
}

void QBookApp::initWebWizard()
{
    qDebug() << Q_FUNC_INFO;

    m_webWizard = new WebWizard(this);
    m_webWizard->setFormFlags(QBookForm::FLAG_SHARED);
    m_webWizard->hide();

    connect(m_webWizard, SIGNAL(welcomePageLoaded()), this, SLOT(syncRequesterServices()), Qt::QueuedConnection);
    connect(m_webWizard, SIGNAL(hideMe()), this, SLOT(goToHome()));
}

void QBookApp::syncRequesterServices()
{
    qDebug() << Q_FUNC_INFO;
    bool bRet = true;

    QString shopName = QBook::settings().value("shopName", "").toString();
    QString readerPartitionName = QBook::settings().value("readerPartitionName", "reader").toString();
    QString version = QBOOKAPP_VERSION;
    QString wsServicesUrl = QBook::settings().value("wsServicesURL", "").toString();
    QString shopUrl = QBook::settings().value("shopUrl").toString();
    QString shopBookUrl = QBook::settings().value("serviceURLs/book").toString();
    QString shopSearchUrl = QBook::settings().value("serviceURLs/search").toString();
    QString eMail = QBook::settings().value("eMail").toString();
    QString shopUserAccountUrl =  QBook::settings().value("shopUserAccountUrl").toString();
    bool activating =  QBook::settings().value("Activated").toBool();
    QBook::settings().remove("Activated");

    qDebug() << Q_FUNC_INFO << "shopBookUrl: " << shopBookUrl;
    qDebug() << Q_FUNC_INFO << "shopSearchUrl: " << shopSearchUrl;
    qDebug() << Q_FUNC_INFO << "activating: " << activating;
    bRet = shopName.size();

    if(bRet)
        bRet = wsServicesUrl.size();

    QString dictionaryToken = QBook::settings().value("dictionaryToken").toString();

    if(bRet)
        bRet = readerPartitionName.size();

    if(bRet)
        bRet = shopUrl.size();

    if(bRet)
        bRet = shopBookUrl.size();

    if(bRet)
        bRet = shopSearchUrl.size();

    if(bRet && !activating)
        bRet = shopUserAccountUrl.size();

    if(bRet && !activating)
        bRet = eMail.size();

    QString shopIconUrl = QBook::settings().value("shopIcon", "").toString();
    QString offDeviceImageUrl = QBook::settings().value("offDeviceImageUrl", "").toString();
    QString restDeviceImageUrl = QBook::settings().value("restDeviceImageUrl", "").toString();
    QString startDeviceImageUrl = QBook::settings().value("startDeviceImageUrl", "").toString();
    QString updateDeviceImageUrl = QBook::settings().value("updateDeviceImageUrl", "").toString();
    QString lowBatteryDeviceImageUrl = QBook::settings().value("lowBatteryDeviceImageUrl", "").toString();
    QString deviceModelName = QBook::settings().value("deviceModelName", "").toString();

    if(bRet)
    {
        bRet = m_pServices->installContentFromPlatForm(shopIconUrl, offDeviceImageUrl, restDeviceImageUrl, startDeviceImageUrl, updateDeviceImageUrl,
                                                       lowBatteryDeviceImageUrl, dictionaryToken, wsServicesUrl);
    }

    if(bRet)
    {
        if(!activating)
            QBook::settings().setValue("setting/linked",true);
        else
            QBook::settings().remove("eMail");
               
        m_pServices->syncServicesSettings(wsServicesUrl, shopName, version, deviceModelName);

        QBook::settings().setValue("setting/activated",true);
        QBook::settings().setValue("setting/initial",false);

        QBook::settings().sync();

        if(!activating)
        {
            connect(this, SIGNAL(syncHelperCreated()), this, SLOT(synchronization()), Qt::UniqueConnection);
            createSyncHelper();
        }

    }else{
        resetServiceSettings();
        ConfirmDialog* dialog = new ConfirmDialog(this,tr("Link failed. Please try it again in a few minutes."));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        Screen::getInstance()->resetQueue();
        dialog->exec();
        delete dialog;

        goToHome();
    }
}

void QBookApp::closeWizard()
{
    qDebug() << Q_FUNC_INFO;
    QBook::settings().setValue("setting/initial",false);
    getStatusBar()->setIconVisible(StatusBar::BUTTON_READ, true);

    if(isLinked()) emit reportStoreLink(); // Library::gatherNumberOfItemsInFilters();

    if(b_wizardFromStore && isLinked())
    {
        b_wizardFromStore = false;
        goToShop();
    }
    else
        goToHome();
}
#endif

void QBookApp::release()
{
    popAllForms();

    delete m_home;
    m_home = 0;
    delete m_pPublicServices;
    m_pPublicServices = 0;
#ifndef HACKERS_EDITION
    delete m_pServices;
    delete m_bqDeviceKey;
    m_pServices = 0;
    m_bqDeviceKey = 0;
#endif
}

void QBookApp::firseInit()
{
#ifndef HACKERS_EDITION
    if (!m_bqDeviceKey->keysCreated()) {
        qDebug() << "creating rsa keys";
        m_bqDeviceKey->generateDeviceKey();
        qDebug() << "after creating rsa keys: " << m_bqDeviceKey->keysCreated();
    }else
        qDebug() << "rsa keys already created";

    if (QBook::settings().value("setting/initial",true).toBool())
    {
        bLoadWizard = true;
    }
#endif
}

void QBookApp::pushTopForm(QBookForm* form)
{
    qDebug() << Q_FUNC_INFO << form;

    if( !m_activeForms.isEmpty() && (m_activeForms.back() == form) )
        return;

    popAllForms();

    // TODO: We can check if the form is in the m_shownFormIdx, so we don't need to pop it.

    doAddForm(form);

    m_activeForms.append(form);// Append it

    doActivateForm(form);
}

void QBookApp::pushForm(QBookForm* form)
{
    qDebug() << Q_FUNC_INFO << form;

    QBookForm* current_form = m_activeForms.back();
    if(current_form != form)
    {
        doDeactivateForm(current_form);
        qDebug() << Q_FUNC_INFO << "Stack: " << m_activeForms;

        doAddForm(form);

        // Relocate at the end
        m_activeForms.removeAll(form);
        m_activeForms.append(form);

        doActivateForm(form);
    }
}

void QBookApp::popForm(QBookForm* form)
{
    qDebug() << Q_FUNC_INFO << form;

    bool wasLast = false;
    if(m_activeForms.contains(form))
    {
        wasLast = (form == m_activeForms.back());

        m_activeForms.removeAll(form);
        if(wasLast)
            doDeactivateForm(form);
    }

    // Delete it.
    doDeleteForm(form);

    if(wasLast && !m_activeForms.isEmpty())
    {
        qDebug() << Q_FUNC_INFO << "Stack: " << m_activeForms;
        QBookForm* current_form = m_activeForms.back();
        doActivateForm(current_form);
    }
}

void QBookApp::popAllForms()
{
    qDebug() << Q_FUNC_INFO;

    while(!m_activeForms.isEmpty())
    {
//        qDebug() << Q_FUNC_INFO << "Idx: " << m_activeForms;
        QBookForm* form = m_activeForms.takeLast();
        if(form)
        {
            doDeactivateForm(form);
            doDeleteForm(form);
        }
    }
}

void QBookApp::doActivateForm(QBookForm* form)
{
    qDebug() << Q_FUNC_INFO << form;
    getStatusBar()->show();
    form->activateForm();
    form->setFocus();
    form->show();
}

void QBookApp::doDeactivateForm(QBookForm* form)
{
    qDebug() << Q_FUNC_INFO << form;
    form->deactivateForm();
    form->hide();
}

void QBookApp::doDeleteForm(QBookForm* form)
{
    qDebug() << Q_FUNC_INFO << form;
    if(! (form->formFlags() & QBookForm::FLAG_SHARED) )
    {
        m_forms.removeAll(form);
        form->deleteLater();
    }
}

void QBookApp::doAddForm( QBookForm* form )
{
    qDebug() << Q_FUNC_INFO << form;

    // Add the form to the list
    if(!m_forms.contains(form))
        m_forms.append(form);
}

void QBookApp::handleLowBattery(int level)
{
    qDebug() << Q_FUNC_INFO;

    if(ADConverter::getInstance()->getStatus() != ADConverter::ADC_STATUS_NO_WIRE)
        return; // Charging. We don't need to warn

    QString message;
    if(level <= BATTERY_LEVEL_LOW_2)
        message = tr("Critical battery level (%1%). Please plug the device to a charger source.").arg(level);
    else
        message = tr("Battery level low(%1).").arg(level);
    PowerManagerLock* powerLock = PowerManager::getNewLock(this);
    powerLock->activate();

    ConfirmDialog* batteryDialog = new ConfirmDialog(this, message);
    batteryDialog->showForSpecifiedTime(TIME_SHOW_LOWBATTERY_DIALOG);
    delete batteryDialog;

    delete powerLock;
}

/** Auto power off because of critical battery */
void QBookApp::handleCriticalBattery()
{
    qDebug() << "--->" << Q_FUNC_INFO;

    if(ADConverter::getInstance()->getStatus() != ADConverter::ADC_STATUS_NO_WIRE)
        return; // Charging. We don't need to power off or warn.

    QString message = tr("Critical battery level, please connect the charger. The ereader will be turned off.");
    PowerManagerLock* powerLock = PowerManager::getNewLock(this);
    powerLock->activate();

    ConfirmDialog* batteryDialog = new ConfirmDialog(this, message);
    batteryDialog->setTimeLasts(TIME_SHOW_CRITICALBATTERY_DIALOG);

    // Show message only if we are connected
    if(m_connectManager && m_connectManager->isConnected())
    {
        if (isSynchronizing())
        {
            m_sync->AbortSync();
        }
    }

    if(m_connectManager){
        m_connectManager->setOffline();
    }

    //batteryDialog->setText(message);
    batteryDialog->showForSpecifiedTime(TIME_SHOW_CRITICALBATTERY_DIALOG);
    delete batteryDialog;
    if(ADConverter::getInstance()->getStatus() != ADConverter::ADC_STATUS_NO_WIRE)
        return; // Charging. We don't need to power off.

    disconnect(Battery::getInstance(), SIGNAL(batteryLevelCritical()), this, SLOT(handleCriticalBattery()));

    if(DeviceInfo::getInstance()->hasFrontLight() && FrontLight::getInstance()->isFrontLightActive())
        FrontLight::getInstance()->switchFrontLight(false);

    PowerManager::getInstance()->powerOffDevice(QBook::settings().value("setting/initial_lang_selection", true).toBool());
}

void QBookApp::handleFullBattery()
{

#ifdef SHOWCASE
    // Do not show message
    return;
#endif

    // Does nothing with no USB or when is synchronizing
    if(ADConverter::getInstance()->getStatus() == ADConverter::ADC_STATUS_NO_WIRE
            // NOTE (epaz) Cambiado por el paso a nueva interfaz
        #ifndef HACKERS_EDITION
            || (isSynchronizing()) || isLinked()
        #endif
            )
        return;

    ConfirmDialog* confirmDialog = new ConfirmDialog(this,tr("Battery full."));
    Screen::getInstance()->resetQueue();
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE|FLAG_IGNORE_QUEUE,Q_FUNC_INFO);
    confirmDialog->exec();
    delete confirmDialog;
    Screen::getInstance()->refreshScreen(Screen::MODE_SAFE,true);
    return;
}

/** With the charger present stops the auto power off in case i started
* because of low battery
* With the charger gone, it warns the user in case the OTA is being downloaded
*/
void QBookApp::handleChargerChanged(bool state)
{
    qDebug() << Q_FUNC_INFO << state;

    if(!m_usbChangeLock){
        m_usbChangeLock = PowerManager::getNewLock();
        m_usbChangeLock->setTimeOut(USB_CHANGE_SUSPEND_WAIT_TIME);
    }

    m_usbChangeLock->activate();
    if(b_batteryLevel)
        b_batteryLevel->close();
    b_charger_state = state;
    QTimer::singleShot(100, this, SLOT(showChargerDialog()));
    emit userEvent();
}

void QBookApp::showChargerDialog()
{
    qDebug() << Q_FUNC_INFO;

    if(!b_batteryLevel)
    {
        b_batteryLevel = new InfoDialog(this, "", 3000);
        b_batteryLevel->hideSpinner();
    }

    int level = Battery::getInstance()->getLevel();
    if(b_charger_state)
        b_batteryLevel->setText(tr("Cargador conectado. Nivel de bateria: ") + QString::number(level) + "%.");
    else
        b_batteryLevel->setText(tr("Cargador desconectado. Nivel de bateria: ") + QString::number(level) + "%.");

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
    b_batteryLevel->showForSpecifiedTime();

}

void QBookApp::handleUsbStateChanged (bool state)
{
    qDebug() << Q_FUNC_INFO << "state: " << state;

    if(!m_usbChangeLock){
        m_usbChangeLock = PowerManager::getNewLock();
        m_usbChangeLock->setTimeOut(USB_CHANGE_SUSPEND_WAIT_TIME);
    }

    m_usbChangeLock->activate();

    emit userEvent();

    if(link != NULL)
    {
        qDebug() << Q_FUNC_INFO << ": Deleted link to PC";
        delete link;
        link = NULL;
        mass_storage = false;
        return;
    }

    if(isSynchronizing() && !b_isSleeping)
    {
        if(state == true) {
            Screen::getInstance()->queueUpdates();
            InfoDialog *dialog = new InfoDialog(this, tr("No se puede conectar el modo almacenamiento mientras se está sincronizando."), 4000);
            Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
            Screen::getInstance()->flushUpdates();
            Screen::getInstance()->resetQueue();
            dialog->hideSpinner();
            dialog->showForSpecifiedTime();
            delete dialog;
        }
        return;
    }

    QBookForm* current_form = getCurrentForm();
    bool inWizard = QBook::settings().value("setting/initial",true).toBool();
    if (state && !inWizard && !b_isOTAUpdating && !b_isSleeping)
    {
        qDebug() << Q_FUNC_INFO << ": Created link";
        link = new QSettingLinkPc(getCurrentForm());
        connect(link, SIGNAL(connectedToPc(bool)), getStatusBar(), SLOT(updateLinkPcState(bool)));
        connect(link, SIGNAL(connectedToPc(bool)), m_library, SLOT(connectedToPc(bool)));
        connect(link, SIGNAL(connectedToPc(bool)), this, SIGNAL(userEvent()));
#ifndef DISABLE_ADOBE_SDK
        connect(link, SIGNAL(connectedToPc(bool)), AdobeDRM::getInstance(), SLOT(connectedToPc(bool)));
#endif


        qDebug() << Q_FUNC_INFO << ": init QSettingLinkPc";

        QString textMessage;
        // If a book is visible
        if( current_form == m_viewer )
        {
            textMessage = tr("Activate mass storage device? Book will be closed and the device will be temporarily not operable.");
        }
        else
        {
            textMessage = tr("Activate mass storage device? Device will be temporarily not operable.");
        }

        Screen::getInstance()->queueUpdates(); //This queueUpdates have been put here to queue the battey refresh that happens at same time.
        SelectionDialog *dialog = new SelectionDialog(NULL, textMessage);
        connect(link, SIGNAL(connectedToPc(bool)), dialog, SLOT(reject()));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
        Screen::getInstance()->resetQueue();
        dialog->exec();
        int result = dialog->result();
        delete dialog;
        if (result == QDialog::Accepted)
        {
            if( current_form == m_viewer ){
                Screen::getInstance()->queueUpdates();
                goToHome();
                Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
                Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
                Screen::getInstance()->flushUpdates();
            }

            if (link)
            {
                mass_storage = true;
                connect(Storage::getInstance(), SIGNAL(sharingOverUSBEnded()), this, SLOT(closePcLink()), Qt::UniqueConnection);

                // Closing the most recent book if it has DRM rights
                if(m_viewer->hasOpenBook())
                {
                    m_viewer->forceReloading();
                }

                link->linkPc();
            }
        }

    }
    else
    	closePcLink();
}

void QBookApp::closePcLink() 
{
	qDebug() << Q_FUNC_INFO;
    if(link != NULL)
    {
        qDebug() << Q_FUNC_INFO << ": Deleted link to PC";
        link->deleteLater();
        link = NULL;
        mass_storage = false;
        emit userEvent();
     }
 }

void QBookApp::handlePartitionMounted(StoragePartition *partition) {
        qDebug() << Q_FUNC_INFO << "received partitionMounted signal for device " << partition->getDevice();
	handleRemoveableChanged(true, partition->getMountPoint());
    checkScreensaverCover();
    if(partition->getMountPoint() == Storage::getInstance()->getPublicPartition()->getMountPoint())
        checkBooksChanged();
}

void QBookApp::handlePartitionUmounted(StoragePartition *partition) {
        qDebug() << Q_FUNC_INFO << "received partitionUmounted signal for device " << partition->getDevice();
	handleRemoveableChanged(false, partition->getMountPoint());
    checkScreensaverCover();
}

/* This is called when a partition has been mounted or umounted */
void QBookApp::handleRemoveableChanged(bool state, const QString& path)
{
   qDebug() << Q_FUNC_INFO << "with state: " << state <<  "and path: " << path;

   disableUserEvents();

   QBookForm* current_form = getCurrentForm();

     /* true means partition is mounted and false partition is umounted*/
    if (state)
    {
        PowerManagerLock *mountingLock = PowerManager::getNewLock();
        mountingLock->activate();

        if(!readingFilesdialog)
            readingFilesdialog = new InfoDialog(this, tr("Reading files..."));

        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        readingFilesdialog->show();
        QCoreApplication::flush();
        QCoreApplication::processEvents();
        connect(m_pModel,  SIGNAL(loadingBooks(int)), this, SLOT(showLoadingBooksDialog(int)), Qt::UniqueConnection);
        connect(m_pModel,  SIGNAL(loadFinished()),    this, SLOT(hideLoadingBooksDialog()), Qt::UniqueConnection);

        m_pModel->addDir(path);
        if(readingFilesdialog)
        {
            delete readingFilesdialog;
            readingFilesdialog = NULL;
        }
        mountingLock->release();
        delete mountingLock;
    }else
        m_pModel->removeDir(path);

    if(current_form == m_home){
        m_home->setupHome();
    }

    // Do not unlock if sleeping
    if(!b_isSleeping)
        enableUserEvents();

#ifndef HACKERS_EDITION
    // Show debug dialog if magic file found in inserted card
    if(state){ // Mounting partition
        // Check magic file
        QString magicFilePath("");
        if(Storage::getInstance()->getRemovablePartition() != NULL && path == Storage::getInstance()->getRemovablePartition()->getMountPoint()){
            magicFilePath = Storage::getInstance()->getRemovablePartition()->getMountPoint();
            magicFilePath.append("/" + QBookDebugDialog::getMagicFileName(MAGIC_FILE));
        }
        if(!magicFilePath.isEmpty() && checkDebugMagicFile(magicFilePath,QBookDebugDialog::getMagicFileName(MAGIC_STRING))){
            /** Capture the live dmesg immediatly after inserting a SD card with magic file inside
            *   This might help capturing low-level problems.
            */
            QString command = "dmesg -T > /mnt/sd/dmesg-auto." +
                    QDateTime::currentDateTime().toString("MMdd_hhmm") + ".log";

            qDebug() << Q_FUNC_INFO << "command" << command;
            char *cmd = command.toLatin1().data();
            system(cmd);
            system("sync");


            if(!m_debugDialog)
                m_debugDialog = new QBookDebugDialog(this);
            m_debugDialog->setup();
            m_debugDialog->exec();
        }

        if(Storage::getInstance()->getRemovablePartition() != NULL && path == Storage::getInstance()->getRemovablePartition()->getMountPoint()) {
            // Entry point for hackers SD check
            QString updateCheckFile;
            if(DeviceInfo::getInstance()->getHwId() == DeviceInfo::E60Q22) //Q22
                updateCheckFile = HACKERS_UPDATE_CHECK_FILE_Q22;
            else // 672 and A22
                updateCheckFile = HACKERS_UPDATE_CHECK_FILE_672;

            QFile authFile(Storage::getInstance()->getPrivatePartition()->getMountPoint() + updateCheckFile);
            if(authFile.exists()){ // Alredy authorised
                InfoDialog *authorisedDialog = new InfoDialog(this);
                authorisedDialog->setText(tr("Device already authorised for hackers firmware installation"));
                authorisedDialog->hideSpinner();
                authorisedDialog->showForSpecifiedTime();
                delete authorisedDialog;
            }
            else if (isLinked() && checkSDForHackers()) {
                if (askForHackersInstallation())  // Ask for authorization
                    allowHackersInstallation();
            }
        }
    }
#endif
}

/*-------------------------------------------------------------------------*/

#ifndef HACKERS_EDITION
bool QBookApp::checkDebugMagicSignature(QByteArray line, QByteArray signature)
{
	return m_bqDeviceKey->checkSignatureForMagic(line, signature);
}

bool QBookApp::checkDebugMagicFile(const QString& magicFileName, const QString& code)
{
    qDebug() << "--->" << Q_FUNC_INFO;

    QFile *file = new QFile(magicFileName);
    if (!file->exists())
        return false;


    QString qs_snum = DeviceInfo::getInstance()->getSerialNumber();
    QByteArray hashData;
    hashData.append(qs_snum);
    hashData.append(code);
    QByteArray hash = QCryptographicHash::hash(hashData,QCryptographicHash::Sha1).toHex();
    if (!file->open(QIODevice::ReadOnly))
        return false;

    QByteArray line = file->readLine().trimmed();
    QByteArray signature = file->readAll();
    file->close();
    if (line != hash) {
        qDebug() << Q_FUNC_INFO << "FAILED hash check with line" << line << "and hash" << hash;
        return false;
    }

   if (!checkDebugMagicSignature(line, signature)) {
        qDebug() << Q_FUNC_INFO << "FAILED signature check with hash" << line << "and signature" << signature;
	return false;
   }

    return true;
}
#endif

/*virtual*/ void QBookApp::keyPressEvent(QKeyEvent *event)
{
    qDebug() << Q_FUNC_INFO;

    if(isLedDebuggingOff())
        Power::getInstance()->setLed(true);

    if (event->key() == QBook::QKEY_POWER) {
            event->accept();
            if(m_timer_light.isActive()) // Home key pressed -> screenShot
            {
                m_homeKeyLock->release();
                m_timer_light.stop();
                takeScreenShot();
            }
            else // Sleep or shutdown process
            {
                m_powerKeyLock->activate();
                m_timer_powerButton.start();
            }
    }else if(event->key() == QBook::QKEY_HOME){
            event->accept();
            if(m_timer_powerButton.isActive())
            {
                m_powerKeyLock->release();
                m_timer_powerButton.stop();
                takeScreenShot();
            }
            else
            {
                m_homeKeyLock->activate();
                m_timer_light.start();
            }
    }else
        QWidget::keyPressEvent(event);
}

/*virtual*/ void QBookApp::keyReleaseEvent(QKeyEvent* event)
{
    qDebug() << Q_FUNC_INFO << (uint)event->key() << QApplication::focusWidget();

    if(isLedDebuggingOff() && !b_powerOffOngoing)
        Power::getInstance()->setLed(false);

    if( event->key() == QBook::QKEY_HOME)
    {
        event->accept();
        m_homeKeyLock->release();

        // No more action if release after FL switch on
        if(!m_timer_light.isActive() )
            return;

        m_timer_light.stop();

        if(QBook::settings().value("setting/initial",true).toBool()){
            QBook::settings().setValue("setting/initial",false);
            QBook::settings().setValue("setting/initial_lang_selection",false);
        }

        wifiConnectionCanceled();
        b_wizardFromStore = false;
        goToHome();

    }
    else if( event->key() == QBook::QKEY_MENU)
    {
        if(QBook::settings().value("setting/initial",true).toBool()){
            QBook::settings().setValue("setting/initial",false);
            QBook::settings().setValue("setting/initial_lang_selection",false);
        }
        wifiConnectionCanceled();
        event->accept();
        b_wizardFromStore = false;
        goToLibrary();

    }
    else if (event->key() == QBook::QKEY_POWER)
    {
        qDebug() << Q_FUNC_INFO << "Release of power key";

        if (m_timer_powerButton.isActive()){
            m_timer_powerButton.stop();
            m_powerKeyLock->release();
            if(!b_powerOffOngoing && !b_keyBlocked) // Hack to block going to sleep inmediatly after wake up. EREADER-2000
                goToSleep();
        }
        event->accept();
    }
    else if (!event->modifiers())
    {
        event->accept();
    }
    else
    {
        QWidget::keyReleaseEvent(event);
    }
}

void QBookApp::tabChange(int /*cmd*/)
{
   // Do Nothing
}

void QBookApp::setupSSLconfig (const QString& path)
{
        qDebug() << Q_FUNC_INFO << path;

        QList<QSslCertificate> certList = QSslCertificate::fromPath(path, QSsl::Pem, QRegExp::Wildcard);

        foreach(const QSslCertificate& cert, certList)
        {
            qDebug() << Q_FUNC_INFO << "adding SSL certificate for " << cert.issuerInfo(QSslCertificate::Organization);
        }

        QSslConfiguration::defaultConfiguration().setCaCertificates(certList);
        QSslConfiguration::defaultConfiguration().setPeerVerifyDepth(3);

        QSslConfiguration::defaultConfiguration().setPeerVerifyMode(QSslSocket::VerifyPeer);
        QSslConfiguration::defaultConfiguration().setProtocol(QSsl::TlsV1);

}


void QBookApp::confirmFwUpgrade(){

    qDebug() << Q_FUNC_INFO;
    disconnect(m_connectManager,SIGNAL(connected()),this,SLOT(checkFwVersion()));

    QString fwVerName = fwDwld->getOTAVersionString();

    SelectionDialog* confirmDialog = new SelectionDialog(this,tr("New version available: ").append(fwVerName.append("\n")).append(tr("Do you want to view details and download?")));
    // Disconnect more checks
    connect(this,SIGNAL(askingPowerOff()),confirmDialog,SLOT(doReject()));
    Screen::getInstance()->resetQueue();
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
    confirmDialog->exec();

    int result = confirmDialog->result();
    delete confirmDialog;
    if(!result)
        return;

    // This is a big ugly hack to bring up the settings screen.
    // This is leaking SettingsDeviceInfo as the main settings dialog is doing.
    Screen::getInstance()->queueUpdates();
    goToSettings();
    m_settingsMenu->hidePreviousChild();
    QBookForm *current_form = getCurrentForm();
    SettingsDeviceInfo *elfDeviceMenu = ((Settings*)current_form)->showInformation();
    elfDeviceMenu->showUpgradeMenu();
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();

    //Reconnnect check again
    connect(m_connectManager,SIGNAL(connected()),this,SLOT(checkFwVersion()), Qt::UniqueConnection);
}

void QBookApp::powerLight()
{
        qDebug() << Q_FUNC_INFO;
        if(!DeviceInfo::getInstance()->hasFrontLight()){
            qDebug() << "This device doesn't have front light";
            return;
        }

        bool current = FrontLight::getInstance()->isFrontLightActive();
        int brightness = FrontLight::getInstance()->getBrightness();
        if(!current)
            FrontLight::getInstance()->setBrightness(brightness);
        FrontLight::getInstance()->setFrontLightActive(!current);

        m_homeKeyLock->release();
}

void QBookApp::askPowerOffDevice()
{
    m_timer_powerButton.stop();
    emit askingPowerOff();
    m_powerKeyLock->release();

    if(mass_storage) {
        InfoDialog *dialog = new InfoDialog(this, tr("Disconnect USB from CPU before power off the device."), 3000);
        dialog->hideSpinner();
        dialog->showForSpecifiedTime();
        delete dialog;
        return;
    }

    SelectionDialog* dialogSelect = new SelectionDialog(NULL,tr("Are you sure you want to turn off the device?"));
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
    Screen::getInstance()->resetQueue();

    b_powerOffOngoing = true;

    dialogSelect->exec();

    if(!dialogSelect->result()){
        b_powerOffOngoing = false;
        delete dialogSelect;
        return;
    }
    shuttingDown();
}

void QBookApp::shuttingDown()
{
    b_powerOffOngoing = true;

    PowerManagerLock *shuttingDownLock = PowerManager::getNewLock();
    shuttingDownLock->activate();

    disconnect(Storage::getInstance(), SIGNAL(partitionMounted(StoragePartition*)),  this, SLOT(handlePartitionMounted(StoragePartition*)));
    disconnect(Storage::getInstance(), SIGNAL(partitionUmounted(StoragePartition*)), this, SLOT(handlePartitionUmounted(StoragePartition*)));

    Screen::getInstance()->queueUpdates();
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_WAITFORCOMPLETION, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
    qApp->processEvents();

    if(DeviceInfo::getInstance()->hasFrontLight() && FrontLight::getInstance()->isFrontLightActive())
        FrontLight::getInstance()->switchFrontLight(false);

    shuttingDownLock->release();
    delete shuttingDownLock;

    PowerManager::powerOffDevice(QBook::settings().value("setting/initial_lang_selection", true).toBool());
}

Keyboard* QBookApp::showKeyboard(const QString& actionText, bool showUp, Keyboard::MODE keyboardMode)
{
    qDebug() << Q_FUNC_INFO;

    m_pKeyboard->setActionKeyText(actionText);

    if(!m_pKeyboard->isVisible())
    {
        qDebug() << Q_FUNC_INFO << "keyboard not visible";
        if(!showUp){//WorkAround We dont have more time to fix it better
            m_pKeyboard->move(0,Screen::getInstance()->screenHeight() - (Screen::getInstance()->screenHeight() == 1024 ? 384 : 300));
        }else
            m_pKeyboard->move(0,0);

        m_pKeyboard->show(keyboardMode);
    }
    return m_pKeyboard;
}

Keyboard* QBookApp::hideKeyboard()
{
    qDebug() << Q_FUNC_INFO << m_pKeyboard;

    if(m_pKeyboard && m_pKeyboard->isVisible())
    {
        m_pKeyboard->hide();
    }

    return m_pKeyboard;
}

Viewer* QBookApp::getViewer()
{
    if(!m_viewer)
        initViewer();

    return m_viewer;
}

#ifndef HACKERS_EDITION
WebStore* QBookApp::browserStore()
{
    if(!m_webStore)
        initWebStore();

    return m_webStore;
}

WebWizard* QBookApp::browserWizard()
{
    if(!m_webWizard)
        initWebWizard();

    return m_webWizard;
}
#endif


Browser* QBookApp::browser()
{
    if(!m_webView)
        initWebView();

    return m_webView;
}

void QBookApp::goToManual()
{
    qDebug() << "--->" << Q_FUNC_INFO;
    QString lang = QBook::settings().value("setting/language", QVariant("es")).toString();
    QString manualFilePath = "";
    manualFilePath.append(MANUALS_PATH).append("manual-").append(lang.toUpper()).append(".epub");

    qDebug() << "OPENING MANUAL IN " << manualFilePath;

    BookInfo* manual = new BookInfo(manualFilePath);
    manual->corrupted = false;
    openContent(manual);
    delete manual;
}

void QBookApp::goToHome()
{
    qDebug() << Q_FUNC_INFO;

    emit goingHome();

    disconnect(this, SIGNAL(wifiConnectionCanceled()), this, SLOT(goToHome()));
    Screen::getInstance()->queueUpdates();
    hideKeyboard();
    pushTopForm(m_home);
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

}

void QBookApp::goToLibrary()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    hideKeyboard();

    pushTopForm(m_library);
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

#ifndef HACKERS_EDITION
void QBookApp::goToShop()
{
    qDebug() << Q_FUNC_INFO;

    disconnectWifiObserverActivateStore();
    disconnectWifiObserverGoToShop();
    if(!m_webStore)
        initWebStore();

    Screen::getInstance()->queueUpdates();
    hideKeyboard();
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_WAITFORCOMPLETION,Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    if(!QBookApp::instance()->isLinked())
    {
        b_wizardFromStore = true;
        goToWizard();
    }else
    {
        pushTopForm(m_webStore);
        if(m_connectManager->isConnected())
        {
            m_sync->sendTransactions();
            m_webStore->activateFormWithConnection();
        }
        else
        {
            connectWifiObserverActivateStore();
            connect(this, SIGNAL(wifiConnectionCanceled()), this, SLOT(goToHome()), Qt::UniqueConnection);
            connect(this, SIGNAL(wifiConnectionCanceled()), this, SLOT(disconnectWifiObserverActivateStore()), Qt::UniqueConnection);
            requestConnection(true);
        }
    }
}

void QBookApp::connectWifiObserverActivateStore()
{
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), m_webStore, SLOT(activateFormWithConnection()), Qt::UniqueConnection);
}

void QBookApp::disconnectWifiObserverActivateStore()
{
    disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), m_webStore, SLOT(activateFormWithConnection()));
}

void QBookApp::goToWizard()
{
    qDebug() << Q_FUNC_INFO;
    if(m_welcomeWizard)
        delete m_welcomeWizard;

    Screen::getInstance()->queueUpdates();
    hideKeyboard();
    m_welcomeWizard = new WizardWelcome(this);
    m_welcomeWizard->setFormFlags(QBookForm::FLAG_SHARED);
    connect(m_welcomeWizard, SIGNAL(closeWizard()), this,SLOT(closeWizard()));
    connect(m_welcomeWizard, SIGNAL(closeWizard()), this,SLOT(checkNewDictionaries()), Qt::UniqueConnection);
    pushTopForm(m_welcomeWizard);

    Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_WAITFORCOMPLETION,Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}
#endif

void QBookApp::goToSearch()
{
        qDebug() << Q_FUNC_INFO;
        Screen::getInstance()->queueUpdates();
        hideKeyboard();

        if(!m_search) {
            m_search = new Search(this);
            m_search->setFormFlags(QBookForm::FLAG_SHARED);
            m_search->move(0, getStatusBar()->height());// Until this form has not the valid height...
        }
        pushTopForm(m_search);
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
}

void QBookApp::goToSettings()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    hideKeyboard();

    if(!m_settingsMenu) {
        m_settingsMenu = new Settings(this);
        m_settingsMenu->setFormFlags(QBookForm::FLAG_SHARED);
        m_settingsMenu->move(0, getStatusBar()->height());// Until this form has not the valid height...
        connect(m_settingsMenu,SIGNAL(hideMe()),this,SLOT(goToHome()));
    }
    pushTopForm(m_settingsMenu);
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_WAITFORCOMPLETION,Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void QBookApp::goToBrowser()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    getStatusBar()->setMenuTitle(tr("Browser"));

    if(!m_webView)
        initWebView();
    pushTopForm(m_webView);
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_WAITFORCOMPLETION,Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    requestConnection();
}

void QBookApp::openContent(const BookInfo* content)
{
    qDebug() << Q_FUNC_INFO << content->path;

    if (content == NULL)
        return;

    disconnect(fwDwld, SIGNAL(checkOTAAvailableFinished(int)), this, SLOT(gotOTACheckResult(int)));
    disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(checkFwVersion()));

    Screen::getInstance()->queueUpdates();
    getStatusBar()->setBusy(true);
    Screen::getInstance()->flushUpdates();

    hideKeyboard();

    if(Viewer::isSupportedFile(content->path))
    {
        if (content->isCorrupted()) {
            InfoDialog* corruptedDialog = new InfoDialog(this);
            corruptedDialog->setTimeLasts(3000);
            corruptedDialog->hideSpinner();
            //FIXME: Get proper text for this and translated
            corruptedDialog->setText(tr("Cannot open book. It is corrupted"));
            corruptedDialog->hideSpinner();
            corruptedDialog->showForSpecifiedTime();
            delete corruptedDialog;
        }
        else
        {
            PowerManagerLock *openingBookLock = PowerManager::getNewLock(this);;
            openingBookLock->activate();

            if(!m_viewer)
                initViewer();

            if(content->m_type == BookInfo::BOOKINFO_TYPE_SUBSCRIPTION && content->hasExpired())
            {
                ConfirmDialog* confirmDialog = new ConfirmDialog(this,tr("Your subscription has expired so the book can't be opened."));
                Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
                confirmDialog->exec();
                delete confirmDialog;
                if(getCurrentForm() == m_home)
                    m_home->setupHome();
                if(getCurrentForm() == m_library)
                    m_library->setupLibrary();
                getStatusBar()->setBusy(false);
                connect(fwDwld, SIGNAL(checkOTAAvailableFinished(int)), this, SLOT(gotOTACheckResult(int)), Qt::UniqueConnection);
                connect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(checkFwVersion()), Qt::UniqueConnection);
                return;
            }

            disableUserEvents();
            m_openDocError = false;

            m_viewer->openDoc(content);
            Screen::getInstance()->queueUpdates();
            if(!m_openDocError && !content->hasExpired())
                pushTopForm(m_viewer);

            getStatusBar()->setBusy(false);
            Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
            Screen::getInstance()->flushUpdates();

            QTimer::singleShot(500, this, SLOT(enableUserEvents()));

            if(!m_openDocError)
                setScreensaverCover(content);
            openingBookLock->release();
            delete openingBookLock;

            connect(fwDwld, SIGNAL(checkOTAAvailableFinished(int)), this, SLOT(gotOTACheckResult(int)), Qt::UniqueConnection);
            connect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(checkFwVersion()), Qt::UniqueConnection);
            return;
        }
    }
    else if(isImage(content->path))
        emit openImage(content->path);
    else
    {
        qDebug() << Q_FUNC_INFO << "Format not supported";
        InfoDialog* formatNotSupportedDialog= new InfoDialog(this,tr("Document format not supported"));
        formatNotSupportedDialog->hideSpinner();
        formatNotSupportedDialog->showForSpecifiedTime();
        delete formatNotSupportedDialog;
    }

    getStatusBar()->setBusy(false);

    connect(fwDwld, SIGNAL(checkOTAAvailableFinished(int)), this, SLOT(gotOTACheckResult(int)), Qt::UniqueConnection);
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(checkFwVersion()), Qt::UniqueConnection);
}

void QBookApp::openLastContent()
{
    qDebug() << Q_FUNC_INFO;

    if(m_passwordRequester)
        m_passwordRequester->passwordDialogCancel(); //WorkAround to hide PassWordRequester when open a book in the password menu EREADER-1326

    if(!m_viewer)
        initViewer();

    const BookInfo *currentBook = m_viewer->getCurrentBookInfo();

    if (m_viewer && currentBook && m_viewer->hasOpenBook())
    {
        const BookInfo* modelBook = m_pModel->getBookInfo(currentBook->path);
        if(modelBook && !modelBook->m_archived)
        {
            getStatusBar()->setBusy(true);
            // Hides keyboard
            if(m_pKeyboard && m_pKeyboard->isVisible())
                hideKeyboard();

            PowerManagerLock *openingLastLock = PowerManager::getNewLock(this);
            openingLastLock->activate();
            if(!modelBook->hasExpired())
            {
                m_viewer->openDoc(modelBook);
                if (!m_openDocError)
                {
                    Screen::getInstance()->queueUpdates();
                    pushTopForm(m_viewer);// TODO: if an error ocurred, we can skip this and show the dialog!
                    getStatusBar()->setBusy(false);
                    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
                    Screen::getInstance()->flushUpdates();
                }
            }
            else if(modelBook->m_type == BookInfo::BOOKINFO_TYPE_SUBSCRIPTION)
            {
                m_viewer->resetViewerBookInfo();
                ConfirmDialog* confirmDialog = new ConfirmDialog(this,tr("Your subscription has expired so the book can't be opened."));
                Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
                confirmDialog->showForSpecifiedTime(TIME_SHOW_SUBSCRIPTION_DIALOG);
                if(getCurrentForm() == m_home)
                    m_home->setupHome();
                else
                    goToHome();
                // Queue done opening book from library
                getStatusBar()->setBusy(false);

                delete confirmDialog;
            }

            if(m_openDocError)
            {
                Screen::getInstance()->queueUpdates();
                if(getCurrentForm() == m_home)
                    m_home->setupHome();
                else
                    goToHome();
                Screen::getInstance()->flushUpdates(); // Queue done opening book from library
                getStatusBar()->setBusy(false);
            }
            openingLastLock->release();
            delete openingLastLock;
            return;
        }
    }

    // If we are here, the viewer didn't have a recent book valid to open.
    // So go to the model to find one.
    QList<const BookInfo*> recentBooks;
    m_pModel->getBooks(recentBooks, Model::LIBRARY_SORT_READING_TIME, true, "", false);
    if (recentBooks.size() > 0)
    {
        getStatusBar()->setBusy(true);
        const BookInfo *lastBook = recentBooks.at(0);
        openContent(lastBook);
        getStatusBar()->setBusy(false);
    }
}

StatusBar* QBookApp::getStatusBar()
{
    return titleBar;
}

int QBookApp::requestConnection(bool allowedToRequestInteraction, bool wizard)
{
    qDebug() << "--->" << Q_FUNC_INFO << "ConnectionManager: isOffline: " << m_connectManager->isOffline();
    qDebug() << "--->" << Q_FUNC_INFO << "ConnectionManager: isConnected: " << m_connectManager->isConnected();

    if (m_connectManager->isConnected()) { //is connected
        return CONNECTED;
    }

    if(!allowedToRequestInteraction && !QBook::settings().value("wifi/disabled",false).toBool()) {
        Wifi::getInstance()->powerOn();
        isPoweringOff = false;
        m_connectManager->powerWifi(true);
        m_connectManager->setOnline();
        if(m_autoconnect_lock)
            m_autoconnect_lock->activate();
        return AUTOCONNECTING;
    }

    if (QBook::settings().value("wifi/disabled",false).toBool() && allowedToRequestInteraction) {
        SelectionDialog* selDialog = new SelectionDialog(this, tr("Wifi connection is disabled. Would you like to turn it on?"));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        Screen::getInstance()->resetQueue();
        selDialog->exec();
        if(!selDialog->result()) {
            emit wifiConnectionCanceled();
            delete selDialog;
            return NOT_CONNECTED;
        }
     }

    if(allowedToRequestInteraction){
        if (wizard)
            showWifiSelection(SettingsWiFiList::NO_ACTION_BEHAVIOR);
        else
            showWifiSelection(SettingsWiFiList::AUTOMATIC_CLOSE_BEHAVIOUR);
        return WIFI_LIST;
    }

    return NOT_CONNECTED;
}

void QBookApp::showQuickSettingsPopup()
{
        qDebug() << Q_FUNC_INFO;

        if(!quickSettingsPopup){
            quickSettingsPopup = new SettingsQuickSettingsPopup(this);
            connect(quickSettingsPopup,SIGNAL(userEvent()),this,SIGNAL(userEvent()));
        }

        if(getCurrentForm() == m_viewer){
            quickSettingsPopup->enabledWifi(false);
        }else{
            quickSettingsPopup->enabledWifi(true);
        }


        qDebug() << Q_FUNC_INFO;
        Screen::getInstance()->queueUpdates();
        hideKeyboard();
        quickSettingsPopup->show();
        quickSettingsPopup->raise();
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
}

void QBookApp::showWifiSelection(SettingsWiFiList::eBehavior behaviour) {
    qDebug() << Q_FUNC_INFO << "behaviour" << behaviour;
    if(!m_wifiSelection){
        m_wifiSelection = new SettingsWiFiList(this, behaviour);
        m_wifiSelection->setFormFlags(QBookForm::FLAG_SHARED);

        connect(m_wifiSelection, SIGNAL(wifiConnectionCanceled()), this, SIGNAL(wifiConnectionCanceled()));
        connect(PowerManager::getInstance(),SIGNAL(backFromSleep()),m_wifiSelection,SLOT(resetSleepTimer()));
    }
    m_wifiSelection->setBehaviour(behaviour);

    if(behaviour == SettingsWiFiList::NO_ACTION_BEHAVIOR){
        m_wifiSelection->setMinimumSize(Screen::getInstance()->screenWidth(), Screen::getInstance()->screenHeight());
        m_wifiSelection->adjustSize();
        m_wifiSelection->setFixedSize(Screen::getInstance()->screenWidth(), Screen::getInstance()->screenHeight());
        m_wifiSelection->move(0, 0);

    }else{
        m_wifiSelection->setMinimumSize(Screen::getInstance()->screenWidth(), Screen::getInstance()->screenHeight() - getStatusBar()->height());
        m_wifiSelection->adjustSize();
        m_wifiSelection->setFixedSize(Screen::getInstance()->screenWidth(), Screen::getInstance()->screenHeight() - getStatusBar()->height());
        m_wifiSelection->move(0, getStatusBar()->height());
    }

    Screen::getInstance()->queueUpdates();
    hideKeyboard();
    pushForm(m_wifiSelection);
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

#ifndef HACKERS_EDITION
void QBookApp::openStoreWithUrl(const QString& url)
{
    qDebug() << Q_FUNC_INFO << "url to open: " << url;

    if(!m_webStore)
        initWebStore();

    if(!url.isEmpty())
        m_webStore->forceUrl(url);
    else
        m_webStore->resetVisitedStore();

    if(ConnectionManager::getInstance()->isConnected())
        goToShop();
    #ifndef FAKE_WIFI
    else
    {
        requestConnection(true);
        connectWifiObserverGoToShop();
        connect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disconnectWifiObserverGoToShop()), Qt::UniqueConnection);
    }
    #endif
}
#endif

void QBookApp::connectWifiObserverGoToShop()
{
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(goToShop()), Qt::UniqueConnection);
}

void QBookApp::disconnectWifiObserverGoToShop()
{
    disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(goToShop()));
}

void QBookApp::openBrowser(const QString& url)
{
    qDebug() << Q_FUNC_INFO;
    if(!m_webView)
        initWebView();
    m_webView->forceUrl(url);
    goToBrowser();
}

bool QBookApp::eventFilter(QObject* watched, QEvent* event)
{
    switch(event->type()) {

    // User input events
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        emit userEvent();
        if(b_userEventsBlocked) {
            // We want to process power key while sleeping so it can be cancelled
            if (event->type() == QEvent::KeyRelease && ((QKeyEvent*)event)->key() == QBook::QKEY_POWER) {
                if (b_isSleeping) {
                    qDebug() << Q_FUNC_INFO << "power key released with events disabled while going to sleep. Canceling sleep";
                    cancelOnGoingSleep();
                    if(isLedDebuggingOff())
                        Power::getInstance()->setLed(false);
                    return true;
                } else {
                    qDebug() << Q_FUNC_INFO << "power key released with event disabled but we are not going to sleep. Could be the poweron one. Ignoring";
                }
            }
            qDebug() << Q_FUNC_INFO << "Touch and key events temporarily blocked";
            return true;
        }
    default:
        return QWidget::eventFilter(watched, event);
    }
}

bool QBookApp::isImage(const QString& path)
{
    if(path.isEmpty()) return false;

    if(supportedImageslist.isEmpty())
        supportedImageslist = QImageReader::supportedImageFormats();

    qDebug() << Q_FUNC_INFO << "supportedImageslist: " << supportedImageslist;
    QString fileName = path.split("/").last();
    QString fileExtension = fileName.split(".").last();
    return supportedImageslist.contains(fileExtension.toLower().toAscii()) && fileExtension.toLower() != "mng";
}

void QBookApp::loadDocError()
{
    qDebug() << Q_FUNC_INFO;
    m_openDocError = true;
}

void QBookApp::WifiLevelChanged(int value) {
	// cm values are from 0 to 100
    qDebug() << Q_FUNC_INFO << value;

    StatusBar::WifiStatusEnum level;
    if(value > 75)
        level = StatusBar::WIFI_LEVEL_4;
    else if (value > 50)
        level = StatusBar::WIFI_LEVEL_3;
    else if (value > 25)
        level = StatusBar::WIFI_LEVEL_2;
    else
        level = StatusBar::WIFI_LEVEL_1;

    getStatusBar()->setWifiStatus(level);
}

/// Check firmware version at server
void QBookApp::checkFwVersion(){
#ifdef HACKERS_EDITION
    return;
#endif
    qDebug() << Q_FUNC_INFO << "Starting FW version check";

    QDateTime current = QDateTime::currentDateTime();
    if(m_lastOTACheck.secsTo(current) < (3600 * 24)) // Ignore if requested less than 1 day ago
        return;

    m_lastOTACheck = current;

    connect(fwDwld, SIGNAL(checkOTAAvailableFinished(int)), this, SLOT(gotOTACheckResult(int)));
    if (!fwDwld->checkOTAAvailable()) {
    	disconnect(fwDwld, SIGNAL(checkOTAAvailableFinished(int)), this, SLOT(gotOTACheckResult(int)));
        if (m_viewer && m_viewer == getCurrentForm()) {
            qDebug() << Q_FUNC_INFO << "We are in the viewer, not showing the user anything";
            return;
        }
    }
}

void QBookApp::checkLink()
{
    qDebug() << Q_FUNC_INFO << "Check link";
#ifndef HACKERS_EDITION
    if (isActivated() || isLinked() || isSleeping() || QBook::settings().value("setting/initial",true).toBool())
        return;

    qDebug() << Q_FUNC_INFO << "m_lastLinkCheck: " << m_lastLinkCheck;
    if(m_lastLinkCheck.daysTo(QDateTime::currentDateTime()) < 15 || m_lastActivatedCheck.secsTo(QDateTime::currentDateTime()) < TIME_SHOW_ACTIVATION_DIALOG)
        return;

    QBook::settings().setValue("setting/lastLinkCheck", QVariant(QDateTime::currentDateTime().toString(Qt::ISODate)));
    m_lastLinkCheck = QDateTime::currentDateTime();

    if(!mass_storage)
    {
        SelectionDialog* linkSelect = new SelectionDialog(NULL,tr("¿Por que no enlazas el dispositivo para poder hacer uso de los diccionarios y recibir avisos de las actualizaciones disponibles?"), tr("Enlazar"), tr("Mas tarde"));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        Screen::getInstance()->resetQueue();
        linkSelect->exec();
        bool result = linkSelect->result();
        delete linkSelect;
        if(result && getCurrentForm() != m_webWizard)
            goToWizard();
    }
#endif
}

void QBookApp::gotOTACheckResult(int result) {
    disconnect(fwDwld, SIGNAL(checkOTAAvailableFinished(int)), this, SLOT(gotOTACheckResult(int)));
    if (result == QFwUpgradeOTA::Error) {
        qDebug() << Q_FUNC_INFO << "Error checking OTA, doing nothing";
    } else if (result == QFwUpgradeOTA::NotAvailable){
        qDebug() << Q_FUNC_INFO << "No OTA available , doing nothing";
    } else if (result == QFwUpgradeOTA::Available) {
        qDebug() << Q_FUNC_INFO << "OTA available , asking for download";
        confirmFwUpgrade();
    }
}

void QBookApp::lostConnection()
{
    QBookApp::instance()->getStatusBar()->setWifiStatus(StatusBar::WIFI_OFF);

    if (isSynchronizing())
        m_sync->AbortSync();
}

QBookForm* QBookApp::getCurrentForm() const
{
    qDebug() << Q_FUNC_INFO;

    if(!m_activeForms.isEmpty())
        return m_activeForms.back();
    return NULL;
}

void QBookApp::prepareToShutdown()
{
    qDebug() << Q_FUNC_INFO;
    cancelSync();
    prepareViewerToSync();
    removeExpiredBooks();
    sync();
}

void QBookApp::partitionUmountFailed(StoragePartition* partition){
       qDebug() << "--->" << Q_FUNC_INFO;
       if(partition->getPartitionType() == StoragePartition::REMOVABLE){
            handleRemoveableChanged(false, partition->getMountPoint());
            InfoDialog* errorDialog = new InfoDialog(this);
            errorDialog->setTimeLasts(10000);
            errorDialog->setText(tr("La extracción de la tarjeta MicroSD ha generado un error. \n Si desea seguir usandola o cualquier otra debe reiniciar el dispositivo"));
            errorDialog->hideSpinner();
            errorDialog->showForSpecifiedTime();
            delete errorDialog;
       }
}

void QBookApp::deviceRemoved(StorageDevice* partition)
{
    // If a book is visible
    QBookForm* current_form = getCurrentForm();
    if( current_form != m_viewer ||
            !m_viewer->getCurrentBookInfo() ||
            partition == NULL ||
            partition->getType() != StorageDevice::REMOVABLE)
        return;

    QString currentPath = m_viewer->getCurrentBookInfo()->path;


    // SD is being removed and the current book is in it -> shows a dialog and goes to home
    if(!currentPath.contains(Storage::getInstance()->getPublicPartition()->getMountPoint()) &&
            !currentPath.contains(Storage::getInstance()->getPrivatePartition()->getMountPoint()) &&
            !currentPath.contains(QString(USERGUIDEPATH))) {

        ConfirmDialog* dialog = new ConfirmDialog(this,tr("The SD card was removed. The current book has been closed."),
                                                  tr("Accept"),Screen::MODE_BLOCK);


        goToHome();
        m_viewer->deleteCurrentBookInfo();
        m_viewer->resetDocView(); //We need reset viewer book path.
        if(!isSleeping())
            dialog->exec();
        delete dialog;

        Screen::getInstance()->resetFlushQueue();
        Screen::getInstance()->refreshScreen(Screen::MODE_SAFE);

    }

}

void QBookApp::handlePartitionCorrupted(StoragePartition *partition)
{
    qDebug() << Q_FUNC_INFO << "device" << partition->getDevice();

    Storage* storage = Storage::getInstance();

    if (partition->getFilesystemType() != StoragePartition::FS_VFAT)
    {
        qDebug() << Q_FUNC_INFO << ": this partition is not VFAT";
        return;
    }

    CorruptedDialog* dialog;
    if (partition->getPartitionType() == StoragePartition::PUBLIC)
        dialog = new CorruptedDialog(this, tr("An error was detected in internal device memory\nWhat do you want to do?"));
    else
        dialog = new CorruptedDialog(this, tr("An error was detected in removable memory\nWhat do you want to do?"));
    dialog->raise();
    Screen::getInstance()->resetQueue();
    dialog->exec();
    int result = dialog->result();
    delete dialog;

    switch(result)
    {
        case CorruptedDialog::ActionFormat: // Format partition
        {
            InfoDialog* infoDialog = new InfoDialog(this,tr("Please, wait until the memory is formatted."));
            infoDialog->showForSpecifiedTime();
            // TODO: "label" of partition has to be a setting
            storage->formatStoragePartition(partition);
            infoDialog->accept();
            delete infoDialog;
        }   break;
        case CorruptedDialog::ActionMount: // mount
            storage->mountStoragePartition(partition);
            break;
        default:
            qDebug() << Q_FUNC_INFO << ": Nothing to do";
            break;

    }
}
void QBookApp::handleUnknownStorage(StorageDevice *device)
{
    qDebug() << Q_FUNC_INFO << device;
    InfoDialog* errorDialog = new InfoDialog(this);
    errorDialog->setTimeLasts(10000);
    errorDialog->setText(tr("This memory is not recognised, please verify it with your computer."));
    errorDialog->hideSpinner();
    errorDialog->showForSpecifiedTime();
    delete errorDialog;
}

#ifndef HACKERS_EDITION
void QBookApp::synchronization(bool lazyDelete)
{
    qDebug() << Q_FUNC_INFO << "Executing synchronization";


    disconnect(this, SIGNAL(syncHelperCreated()), this, SLOT(synchronization()));
    b_lazyDelete = lazyDelete;

    if(isSynchronizing()){
        qDebug() << Q_FUNC_INFO << "synchronization in execution";
        return;
    }

    b_synchronizing = true;

    if(isPoweringOff || !isLinked() || (m_library && m_library->isUnarchivingBook()))
    {
        b_synchronizing = false;
        emit periodicSyncFinished();
        return;
    }

    bool offline = !m_connectManager || !m_connectManager->isConnected();

#ifndef HACKERS_EDITION
    checkUserSubscription();
#endif

    // Library synchronization
    if(!offline)
    {
        stopThumbnailGeneration();
        connect(this, SIGNAL(syncModelFinished()), this, SLOT(launchSync()), Qt::UniqueConnection);
        QtConcurrent::run(this, &QBookApp::syncModel);
    }else
        b_synchronizing = false;
}

void QBookApp::launchSync()
{
    qDebug() << Q_FUNC_INFO;

    bool premium = false;
    QDateTime expiredDate = QDateTime::fromString(QBook::settings().value("subscription/subscriptionEndDate").toString(), Qt::ISODate);
    QDateTime currentTime = QDateTime::currentDateTimeUtc();

    if(expiredDate.isValid() && expiredDate > currentTime)
        premium = true;

    disconnect(this, SIGNAL(syncModelFinished()), this, SLOT(launchSync()));
    m_syncLock->activate();
    getStatusBar()->handleSyncStart();

    if(m_home)
        m_home->initHomeList();

    getSyncHelper()->Start(b_lazyDelete, premium);
}

void QBookApp::synchronizationEnd(int /*status*/, bool dirtyLibrary)
{
    qDebug() << Q_FUNC_INFO << "dirtyLibrary";
    QtConcurrent::run(this, &QBookApp::syncModel);

    b_synchronizing = false;
    getStatusBar()->handleSyncEnd();
    m_pModel->clearTransactions();

    if(dirtyLibrary)
        m_home->setupHome();

    removeExpiredBooks();
    m_library->setReloadModel(dirtyLibrary);
    m_library->reloadModel();
    m_library->resumeThumbnailGeneration();
    QBook::settings().setValue("setting/library/lastSync", QVariant(QDateTime::currentDateTime().toString(Qt::ISODate)));
    emit periodicSyncFinished();
    m_syncLock->release();
}
#endif

void QBookApp::removeExpiredBooks()
{
    qDebug () << Q_FUNC_INFO;

    // Get the local model, and put it inside a QHash
    QList<const BookInfo*> localBookList;
    QBookApp::instance()->getModel()->getSubscriptionExpiredBooks(localBookList, Storage::getInstance()->getPrivatePartition()->getMountPoint());

    QList<const BookInfo*>::iterator itL = localBookList.begin();
    QList<const BookInfo*>::iterator itLE = localBookList.end();
    for(; itL != itLE; ++itL)
    {
        const BookInfo* bookInfo = *itL;
        // NOTE: we check the bookInfo's expiration date instead of book's because it may be newer.
        if(bookInfo->hasExpired())
        {
            qDebug() << Q_FUNC_INFO << "deleting: " << bookInfo->path;
            QFile::remove(bookInfo->path);
            m_pModel->closeBook(bookInfo);
        }
    }
}

void QBookApp::unlinkForced()
{
    qDebug() << Q_FUNC_INFO;

/* FIXME: FER
    if(m_connectManager && m_connectManager->IsConnected())
    {
        m_connectManager->disconnectWifi();
    }
*/
    disconnect(Storage::getInstance(), SIGNAL(partitionMounted(StoragePartition*)), QBookApp::instance(), SLOT(handlePartitionMounted(StoragePartition*)));
    InfoDialog* dialog = new InfoDialog(this,tr("This device is not linked to any account. It would be locally unlink. Please wait for a few minutes."));
    SettingsUnLinkStoreDevice *unlink = new SettingsUnLinkStoreDevice(this);
    QFuture<void> future = QtConcurrent::run(unlink, &SettingsUnLinkStoreDevice::restoreSettinsAndPartitions, dialog);
    if (!future.isFinished())
    {
        Screen::getInstance()->resetQueue();
        dialog->exec();
    }

    qDebug() << Q_FUNC_INFO << "Finished";
    QApplication::quit();
    delete dialog;
}

void QBookApp::checkStartupDebugOpts()
{
#ifndef HACKERS_EDITION
    // Show debugging option dialog
    QString magicFilePath("");
    if(Storage::getInstance()->getRemovablePartition() != NULL){
        magicFilePath = Storage::getInstance()->getRemovablePartition()->getMountPoint();
        magicFilePath.append("/" + QBookDebugDialog::getMagicFileName(MAGIC_FILE));
    }

    if(!magicFilePath.isEmpty() && checkDebugMagicFile(magicFilePath,QBookDebugDialog::getMagicFileName(MAGIC_STRING))){

        if(!m_debugDialog)
            m_debugDialog = new QBookDebugDialog(this);
        m_debugDialog->setup();
        m_debugDialog->exec();
    }

    // Enable debug options silently
    else if(QBookDebugDialog::isAnyFeatureEnabled()){
        if(!m_debugDialog)
            m_debugDialog = new QBookDebugDialog(this);
        m_debugDialog->setup();
        m_debugDialog->init();
    }
#endif
}

void QBookApp::firstChecksAfterStartup()
{
    checkLanguageChanged();
    checkLink();
    checkBooksChanged();

    if(m_connectManager && m_connectManager->isConnected())
    {
        setWifiStatusAsConnected();
    #ifndef HACKERS_EDITION
        if(isLinked() || isActivated())
        {
            checkFwVersion();
            checkNewDictionaries();
            synchronization(true);
            checkTimeSubscriptionToExpire();
            return;
        }
    #endif
    }

    checkTimeSubscriptionToExpire();
    handleUsbStateChanged(ADConverter::getInstance()->getStatus() == ADConverter::ADC_STATUS_PC);
}

void QBookApp::connectDialogSlots()
{
    connect(ADConverter::getInstance(), SIGNAL(chargerStatusChange(bool)), this, SLOT(handleChargerChanged(bool)));
    connect(ADConverter::getInstance(), SIGNAL(pcPresentStatusChange(bool)), this, SLOT(handleUsbStateChanged(bool)), Qt::QueuedConnection);

    connect(Battery::getInstance(), SIGNAL(batteryLevelLow(int)), this, SLOT(handleLowBattery(int)));
    connect(Battery::getInstance(), SIGNAL(batteryLevelCritical()), this, SLOT(handleCriticalBattery()));
    connect(Battery::getInstance(), SIGNAL(batteryLevelFull()), this,SLOT(handleFullBattery()));

}

void QBookApp::syncModel()
{
    qDebug() << Q_FUNC_INFO;

    if(getSyncHelper()->GetServerTimestamp() == 0)
        m_pModel->syncModel(QString(""), m_pModel->getServerTimestamp());
    else
        m_pModel->syncModel(QString(""), getSyncHelper()->GetServerTimestamp());

    emit syncModelFinished();
    qDebug() <<Q_FUNC_INFO << "finished";
}

void QBookApp::showSleep()
{
    qDebug() << Q_FUNC_INFO;

    PowerManagerLock *lock = PowerManager::getNewLock(this);
    lock->activate();

    Screen::getInstance()->resetQueue();
    Screen::getInstance()->queueUpdates();
    m_sleepScreen = new QBookScreenSaver();
    m_sleepScreen->setFixedSize(Screen::getInstance()->screenWidth(), Screen::getInstance()->screenHeight());
    m_sleepScreen->setScreenType(QBookScreenSaver::SLEEP);
#ifdef Q_WS_QWS
    m_sleepScreen->showFullScreen();
#else
    m_sleepScreen->show();
#endif
    connect(PowerManager::getInstance(), SIGNAL(checkWhileSleeping()), m_sleepScreen, SLOT(repaint()));
    qApp->processEvents();
    Screen::getInstance()->refreshScreen(Screen::MODE_SAFE,FLAG_WAITFORCOMPLETION|FLAG_FULLSCREEN_UPDATE);
    Screen::getInstance()->flushUpdates();

    lock->release();
    delete lock;
}

void QBookApp::closeSleep()
{
    qDebug() << Q_FUNC_INFO;

    PowerManagerLock *lock = PowerManager::getNewLock(this);
    lock->activate();

    Screen::getInstance()->queueUpdates();
    Screen::getInstance()->flushColorScreen();
    Screen::getInstance()->setMode(Screen::MODE_QUICK, true, FLAG_WAITFORCOMPLETION, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    Screen::getInstance()->queueUpdates();
    if(m_sleepScreen){
        m_sleepScreen->close();
        delete m_sleepScreen;
        m_sleepScreen = NULL;
    }

    Screen::getInstance()->refreshScreen(Screen::MODE_SAFE,FLAG_WAITFORCOMPLETION|FLAG_FULLSCREEN_UPDATE);
    Screen::getInstance()->flushUpdates();

    if(DeviceInfo::getInstance()->hasFrontLight() && FrontLight::getInstance()->isFrontLightActive())
        FrontLight::getInstance()->switchFrontLight(true);

    lock->release();
    delete lock;
}

void QBookApp::goToSleep()
{
    qDebug() << Q_FUNC_INFO;

    if (b_isSleeping) {
        qDebug() << Q_FUNC_INFO << "we are in the process of sleeping, ignoring request";
        return;
    }

    if(mass_storage) {
        InfoDialog *dialog = new InfoDialog(this, tr("Disconnect USB from CPU before send to sleep the device."), 3000);
        dialog->hideSpinner();
        dialog->showForSpecifiedTime();
        delete dialog;
        return;
    }

    disableUserEvents();

    emit startSleep();

    if(DeviceInfo::getInstance()->hasFrontLight() && FrontLight::getInstance()->isFrontLightActive()){
        int current = FrontLight::getInstance()->getBrightness();
        FrontLight::getInstance()->setBrightness(current);
        FrontLight::getInstance()->switchFrontLight(false);
    }

    Screen::getInstance()->queueUpdates();
    if(m_viewer && getCurrentForm() == m_viewer)
        m_viewer->hideAllElements();

    b_isSleeping = true;
    b_cancelSleep = false;

    disconnect(fwDwld, SIGNAL(checkOTAAvailableFinished(int)), this, SLOT(gotOTACheckResult(int)));
    disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(checkFwVersion()));

    Screen::getInstance()->flushColorScreen();
    Screen::getInstance()->setMode(Screen::MODE_QUICK, true, FLAG_WAITFORCOMPLETION, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    showSleep();
    if(!b_cancelSleep && b_isSleeping)
        PowerManager::getInstance()->goToSleep();
}

void QBookApp::cancelOnGoingSleep()
{
    qDebug() << Q_FUNC_INFO;

    if (b_cancelSleep) {
        qDebug() << Q_FUNC_INFO << "we are in the process of canceling sleeping, ignoring request";
        return;
    }

    b_cancelSleep = true;
    b_isSleeping = false;

    closeSleep();

    enableUserEvents();

    connect(fwDwld, SIGNAL(checkOTAAvailableFinished(int)), this, SLOT(gotOTACheckResult(int)), Qt::UniqueConnection);
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(checkFwVersion()), Qt::UniqueConnection);

    if(m_viewer && m_viewer == getCurrentForm())
    {
        m_viewer->updateTimeOnPage();
        m_viewer->calculateTimeForStep();
    }

    PowerManager::getInstance()->cancelSleep();
    emit cancelSleep();
}

void QBookApp::showDeviceInfoDialog()
{
        deviceInfoDialog = new DeviceInfoDialog(this);
        deviceInfoDialog->setMinimumSize(Screen::getInstance()->screenWidth(), Screen::getInstance()->screenHeight());
        deviceInfoDialog->setFixedSize(Screen::getInstance()->screenWidth(), Screen::getInstance()->screenHeight());
        deviceInfoDialog->show();

}

void QBookApp::prepareToSleep()
{
    qDebug() << Q_FUNC_INFO;

    if(m_syncDialog){
        delete m_syncDialog;
        m_syncDialog = NULL;
    }

    if(deviceInfoDialog){
        delete deviceInfoDialog;
        deviceInfoDialog = NULL;
    }

    bool viewerBookInList = false;
    bool booksDeleted = false;
    QString viewerBookPath = "";

    if(m_viewer && m_viewer->getCurrentBookInfo())
        viewerBookPath = m_viewer->getCurrentBookInfo()->path;

    DeleteLaterFile::exec(Storage::getInstance()->getPrivatePartition()->getMountPoint(), getModel(), viewerBookPath, viewerBookInList, booksDeleted);

    if(m_viewer){
        if(viewerBookInList){
            m_viewer->resetDocView();

            if(((m_viewer && m_viewer->isVisible()) || (m_home && m_home->isVisible())))
                goToHome();
        }else{
            if(m_viewer == getCurrentForm()){
                m_viewer->prepareViewerBeforeSleep();
                m_viewer->setTimestamp(QDateTime::currentDateTime().toMSecsSinceEpoch());
                m_viewer->updateBookInfo();
            }
        }
    }

#ifndef HACKERS_EDITION
    if (!QBook::settings().value("wifi/disabled",false).toBool() && isLinked())
    {
        // If wifi is enabled and device is linked we should try to sync before sleep
        bool connected = false;

        if (m_viewer && getCurrentForm() == m_viewer) {
            qDebug() << Q_FUNC_INFO << "We are on the viewer.";

            if (ConnectionManager::getInstance()->isConnected()) {
                connected = true;
                qDebug() << Q_FUNC_INFO << "we are connected (maybe we are in emulator)";

                if (!isSynchronizing()) {
                    qDebug() << Q_FUNC_INFO << "Launching sync";
                    synchronization();
                } else {
                    qDebug() << Q_FUNC_INFO << "There was a sync in progress not launching a new one";
                }
            } else {
                qDebug() << Q_FUNC_INFO << "so we need to turn wifi on to do a sync before sleep";
                resumeWifi();
                // on connected we will launch the sync
                connected = waitForConnection();
            }
        } else {
            qDebug() << Q_FUNC_INFO << "We are not in viewer";

            if (ConnectionManager::getInstance()->isConnected()) {
                qDebug() << Q_FUNC_INFO << "We are connected";
                connected = true;

                if (!isSynchronizing()) {
                    qDebug() << Q_FUNC_INFO << "Launching sync";
                    synchronization();
                } else {
                    qDebug() << Q_FUNC_INFO << "There was a sync in progress not launching a new one";
                }
            } else {
                qDebug() << Q_FUNC_INFO << "We are not connected";

                if (!ConnectionManager::getInstance()->getConnectingNetwork().isEmpty()) {
                    // if the user came from the viewer we are still resuming wifi, so let's wait
                    qDebug() << Q_FUNC_INFO << "Looks like we are connecting, waiting for connection";
                    // on connected we will launch the sync
                    connected = waitForConnection();
                }
            }
        }

        if (b_cancelSleep) {
            qDebug() << "Sleep cancelled while waiting for connection";
            if (m_viewer && getCurrentForm() == m_viewer) {
                qDebug() << "As we are in the viewer we need to powerOff wifi again";
                powerOffWifiAsync();
            }
            b_cancelSleep = false;
            b_isSleeping = false;
            return;
        }

        if (connected)
            waitForSyncFinished();

        if (b_cancelSleep) {
            qDebug() << "Sleep cancelled while waiting for sync";
            if (m_viewer && getCurrentForm() == m_viewer) {
                qDebug() << "As we are in the viewer we need to powerOff wifi again";
                powerOffWifiAsync();
            }
            b_cancelSleep = false;
            b_isSleeping = false;
            return;
        }

        // Disconnect Wifi
        powerOffWifi();
        if (b_cancelSleep) {
            qDebug() << "Sleep cancelled while powering off wifi.";
            if (m_viewer && getCurrentForm() != m_viewer) {
                qDebug() << "As we are not in the viewer we need to resume wifi";
                resumeWifi();
            } else {
                qDebug() << "As we are in the viewer we do not need to resume wifi";
            }
            b_cancelSleep = false;
            b_isSleeping = false;
            return;
        }
    } else 
#endif
    if (!QBook::settings().value("wifi/disabled",false).toBool()){
        qDebug() << Q_FUNC_INFO << "Wifi is enabeld but device is not linked. Not trying to sync, just powering off wifi before sleep";
        powerOffWifi();
        QtConcurrent::run(this, &QBookApp::syncModel);
    } else {
        QtConcurrent::run(this, &QBookApp::syncModel);
        qDebug() << Q_FUNC_INFO << "Wifi is disabled, so do not launch a sync before sleep";
    }
    b_cancelSleep = false;
    b_isSleeping = false;
    qDebug() << Q_FUNC_INFO << "Prepare to sleep is done, leting PowerManager to sleep us";
}

bool QBookApp::waitForConnection()
{
    qDebug() << Q_FUNC_INFO;

    bool connected;
    QTimer timer;
    QEventLoop loop;
    loop.connect(ConnectionManager::getInstance(), SIGNAL(connected()), SLOT(quit()));
    loop.connect(this, SIGNAL(cancelSleep()), SLOT(quit()));
    loop.connect(&timer, SIGNAL(timeout()), SLOT(quit()));
    timer.start(TIME_TO_AUTOCONNECT_LOCK * 1000);
    if (ConnectionManager::getInstance()->isConnected()) {
         qDebug() << "Looks like we are already connected, not waiting";
         return true;
    }
    loop.exec();
    connected = ConnectionManager::getInstance()->isConnected();
    qDebug() << Q_FUNC_INFO << "Finished waiting for connection. Connected: " << connected << " cancelSleep" << b_cancelSleep;
    return connected;
}

void QBookApp::waitForSyncFinished()
{
    qDebug() << Q_FUNC_INFO;

    QEventLoop loop;
    loop.connect(this, SIGNAL(periodicSyncFinished()), SLOT(quit()));
    loop.connect(this, SIGNAL(cancelSleep()), SLOT(quit()));

    if (!isSynchronizing()) {
         qDebug() << Q_FUNC_INFO << "Synchronization not running, maybe it alredy finished. Not waiting";
         return;
    }
    loop.exec();
    if (!b_cancelSleep)
        qDebug() << Q_FUNC_INFO << "sync finished";
    else
        qDebug() << Q_FUNC_INFO << "sync not finished but received cancelSleep, so not waiting anymore";
}

void QBookApp::resumeAfterSleep()
{
    qDebug() << Q_FUNC_INFO;

    b_cancelSleep = false;
    b_isSleeping = false;

    if(isLedDebuggingOff())
        Power::getInstance()->setLed(false);

    QTimer::singleShot(500, this, SLOT(enableUserEvents()));
    QTimer::singleShot(2000, this, SLOT(enablePowerKey()));

    QBookForm* current_form = getCurrentForm();

#ifndef HACKERS_EDITION
    if(current_form == m_webWizard && isLinked())
    {
        closeWizard();
    }
#endif

    if(m_viewer)
    {
        m_viewer->calculateTimeForStep();
        m_viewer->updateTimeOnPage();
    }
    if (!QBook::settings().value("wifi/disabled",false).toBool()) {
        if (m_viewer != current_form) {
            // If we are not in the viewer we always restore wifi. Sync may happen if device is linked..
            qDebug() << Q_FUNC_INFO << "enabling wifi after sleep (we are not in the viewer)";
            resumeWifi();
        }
        #ifndef HACKERS_EDITION
        else if (isLinked())
        {
            // If we are in the viewer and we are linked we ays restore wifi fo force sync...
            qDebug() << Q_FUNC_INFO << "enabling wifi after sleep (we are in the viewer and linked)";
            resumeWifi();
            // we should poweoff wifi after sync is finished or not getting connection
            m_connectAfterSleepTimer.setSingleShot(true);
            // If we don't get connected after 25 secs, poweroff the wifi
            connect(&m_connectAfterSleepTimer, SIGNAL(timeout()), this, SLOT(powerOffWifiAsync()));
            m_connectAfterSleepTimer.start(TIME_TO_AUTOCONNECT_LOCK * 1000);
            // If we get connected we cancel the timer and power off after sync has been finished
            connect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(connectedAfterSleepForSync()), Qt::UniqueConnection);
        }
        #endif
    }
    else
    {
        removeExpiredBooks();
        checkTimeSubscriptionToExpire();
    }

    if(current_form == m_home)
        m_home->setupHome();

    connect(fwDwld, SIGNAL(checkOTAAvailableFinished(int)), this, SLOT(gotOTACheckResult(int)), Qt::UniqueConnection);
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(checkFwVersion()), Qt::UniqueConnection);
    checkLink();
    QTimer::singleShot(500, this, SLOT(checkTimeSubscriptionToExpire()));
}

void QBookApp::enablePowerKey()
{
        b_keyBlocked = false;
}

void QBookApp::disablePowerKey()
{
        b_keyBlocked = true;
}

void QBookApp::connectedAfterSleepForSync()
{
     qDebug() << Q_FUNC_INFO;
    disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(connectedAfterSleepForSync()));
    // We got connected, cancel the poweroff timer
    m_connectAfterSleepTimer.stop();
    if(isSynchronizing())
        connect(this, SIGNAL(periodicSyncFinished()), this, SLOT(powerOffWifiAsync()), Qt::UniqueConnection);
    else
        powerOffWifiAsync();
}

void QBookApp::powerOffWifi()
{
    qDebug() << Q_FUNC_INFO;
    ConnectionManager::getInstance()->setOffline();
    ConnectionManager::getInstance()->powerWifi(false);
    Wifi::getInstance()->powerOff();
}

void QBookApp::powerOffWifiAsync()
{
    qDebug() << Q_FUNC_INFO;
    if (m_viewer && getCurrentForm() != m_viewer) {
        qDebug() << Q_FUNC_INFO << "We are not anymore in the viewer, so we should not turn wifi off!";
        return;
    }

    isPoweringOff = true;
    removeExpiredBooks();
    checkTimeSubscriptionToExpire();
    disconnect(this, SIGNAL(periodicSyncFinished()), this, SLOT(powerOffWifiAsync()));
    offlineHelper = new QProcess(this);
    QStringList args;
#ifdef Q_WS_QWS
    args << "-qws";
#endif
    qDebug() << Q_FUNC_INFO << "Launching Offline helper with args: " << args;
    offlineHelper->start(OFFLINE_HELPER, args);
    getStatusBar()->hideWifiCont();
}

void QBookApp::lockAutoconnectOnWifiPower(bool lock)
{
        qDebug() << Q_FUNC_INFO;
        if (lock){
            if(m_autoconnect_lock)
                m_autoconnect_lock->activate();

            m_afterWifiOnTimer.setSingleShot(true);

            // One minute waiting before connman try to connect in its own way
            m_afterWifiOnTimer.start(60000);

        }else{
            if(m_autoconnect_lock)
                m_autoconnect_lock->release();
            m_afterWifiOnTimer.stop();
        }
}

void QBookApp::resumeWifi()
{
    qDebug() << Q_FUNC_INFO;

    if (offlineHelper != NULL) {
        // Ensure the offline process has finished
       if (offlineHelper->state() != QProcess::NotRunning)
            offlineHelper->waitForFinished();

       delete offlineHelper;
       offlineHelper = NULL;
    }

    if (ConnectionManager::getInstance()->isConnected()) {
#ifndef HACKERS_EDITION
    	qDebug() << Q_FUNC_INFO << "resumeWifi but we are already connected. Probably in the emulator with fake wifi. We do nothing and force sync";
        synchronization();
#endif
        return;
    }
    
    startedResumingWifi();
     Wifi::getInstance()->powerOn();
     isPoweringOff = false;
     ConnectionManager::getInstance()->powerWifi(true);

    ConnectionManager::getInstance()->setOnline(false);
    if(m_autoconnect_lock)
        m_autoconnect_lock->activate();
}

void QBookApp::enableUserEvents(){
    qDebug() << Q_FUNC_INFO;

    b_userEventsBlocked = false;
}

void QBookApp::disableUserEvents(){
    qDebug() << Q_FUNC_INFO;

    b_userEventsBlocked = true;
}

void QBookApp::showRestoringImage()
{
    qDebug() << "--->" << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();
    QString lang = QBook::settings().value("setting/language", QVariant("es")).toString();
    QString imageUrl = QString(":/restoring_screen_%1.png").arg(lang);

    QImage restoringImage(imageUrl);
    if (restoringImage.isNull())
        restoringImage.load(":/restoring_screen_en.png");
    Screen::getInstance()->flushImage(restoringImage,QPoint(0,0));
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE|FLAG_WAITFORCOMPLETION,Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void QBookApp::releaseAutoconnectPowerLock()
{
    if(m_autoconnect_lock)
        m_autoconnect_lock->release();
}

void QBookApp::activateConnectedPowerLock()
{
    m_connected_lock->activate();
    m_wifiSleepTimer.setSingleShot(true);
    restartWifiSleepTimer();
    connect(this, SIGNAL(userEvent()),this, SLOT(restartWifiSleepTimer()));

}

void QBookApp::restartWifiSleepTimer(){

    int interval = QBook::settings().value("setting/sleepTimeInSecs", POWERMANAGER_TIME_AUTOSLEEPSECS).toInt() * 1000;
    m_wifiSleepTimer.start(interval);
}

void QBookApp::releaseConnectedPowerLock()
{
    m_connected_lock->release();
    m_wifiSleepTimer.stop();
    disconnect(this, SIGNAL(userEvent()),this,SLOT(restartWifiSleepTimer()));
}

void QBookApp::checkConnectedPowerLock()
{
    /* Check if m_connected_lock is the only lock present */ 
    if (PowerManager::getInstance()->getCurrentLocks(true).size() == 1 &&
        PowerManager::getInstance()->getCurrentLocks(true).first() == m_connected_lock) {
        qDebug() << Q_FUNC_INFO << "m_wifiSleepTimer triggered, only m_connected_lock is present, forcing sleep";
#ifndef SHOWCASE // Do not auto-sleep if exposed in closed showcase
        goToSleep();
#endif
    } else {
        qDebug() << Q_FUNC_INFO << "There are other powerLocks that are not m_connected_lock, restarting timer";
	restartWifiSleepTimer();
    }
}

void QBookApp::emitSwipe(int direction){
    qDebug() << "--->" << Q_FUNC_INFO << direction;

    emit swipe(direction);
}

#ifndef HACKERS_EDITION
bool QBookApp::isLinked()
{
    return QBook::settings().value("setting/linked", false).toBool();
}

bool QBookApp::isActivated()
{
    return QBook::settings().value("setting/activated", false).toBool();
}
#endif

void QBookApp::dbusServicesChanged ( const QString & name, const QString & oldOwner, const QString & newOwner )
{
    qDebug() << Q_FUNC_INFO << name << oldOwner << newOwner;
    if (name == "com.bqreaders.SyncHelperService") {
        if (!oldOwner.isEmpty() && newOwner.isEmpty()) {
            qDebug() << Q_FUNC_INFO << "sync helper died. restarting com.bqreaders.SyncHelperService";
	    QDBusReply<void> reply;
            reply = QDBusConnection::systemBus().interface()->startService("com.bqreaders.SyncHelperService");
      	    if (!reply.isValid()) {
		    qDebug() << "Starting com.bqreaders.SyncHelperService failed:" << reply.error().message();  
	    } else {
		    qDebug() << "Starting com.bqreaders.SyncHelperService ok:";
	    }
        } else if (oldOwner.isEmpty() && !newOwner.isEmpty()) {
            qDebug() << Q_FUNC_INFO << "sync helper restarted. doing init";
            initSyncHelper();
        }
    } else if (name == "net.connman") {
        if (!oldOwner.isEmpty() && newOwner.isEmpty()) {
            qDebug() << "\nWARNING: connmand died!\n";
        } else if (oldOwner.isEmpty() && !newOwner.isEmpty()) {
            qDebug() << "\nconnmand is up again notifing to get Agent re-registered!\n";
            emit connmandRestarted();
        }
   }
}

bool QBookApp::isViewerCurrentForm()
{
    return m_viewer && m_viewer == getCurrentForm();
}

bool QBookApp::isSynchronizing()
{
       SyncHelper *syncHelper = getSyncHelper();
       return syncHelper && syncHelper->isValid() && syncHelper->IsSynchronizing() || b_synchronizing;
}

void QBookApp::viewRecentBooks()
{
    Screen::getInstance()->queueUpdates();
    m_library->viewActiveBooks();
    goToLibrary();
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE,true);
    Screen::getInstance()->flushUpdates();
}

void QBookApp::cancelSync()
{
    qDebug() << Q_FUNC_INFO;

    if(isSynchronizing()){
        b_synchronizing = false;
        m_sync->Cancel();
    }
}

#ifndef HACKERS_EDITION

bool QBookApp::checkSDForHackers()
{
    qDebug() << Q_FUNC_INFO;

    QString updateCheckFile;
    if(DeviceInfo::getInstance()->getHwId() == DeviceInfo::E60Q22) //Q22
        updateCheckFile = HACKERS_UPDATE_CHECK_FILE_Q22;
    else // 672 and A22
        updateCheckFile = HACKERS_UPDATE_CHECK_FILE_672;

    QFile authFile(Storage::getInstance()->getPrivatePartition()->getMountPoint() + updateCheckFile);
    if(authFile.exists())
        return false;
    StoragePartition *sd = Storage::getInstance()->getRemovablePartition();
    if (!sd)
        return false;
    if (!sd->isMounted())
        return false;
    QFile file(sd->getMountPoint() + "/I_WANT_TO_BE_A_HACKER");
    if (!file.exists())
        return false;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
         return false;
    QByteArray line = file.readLine().trimmed();
    if (line != DeviceInfo::getInstance()->getSerialNumber())
         return false;

    qDebug() << Q_FUNC_INFO << "file I_WANT_TO_BE_A_HACKER present on external SD and has proper serial number";
    return true;
}


bool QBookApp::askForHackersInstallation()
{
    SelectionDialog* dialogSelect = new SelectionDialog(this,tr("Are you sure you want to enable Hackers firmware installation?"));
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
    Screen::getInstance()->resetQueue();
    dialogSelect->exec();

    if(!dialogSelect->result()){
        delete dialogSelect;
        return false;
    }

    // Ask twice :)
    dialogSelect = new SelectionDialog(this,tr("Enabling Hackers firmware installation will void your warranty. Are you really sure you want to continue?"));
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
    Screen::getInstance()->resetQueue();
    dialogSelect->exec();

    if(!dialogSelect->result()){
        delete dialogSelect;
        return false;
    }

    delete dialogSelect;
    return true;
}

void QBookApp::allowHackersInstallation()
{
    qDebug() << Q_FUNC_INFO;
    if (m_connectManager->isConnected()) {
        requestHackersInstallation();
    } else {
        int connectionRes = requestConnection(true);

        if (connectionRes == NOT_CONNECTED) {
            ConfirmDialog *connectionDialog = new ConfirmDialog(this);
            connectionDialog->setText(tr("Notification over the network is required to allow Hackers firmware. Aborting"));
            Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
            connectionDialog->exec();
            delete connectionDialog;
            return;
        }
       
        connect(m_connectManager, SIGNAL(connected()), this, SLOT(requestHackersInstallation()));
    }
}

QString QBookApp::getHackersAuth()
{
    static const char *salt = "ErrbansZme`VW^Wc";
    // Do some stupid obfuscation. Resulting string should be "Esteesyaunjacker"
    char *tmp = strdup(salt);
    char *tmp2 = tmp;
    int i;
    for (i=0; i< strlen(salt); i++) {
        tmp2[i] = (char) ((int)tmp2[i] + i);
    }
    QString qs_snum = DeviceInfo::getInstance()->getSerialNumber();
    QByteArray hashData;
    hashData.append(qs_snum);
    hashData.append(tmp);
    QByteArray hash = QCryptographicHash::hash(hashData,QCryptographicHash::Sha1).toHex();
    free(tmp);
    return hash;
}

bool QBookApp::deviceHackAuthorized(QString auth)
{
        if(auth == getHackersAuth())
            return true;
        else if(auth.compare("DeviceHackNotAuthorized") == 0)
            return false;
}

void QBookApp::requestHackersInstallation()
{
    qDebug() << Q_FUNC_INFO;
    disconnect(m_connectManager, SIGNAL(connected()), this, SLOT(requestHackersInstallation()));

    QString updateCheckFile;
    if(DeviceInfo::getInstance()->getHwId() == DeviceInfo::E60Q22) //Q22
        updateCheckFile = HACKERS_UPDATE_CHECK_FILE_Q22;
    else // 672 and A22
        updateCheckFile = HACKERS_UPDATE_CHECK_FILE_672;

    QString auth = m_pServices->hackersInstallation();
    if (auth.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "Error with hackers installation auth: " << auth;
        ConfirmDialog *erroDialog = new ConfirmDialog(this);
        erroDialog->setText(tr("Error obtaining auth from bq servers for firmware installation. Aborting"));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        erroDialog->exec();
        delete erroDialog;
        return;
    }
    else if (!deviceHackAuthorized(auth)) {
        qDebug() << Q_FUNC_INFO << "Error with hackers installation auth: " << auth;
        ConfirmDialog *erroDialog = new ConfirmDialog(this);
        erroDialog->setText(tr("Device not authorized for developers firmware installation, please contact customer support to get authorization"));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        erroDialog->exec();
        delete erroDialog;
        return;
    }
    QFile authFile(Storage::getInstance()->getPrivatePartition()->getMountPoint() + updateCheckFile);
    authFile.open(QIODevice::WriteOnly | QIODevice::Text);
    authFile.write("ok");
    authFile.close();
    ConfirmDialog *okDialog = new ConfirmDialog(this);
    okDialog->setText(tr("Hackers firmware installation enabled. Download firmware, place it on SD card and power on the device while holding home button pressed."));
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
    okDialog->exec();
    delete okDialog;

}
#endif
    
void QBookApp::hackersInit()
{
#ifdef HACKERS_EDITION
    qDebug() << Q_FUNC_INFO;
    if (QBook::settings().value("setting/hackersUsbNetwork").toBool() && !QBookDevel::isUsbNetworkEnabled())
        QBookDevel::enableUsbNetwork(true);

    if (!QBook::settings().value("setting/hackersUsbNetwork").toBool() && QBookDevel::isUsbNetworkEnabled())
        QBookDevel::enableUsbNetwork(false);

    if (QBook::settings().value("setting/hackersTelnet").toBool() && !QBookDevel::isTelnetEnabled())
        QBookDevel::enableTelnet();

    if (!QBook::settings().value("setting/hackersTelnet").toBool() && QBookDevel::isTelnetEnabled())
        QBookDevel::disableTelnet();
#endif
}

#ifndef HACKERS_EDITION
void QBookApp::syncOnlyViewerBook()
{
    qDebug() << Q_FUNC_INFO;
    BookInfo *bookInfo = new BookInfo(*m_viewer->getCurrentBookInfo());
    getDeviceServices()->syncOnlyOneBook(bookInfo);
    getModel()->updateBook(bookInfo);
}

#endif

void QBookApp::prepareViewerToSync()
{
    qDebug() << Q_FUNC_INFO;
    bool viewerBookInList = false;
    bool booksDeleted = false;

    QString viewerBookPath = "";
    if(m_viewer && m_viewer->getCurrentBookInfo())
        viewerBookPath = m_viewer->getCurrentBookInfo()->path;

    DeleteLaterFile::exec(Storage::getInstance()->getPrivatePartition()->getMountPoint(), getModel(), viewerBookPath, viewerBookInList, booksDeleted);

    if(m_viewer && m_viewer->getCurrentBookInfo() && !viewerBookInList)
    {
        if(!viewerBookInList)
        {
            m_viewer->prepareViewerBeforeSleep();
            m_viewer->setTimestamp(QDateTime::currentDateTime().toMSecsSinceEpoch());
            m_viewer->updateBookInfo();
        }

        #ifndef HACKERS_EDITION
        if(m_viewer->getCurrentBookInfo()->path.contains(Storage::getInstance()->getPrivatePartition()->getMountPoint()))
        {
            if (!QBook::settings().value("wifi/disabled",false).toBool() && isLinked())
            {
                bool connected = false;
                qDebug() << Q_FUNC_INFO << "We are on the viewer.";
                if (ConnectionManager::getInstance()->isConnected())
                {
                    connected = true;
                    if (!isSynchronizing())
                    {
                        qDebug() << Q_FUNC_INFO << "Launching sync";
                            syncOnlyViewerBook();
                    }
                }else{
                    qDebug() << Q_FUNC_INFO << "so we need to turn wifi on to do a sync before sleep";
                    disconnect(fwDwld, SIGNAL(checkOTAAvailableFinished(int)),        this, SLOT(gotOTACheckResult(int)));
                    disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(checkFwVersion()));
                    disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(synchronization()));

                    resumeWifi();
                    connected = waitForConnection();
                        if(connected)
                            syncOnlyViewerBook();
                }
            }
        }
        #endif
    }

    syncModel();
}

void QBookApp::showLoadingBooksDialog(int loadingBooksCount)
{
    qDebug() << Q_FUNC_INFO;
    disconnect(m_pModel,  SIGNAL(loadingBooks(int)), this, SLOT(showLoadingBooksDialog(int)));

    if(readingFilesdialog)
    {
        delete readingFilesdialog;
        readingFilesdialog = NULL;
    }

    QString text = QString(tr("Nuevos libros encontrados: ") + QString::number(loadingBooksCount));
    StoragePartition* partition = Storage::getInstance()->getRemovablePartition();
    qDebug() << Q_FUNC_INFO << "SD mounted: " << (partition && (partition->isMounted()));
    if(partition && (partition->isMounted()))
        text.append(tr("\nPor favor no extraigas la SD."));
    if(!loadingBooksDialog)
    {
        loadingBooksDialog = new ProgressDialog(this, text);
        loadingBooksDialog->hideCancelButton();
        loadingBooksDialog->setHideBtn(false);
        loadingBooksDialog->setTextValue(false);
        loadingBooksDialog->setModal(true);
        connect(m_pModel, SIGNAL(downloadProgress(int)), loadingBooksDialog, SLOT(setProgressBar(int)), Qt::UniqueConnection);
    }
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    loadingBooksDialog->show();
    QCoreApplication::flush();
    QCoreApplication::processEvents();
}

void QBookApp::hideLoadingBooksDialog()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    disconnect(m_pModel, SIGNAL(loadFinished()), this, SLOT(hideLoadingBooksDialog()));
    if(loadingBooksDialog)
    {
        loadingBooksDialog->hide();
        delete loadingBooksDialog;
        loadingBooksDialog = NULL;
    }
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void QBookApp::takeScreenShot(){
    qDebug() << Q_FUNC_INFO;

    // Take screenshot
    QPixmap screenshot = QPixmap::grabWidget(this);

    // Save to file
    QString folderName = "Screenshots";
    QDir storageDir(Storage::getInstance()->getPublicPartition()->getMountPoint());
    storageDir.mkdir(folderName);
    QString fileName = QBOOKAPP_VERSION + QDateTime::currentDateTime().toString("_MMdd_hhmmss");
    QString extension = "jpg";

    QString filePath = storageDir.path() + QDir::separator() + folderName
            + QDir::separator() + fileName + "." + extension;
    qDebug() << Q_FUNC_INFO << "Saving" << filePath;
    screenshot.save(filePath,extension.toAscii());

    // Showing dialog
    InfoDialog *dialog = new InfoDialog(this, tr("Screenshot saved to ") + folderName + QDir::separator() + fileName + "." + extension, 5000);
    dialog->showForSpecifiedTime();
    if(getCurrentForm() == m_library && m_library->getFilterMode() == Library::ELFM_INTERNAL)
        m_library->setupLibrary();
    delete dialog;

    if(isLedDebuggingOff())
        Power::getInstance()->setLed(false);
}

void QBookApp::wifiProblem()
{
    qDebug() << Q_FUNC_INFO;
    QString message = tr("Ha ocurrido un problema con la wifi.\nPor favor, reinicia el dispositivo si necesitas seguir usandola.");
    ConfirmDialog* wifiProblemDialog = new ConfirmDialog(this, message, tr("Accept"),Screen::MODE_BLOCK);
    wifiProblemDialog->exec();
    delete wifiProblemDialog;
    Screen::getInstance()->refreshScreen(Screen::MODE_SAFE,true);
}

BookInfo* QBookApp::openDocWithoutActivateForm(const BookInfo* bookInfo)
{
    qDebug() << Q_FUNC_INFO;
    QBookApp::instance()->getViewer()->openDoc(bookInfo);
    QBookApp::instance()->getViewer()->calculateLocationsChapter();
    getStatusBar()->show();//viewer->openDoc hides status bar so we need queue screen before openDoc and flush after show status bar.
    BookInfo* book = new BookInfo(*QBookApp::instance()->getViewer()->getCurrentBookInfo());
    QBookApp::instance()->getViewer()->deleteCurrentBookInfo();
    return book;
}

void QBookApp::enablePowerKeyWatcher(bool active)
{
    qDebug() << Q_FUNC_INFO;

    if(active)
        m_powerKeyWatcher.start();
    else
        m_powerKeyWatcher.stop();
}

void QBookApp::checkLongPressPowerOff(){

    // Check power key
    if(Power::getInstance()->isPowerKeyPressed()){
        // Update led
        if((i_powerKeySamplesCounter == 0)&& isLedDebuggingOff())
            Power::getInstance()->setLed(true);

        i_powerKeySamplesCounter++;
        qDebug() << Q_FUNC_INFO << "PowerKey pressed," << i_powerKeySamplesCounter << "samples";
        if(i_powerKeySamplesCounter > POWERMANAGER_PWROFF_LONGPRESS/POWER_KEY_WATCHER_PERIOD){
            qDebug() << Q_FUNC_INFO << "POWERKEY LONG PRESSED";
            emit powerKeyLongPressed();
        }
    }
    else
    {
        // Set led
        if((i_powerKeySamplesCounter != 0) && isLedDebuggingOff())
            Power::getInstance()->setLed(false);

        // Reset counter
        i_powerKeySamplesCounter = 0;
    }
}

#ifndef HACKERS_EDITION
void QBookApp::resetServiceSettings()
{
    qDebug() << Q_FUNC_INFO;
    if(m_webWizard)
        m_webWizard->setWebPageEmail("");

    QBook::settings().setValue("shopName", "Tienda");
    QBook::settings().setValue("readerPartitionName", "reader");
    QBook::settings().setValue("deviceModelName", "reader");
    QBook::settings().setValue("wsServicesURL", "");

    QBook::settings().setValue("shopIcon", "");
    QBook::settings().setValue("offDeviceImageUrl", "");
    QBook::settings().setValue("restDeviceImageUrl", "");
    QBook::settings().setValue("startDeviceImageUrl", "");
    QBook::settings().setValue("updateDeviceImageUrl", "");
    QBook::settings().setValue("lowBatteryDeviceImageUrl", "");
    QBook::settings().setValue("dictionaryToken", "");
    QBook::settings().setValue("shopUrl","");
    QBook::settings().setValue("serviceURLs/book", "");
    QBook::settings().setValue("serviceURLs/search", "");

    QBook::settings().setValue("eMail", "");
    QBook::settings().setValue("name", "");
    QBook::settings().setValue("surname", "");
    QBook::settings().setValue("shopUserAccountUrl", "");
}
#endif

void QBookApp::checkScreensaverCoverAfterSync(){
    qDebug() << Q_FUNC_INFO;

    // Do not update cover if viewer opened
    if(m_viewer != getCurrentForm())
        checkScreensaverCover();
}

void QBookApp::checkScreensaverCover(){
    qDebug() << Q_FUNC_INFO;

    // Get most recent book
    QList<const BookInfo*> recentBooks;
    getModel()->getNowReadingBooks(recentBooks, "");
    if(!recentBooks.size()) // Empty list
    {
        QBook::settings().setValue("setting/screensaverCoverToShow","");
        return;
    }
    qSort(recentBooks.begin(), recentBooks.end(), readingTimeAfterThan);

    setScreensaverCover(recentBooks.at(0));

}


void QBookApp::setScreensaverCover(const BookInfo* bookInfo)
{
    qDebug() << Q_FUNC_INFO << "showing:" << bookInfo->path;

    QString newImagePath("");
    QString currentImagePath = QBook::settings().value("setting/screensaverCoverToShow", "").toString();
    QFileInfo currentImageFile(currentImagePath);

    QString fileName = bookInfo->path + ".cover";
    fileName.replace(QDir::separator(),"_");
    newImagePath = Storage::getInstance()->getPrivatePartition()->getMountPoint() + QDir::separator() + fileName;

    // Exit if no change
    if(fileName == currentImageFile.completeBaseName() || bookInfo->path.contains("/app/share/userGuides/"))
        return;

    // Try to extract from epub and render 1st page if not possible    
    bool gotCover = MetaDataExtractor::extractCover(bookInfo->path, newImagePath);
    if (!gotCover) newImagePath = QDocView::coverPage(bookInfo->path, newImagePath); // Render 1st page if not have cover.

    // Reset if image not created
    QImageReader image(newImagePath);
    QFile imageFile(newImagePath);
    if(image.canRead())
    {
        newImagePath += "." + image.format();
        imageFile.rename(newImagePath);
    }
    else
    {
        qDebug() << Q_FUNC_INFO << "Error extracting cover";
        newImagePath = "";
    }

    // Remove previous
    QFile oldImage(currentImagePath);
    if(oldImage.exists()) // remove previous
        oldImage.remove();

    QBook::settings().setValue("setting/screensaverCoverToShow", newImagePath);
}

void QBookApp::stopThumbnailGeneration()
{
    qDebug() << Q_FUNC_INFO;
    m_library->stopThumbnailGeneration();
}

void QBookApp::resumeThumbnailGeneration()
{
    qDebug() << Q_FUNC_INFO;
    m_library->resumeThumbnailGeneration();
}

QSize QBookApp::getHomeThumbnailSize(){
    qDebug() << Q_FUNC_INFO;

    return m_home->getHomeThumbnailSize();
}

#ifndef HACKERS_EDITION
void QBookApp::checkOTAFile()
{
    qDebug() << Q_FUNC_INFO;
    if (QFile::exists("/OTA_OK")) {
        qDebug() << Q_FUNC_INFO << "First start after an OTA update.";
        QFile::remove("/OTA_OK");

        //Workaround to reboot device after OTA to take the full kernel
        showRestoringImage();
        QCoreApplication::flush();
        qApp->processEvents();
        system("reboot");
    }

    if (QFile::exists("/OTA_KO")) {
        qDebug() <<Q_FUNC_INFO << "OTA Install was wrong!";
        //FIXME: Should we pop up a dialog for the user?
        QFile::remove("/OTA_KO");
    }
}

void QBookApp::checkNewDictionaries()
{
    qDebug() << Q_FUNC_INFO;

    QString dixioToken = "";
    if(!availableDictsList.size())
        availableDictsList = Dictionary::instance()->parseAvailableDictionaries(QString::fromUtf8(Dictionary::instance()->getJsonAvailableDicts()), dixioToken).values();

    renewDictionaryToken(dixioToken);

    if(QBook::settings().value("setting/dictionaryAvailable", false).toBool())
        return;

    QList<DictionaryParams> dictionaryList = Dictionary::instance()->parseCurrentDictionaries(QString::fromUtf8(Dictionary::instance()->getJsonDictionaries())).values();
    QList<DictionaryParams> pendingDictionaryList;
    bool newDictionaryFound = false;

    QList<DictionaryParams>::iterator it = availableDictsList.begin();
    QList<DictionaryParams>::iterator itEnd = availableDictsList.end();
    while(it != itEnd)
    {
        qDebug() << Q_FUNC_INFO << "id: " << (*it).id << " commercialName: " << (*it).commercialName;

        newDictionaryFound = true;
        for (int i = 0; i < dictionaryList.size(); i++){
            if(dictionaryList[i].id == (*it).id){
                newDictionaryFound = false;
                break;
            }
        }
        (*it).activationState = false;

        if(newDictionaryFound)
            pendingDictionaryList.append(*it);

        ++it;
    }

    if(pendingDictionaryList.size() > 0)
    {
        QBook::settings().setValue("setting/dictionaryAvailable", true);
        QBook::settings().sync();

        SelectionDialog* confirmDialog = new SelectionDialog(this,tr("New dictionaries available. Do you want to proceed with installation?"));
        connect(this,SIGNAL(askingPowerOff()),confirmDialog,SLOT(doReject()));
        Screen::getInstance()->resetQueue();
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
        confirmDialog->exec();

        if(confirmDialog->result())
        {
            Screen::getInstance()->queueUpdates();
            delete confirmDialog;
            goToSettings();
            QBookForm *current_form = getCurrentForm();
            SettingsDeviceOptionsMenu *deviceOptions = ((Settings*)current_form)->getDeviceOptions();
            deviceOptions->showDictionaryMenu();
            Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
            Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
            Screen::getInstance()->flushUpdates();
        }else
        {
            Screen::getInstance()->queueUpdates();
            delete confirmDialog;
            ConfirmDialog* dialog= new ConfirmDialog(this,tr("You can perform the installation later from dictionary settings."));
            Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
            Screen::getInstance()->flushUpdates();
            dialog->exec();
            delete dialog;
        }
    }else{
        QBook::settings().setValue("setting/dictionaryAvailable", false);
        QBook::settings().sync();
    }
}
#endif

bool QBookApp::isResumingWifi()
{
    return b_isResumingWifi;
}

void QBookApp::finishedResumingWifi()
{
    qDebug() << Q_FUNC_INFO;
    b_isResumingWifi = false;
}

void QBookApp::startedResumingWifi()
{
    qDebug() << Q_FUNC_INFO;
    b_isResumingWifi = true;
}

void QBookApp::checkTimeSubscriptionToExpire()
{
#ifndef HACKERS_EDITION
    if(!QBook::settings().value("subscription/subscriptionAllowed", false).toBool())
        return;

    QDateTime expiredDate = QDateTime::fromString(QBook::settings().value("subscription/subscriptionEndDate").toString(), Qt::ISODate);
    if(!expiredDate.isValid() || m_viewer->isOpeningDoc())
        return;

    int status = QBook::settings().value("subscription/status").toInt();
    if(status == bqDeviceServicesSubscription::trial)
    {
        checkTrialTimePeriod();
        return;
    }
    QDateTime currentTime = QDateTime::currentDateTimeUtc();
    int daysLeft = currentTime.daysTo(expiredDate);
    if(daysLeft > 10)
        return;
    if(!QBook::settings().value("subscription/expiredDialogShown", false).toBool())
    {
        ConfirmDialog *subscriptionDialog = new ConfirmDialog(this);
        QBook::settings().setValue("subscription/expiredDialogShown", true);
        if(daysLeft <= 0 && currentTime > expiredDate)
        {
            subscriptionDialog->setText(tr("Your subscription has expired."));
            QBook::settings().setValue("subscription/rememberExpiredDialogShown", true);
        }
        if(0 <= daysLeft && daysLeft < 1 && currentTime < expiredDate)
        {
            subscriptionDialog->setText(tr("Your subscription will be expired today."));
            QBook::settings().setValue("subscription/rememberExpiredDialogShown", true);
        }
        else if(daysLeft == 1)
        {
            subscriptionDialog->setText(tr("Your subscription will be expired tomorrow."));
            QBook::settings().setValue("subscription/rememberExpiredDialogShown", true);
        }
        else if(1 < daysLeft && daysLeft <= 10)
            subscriptionDialog->setText(tr("Your subscription will be expired in %1 days.").arg(daysLeft));
        QBook::settings().sync();
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        subscriptionDialog->exec();
        delete subscriptionDialog;
    }
    else if(!QBook::settings().value("subscription/rememberExpiredDialogShown", false).toBool()
             && (daysLeft <= 1))
    {
        ConfirmDialog *subscriptionDialog2 = new ConfirmDialog(this);
        QBook::settings().setValue("subscription/rememberExpiredDialogShown", true);
        QBook::settings().sync();
        if(daysLeft <= 0 && currentTime > expiredDate)
            subscriptionDialog2->setText(tr("Your subscription has expired."));
        if(0 <= daysLeft && daysLeft < 1 && currentTime < expiredDate)
            subscriptionDialog2->setText(tr("Your subscription will be expired today."));
        else if(daysLeft == 1)
            subscriptionDialog2->setText(tr("Your subscription will be expired tomorrow."));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        subscriptionDialog2->exec();
        delete subscriptionDialog2;
    }
#endif
}

#ifndef HACKERS_EDITION
void QBookApp::checkTrialTimePeriod()
{
    QDateTime currentTime = QDateTime::currentDateTimeUtc();
    QDateTime expiredDate = QDateTime::fromString(QBook::settings().value("subscription/canReadSubscriptionBooksEndDate").toString(), Qt::ISODate);
    int daysLeft = currentTime.daysTo(expiredDate);
    if(daysLeft > 10)
        return;
    if(!QBook::settings().value("subscription/expiredDialogShown", false).toBool())
    {
        ConfirmDialog *subscriptionDialog = new ConfirmDialog(this);
        QBook::settings().setValue("subscription/expiredDialogShown", true);
        if(daysLeft <= 0 && currentTime > expiredDate)
        {
            subscriptionDialog->setText(tr("Your trial period has expired."));
            QBook::settings().setValue("subscription/rememberExpiredDialogShown", true);
        }
        if(0 <= daysLeft && daysLeft < 1 && currentTime < expiredDate)
        {
            subscriptionDialog->setText(tr("Your trial period will be expired today."));
            QBook::settings().setValue("subscription/rememberExpiredDialogShown", true);
        }
        else if(daysLeft == 1)
        {
            subscriptionDialog->setText(tr("Your trial period will be expired tomorrow."));
            QBook::settings().setValue("subscription/rememberExpiredDialogShown", true);
        }
        else if(1 < daysLeft && daysLeft <= 10)
            subscriptionDialog->setText(tr("Your trial period will be expired in %1 days.").arg(daysLeft));
        QBook::settings().sync();
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        subscriptionDialog->exec();
        delete subscriptionDialog;
    }
    else if(!QBook::settings().value("subscription/rememberExpiredDialogShown", false).toBool()
             && (daysLeft <= 1))
    {
        ConfirmDialog *subscriptionDialog2 = new ConfirmDialog(this);
        QBook::settings().setValue("subscription/rememberExpiredDialogShown", true);
        QBook::settings().sync();
        if(daysLeft <= 0 && currentTime > expiredDate)
            subscriptionDialog2->setText(tr("Your trial period has expired."));
        if(0 <= daysLeft && daysLeft < 1 && currentTime < expiredDate)
            subscriptionDialog2->setText(tr("Your trial period will be expired today."));
        else if(daysLeft == 1)
            subscriptionDialog2->setText(tr("Your trial period will be expired tomorrow."));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        subscriptionDialog2->exec();
        delete subscriptionDialog2;
    }
}

void QBookApp::syncSubcriptionInfo()
{

        qDebug() << Q_FUNC_INFO;

        if(ConnectionManager::getInstance()->isConnected())
        {
            checkUserSubscription();
        }
        else if(!ConnectionManager::getInstance()->isOffline()){
    #ifndef FAKE_WIFI
            resumeWifi();
            bool connected = waitForConnection();
            if(connected)
                checkUserSubscription();
    #endif
        }
        checkSubscriptionExpired();
}

void QBookApp::checkSubscriptionExpired()
{
    QDateTime expiredDate = QDateTime::fromString(QBook::settings().value("subscription/subscriptionEndDate").toString(), Qt::ISODate);
    bool isShown = QBook::settings().value("subscription/expiredSubscriptionDialogShown", false).toBool();
    QDateTime currentTime = QDateTime::currentDateTimeUtc();
    if(expiredDate.isValid() && !isShown && currentTime > expiredDate)
    {
        ConfirmDialog *subscriptionDialog = new ConfirmDialog(this);
        subscriptionDialog->setText(tr("Your subscription has expired."));
        QBook::settings().setValue("subscription/expiredSubscriptionDialogShown", true);
        QBook::settings().sync();
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        subscriptionDialog->exec();
        delete subscriptionDialog;
    }
}

void QBookApp::checkUserSubscription()
{
    qDebug() << Q_FUNC_INFO;
    if(!QBook::settings().value("subscription/subscriptionAllowedChecked", false).toBool())
    {
        bqDeviceServicesClient::SubscriptionAllowed allowed = getDeviceServices()->isSubscriptionAllowed();

        switch(allowed)
        {
        case bqDeviceServicesClient::ALLOWED:
            QBook::settings().setValue("subscription/subscriptionAllowedChecked", true);
            QBook::settings().setValue("subscription/subscriptionAllowed", true);
            break;

        case bqDeviceServicesClient::NOT_ALLOWED:
            QBook::settings().setValue("subscription/subscriptionAllowedChecked", true);
            QBook::settings().setValue("subscription/subscriptionAllowed", false);
            break;
        case bqDeviceServicesClient::UNKNOWN:
            QBook::settings().setValue("subscription/subscriptionAllowed", true);
            break;
        }
        qDebug() << Q_FUNC_INFO << "allowed: " << allowed;

    }

    // UserInfo
    bqDeviceServicesClientUserInfo userInfoSubscription;
    if(getDeviceServices()->getUserInfoSubscription(userInfoSubscription))
    {
        QBook::settings().setValue("name", userInfoSubscription.name);
        QBook::settings().setValue("surname", userInfoSubscription.surname);
        QBook::settings().setValue("subscription/hasExisted", true);
        QBook::settings().setValue("subscription/cancelled", userInfoSubscription.subscription.cancelled);
        QBook::settings().setValue("subscription/pendingPayment", userInfoSubscription.subscription.pendingPayments);
        QBook::settings().setValue("subscription/status", userInfoSubscription.subscription.status);
        QString time = userInfoSubscription.subscription.subscriptionEndDate.toString(Qt::ISODate);
        if(time != QBook::settings().value("subscription/subscriptionEndDate").toString())
        {
            QBook::settings().setValue("subscription/subscriptionEndDate", time);
            QBook::settings().setValue("subscription/canReadSubscriptionBooksEndDate", userInfoSubscription.subscription.canReadSubscriptionBooksEndDate.toString(Qt::ISODate));
            QBook::settings().setValue("subscription/expiredDialogShown", false);
            QBook::settings().setValue("subscription/rememberExpiredDialogShown", false);
            QBook::settings().setValue("subscription/expiredSubscriptionDialogShown", false);
        }
    }
    QBook::settings().sync();
}

void QBookApp::renewDictionaryToken(const QString& dixioToken)
{
    qDebug() << Q_FUNC_INFO;

    QString currentDixioKey = Dictionary::instance()->getDixioKey();

    qDebug() << Q_FUNC_INFO << "New dixioToken: " << dixioToken << " currentDixioKey: " << currentDixioKey;
    if(dixioToken.trimmed().isEmpty() || currentDixioKey.trimmed() == dixioToken.trimmed())
        return;

    QString tokenPath = Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + "dixioKey";
    QFile file(tokenPath);

    if(!file.open(QIODevice::ReadWrite))
        return;

    file.seek(0);
    file.resize(0);
    file.write(dixioToken.toAscii());
    file.close();

    Dictionary::instance()->renewDixioKey();
}

void QBookApp::askForActivationDevice()
{
    qDebug() << Q_FUNC_INFO;

    if(isActivated() || isLinked())
        return;

    qDebug() << Q_FUNC_INFO << "m_lastLinkCheck: " << m_lastActivatedCheck;
    if(m_lastActivatedCheck.daysTo(QDateTime::currentDateTime()) < 7 || m_lastLinkCheck.secsTo(QDateTime::currentDateTime()) < TIME_SHOW_ACTIVATION_DIALOG)
        return;

    QBook::settings().setValue("setting/lastActivatedCheck", QVariant(QDateTime::currentDateTime().toString(Qt::ISODate)));
    m_lastActivatedCheck = QDateTime::currentDateTime();

    if(!mass_storage)
    {
        SelectionDialog* linkSelect = new SelectionDialog(NULL,tr("Haz uso de los diccionarios y de las actualizaciones activando tu dispositivo en un solo paso."), tr("Activar"), tr("Mas tarde"));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        Screen::getInstance()->resetQueue();
        linkSelect->exec();
        bool result = linkSelect->result();
        delete linkSelect;
        if(result && getCurrentForm() != m_webWizard)
            goToWizard();

    }
}
#endif

bool QBookApp::isLedDebuggingOff()
{
#ifndef HACKERS_EDITION
    return !(m_debugDialog && m_debugDialog->isledIndicatorEnabled());
#else
    return true;
#endif
}

QString QBookApp::getImageResource(const QString& path, bool isLine)
{
    qDebug() << Q_FUNC_INFO;
    uint extension = getViewer()->getFileExtension(path);
    switch(extension)
    {
    case Viewer::EXT_CHM:
        if(isLine)
            return QString::fromUtf8(":/res/book_chm_list_big.png");
        else
            return QString::fromUtf8(":/res/no_cover_chm.png");
        break;
    case Viewer::EXT_DOC:
        if(isLine)
            return QString::fromUtf8(":/res/book_doc_list_big.png");
        else
            return QString::fromUtf8(":/res/no_cover_doc.png");
        break;
    case Viewer::EXT_EPUB:
        if(isLine)
            return QString::fromUtf8(":/res/book_epub_list_big.png");
        else
            return QString::fromUtf8(":/res/no_cover_epub.png");
        break;
    case Viewer::EXT_FB2:
        if(isLine)
            return QString::fromUtf8(":/res/book_fb2_list_big.png");
        else
            return QString::fromUtf8(":/res/no_cover_fb2.png");
        break;
    case Viewer::EXT_HTML:
        if(isLine)
            return QString::fromUtf8(":/res/book_html_list_big.png");
        else
            return QString::fromUtf8(":/res/no_cover_html.png");
        break;
    case Viewer::EXT_MOBI:
        if(isLine)
            return QString::fromUtf8(":/res/book_mobi_list_big.png");
        else
            return QString::fromUtf8(":/res/no_cover_mobi.png");
        break;
    case Viewer::EXT_PDF:
        if(isLine)
            return QString::fromUtf8(":/res/book_pdf_list_big.png");
        else
            return QString::fromUtf8(":/res/no_cover_pdf.png");
        break;
    case Viewer::EXT_RTF:
        if(isLine)
            return QString::fromUtf8(":/res/book_rtf_list_big.png");
        else
            return QString::fromUtf8(":/res/no_cover_rtf.png");
        break;
    case Viewer::EXT_TXT:
        if(isLine)
            return QString::fromUtf8(":/res/book_txt_list_big.png");
        else
            return QString::fromUtf8(":/res/no_cover_txt.png");
        break;
    default:
        if(isLine)
            return QString::fromUtf8(":/res/unknow_list_big.png");
        else
            return QString::fromUtf8("res/unknow_file.png");
        break;
    }
}

void QBookApp::handleViewerConf()
{
    getStatusBar()->setSpinner(true);
    Screen::getInstance()->queueUpdates();
    if(!m_settingsMenu)
    {
        goToSettings();
        m_settingsMenu->goToViewerMenu();
    }
    else
    {
        m_settingsMenu->goToViewerMenu();
        goToSettings();
    }
    connect(m_settingsMenu, SIGNAL(goToViewer()), this, SLOT(handleGoToViewer()));
    Screen::getInstance()->flushUpdates();
    getStatusBar()->setSpinner(false);
}

void QBookApp::handleGoToViewer()
{
    if(m_settingsMenu)
        disconnect(m_settingsMenu, SIGNAL(goToViewer()), this, SLOT(handleGoToViewer()));
    openLastContent();
}

void QBookApp::generateBookCover( BookInfo* bookInfo)
{
    qDebug() << Q_FUNC_INFO;

    // Get expected thumbnail image path
    QFileInfo fi(bookInfo->path);

    // Check dir
    QDir dir(fi.absolutePath());
    if(!dir.exists( ".thumbnail/"))
        dir.mkdir( ".thumbnail/");

    QString thumbnailPath(fi.absolutePath() + QDir::separator() + ".thumbnail" + QDir::separator() + fi.fileName() + THUMBNAIL_SUFIX);

    bool gotCover = MetaDataExtractor::extractCover(bookInfo->path, thumbnailPath);
    if (!gotCover)
        thumbnailPath = QDocView::coverPage(bookInfo->path, thumbnailPath); // Render 1st page if not have cover.

    QImageReader image(thumbnailPath);
    QFile imageFile(thumbnailPath);
    if(!image.canRead())
    {
        qWarning() << Q_FUNC_INFO << "Error extracting cover";
        return;
    }
    else
    {
        Library::fromCover2Thumbnail(thumbnailPath);
        QImageReader modifiedImage(thumbnailPath);
        thumbnailPath += "." + modifiedImage.format();
        imageFile.rename(thumbnailPath);

        bookInfo->thumbnail = thumbnailPath;
        getModel()->updateBook(bookInfo);
    }
}

QList<QByteArray> QBookApp::getSupportedImageslist()
{
    qDebug() << Q_FUNC_INFO;
    if(supportedImageslist.isEmpty())
        supportedImageslist = QImageReader::supportedImageFormats();

    return supportedImageslist;
}

void QBookApp::createNewCollection(const BookInfo* bookToAdd)
{
    goToLibrary();
    m_library->setFromViewer(true);
    m_library->createNewCollection(bookToAdd);
}

void QBookApp::connectConnectionManagerSignals()
{
    qDebug() << Q_FUNC_INFO;
    connect(m_connectManager, SIGNAL(connected()), this, SLOT(releaseAutoconnectPowerLock()), Qt::DirectConnection);
    connect(m_connectManager, SIGNAL(connected()), this, SLOT(activateConnectedPowerLock()));
    connect(m_connectManager, SIGNAL(connected()), this, SLOT(finishedResumingWifi()));
    connect(m_connectManager, SIGNAL(connected()), this, SLOT(setWifiStatusAsConnected()));
    connect(m_connectManager, SIGNAL(connected()), this, SLOT(askForActivationDevice()), Qt::UniqueConnection);
    connect(m_connectManager, SIGNAL(connected()), this, SLOT(synchronization()), Qt::UniqueConnection);
    connect(m_connectManager, SIGNAL(connected()), this, SLOT(checkNewDictionaries()), Qt::UniqueConnection);
    connect(m_connectManager, SIGNAL(connected()), this, SLOT(checkFwVersion()));
}

void QBookApp::disconnectConnectionManagerSignals()
{
    qDebug() << Q_FUNC_INFO;
    disconnect(m_connectManager, SIGNAL(connected()), this, SLOT(releaseAutoconnectPowerLock()));
    disconnect(m_connectManager, SIGNAL(connected()), this, SLOT(activateConnectedPowerLock()));
    disconnect(m_connectManager, SIGNAL(connected()), this, SLOT(finishedResumingWifi()));
    disconnect(m_connectManager, SIGNAL(connected()), this, SLOT(setWifiStatusAsConnected()));
    disconnect(m_connectManager, SIGNAL(connected()), this, SLOT(askForActivationDevice()));
    disconnect(m_connectManager, SIGNAL(connected()), this, SLOT(synchronization()));
    disconnect(m_connectManager, SIGNAL(connected()), this, SLOT(checkNewDictionaries()));
    disconnect(m_connectManager, SIGNAL(connected()), this, SLOT(checkFwVersion()));
}

void QBookApp::checkBooksChanged()
{
    qDebug() << Q_FUNC_INFO;
    QList<const BookInfo*> m_books;
    m_books.clear();
    getModel()->getBooksInPath(m_books, Storage::getInstance()->getPublicPartition()->getMountPoint());
    if(Storage::getInstance()->getRemovablePartition() && Storage::getInstance()->getRemovablePartition()->isMounted())
        getModel()->getBooksInPath(m_books, Storage::getInstance()->getRemovablePartition()->getMountPoint());

    QList<const BookInfo*>::const_iterator it = m_books.constBegin();
    QList<const BookInfo*>::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        BookInfo* book = new BookInfo (*(*it));
        QFileInfo bookFile(book->path);
        bookFile.refresh();
        if(book->size != bookFile.size())
        {
            QFile::remove(book->thumbnail);
            getModel()->loadDefaultInfo(book->path);
        }
    }
}

