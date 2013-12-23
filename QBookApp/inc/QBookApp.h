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

#ifndef QBOOKAPP_H
#define QBOOKAPP_H

#include <QWidget>
#include <QSize>
#include "ui_QBookApp.h"
#include "Keyboard.h"
#include "SettingsWiFiList.h"
#include "Dictionary.h"
// Predeclarations
class QBookForm;
class BookInfo;
class QFwUpgradeOTA;
class bqSettingsMenu;
class Model;
class StorageDevice;
class StoragePartition;
class ConnectionManager;
class PasswordRequester;
#include "SyncHelper.h"
class ElfSettingsMenu;
class WizardWelcome;
class Settings;
class QBookListBrowser;
class Browser;
class StatusBar;
class OpenBooks;
class bqHome;
class InfoDialog;
class ProgressDialog;
class QDialog;
class QBookDebugDialog;
class PowerManagerLock;
class bqDeviceServices;
class LibrarySyncHelper;
class QBookScreenSaver;

#ifdef __QBOOK_ARCHIVE__
class QArchiveBrowser;
#endif

#ifdef	_QBOOK_RSS_READER_
class QFeedReaderBrowser;
#endif

class Viewer;
class QSettingInfoDialog;
class QSettingLinkPc;
class Home;
class Library;
class PowerManagerLock;
#ifndef HACKERS_EDITION
class bqDeviceKey;
class WebStore;
class WebWizard;
#endif
class SettingsQuickSettingsPopup;
class Search;
class DeviceInfoDialog;

class QBookApp : public QWidget, protected Ui::QBookApp
{
	Q_OBJECT

public:
    enum RefreshModel{
        CMD_ALL,
        CMD_BOOK,
        CMD_IMAGE,
        CMD_AUDIO,
        CMD_SIM

	};

   enum {
       BOOK_ST_OPEN,
       BOOK_ST_CLOSED
   };

   enum {
       BOOK_RST_READING,
       BOOK_RST_READ,
       BOOK_RST_NOTREAD
   };

  enum  RequestConnectionResult {
       CONNECTED,
       AUTOCONNECTING,
       NOT_CONNECTED,
       WIFI_LIST
  };

private:
    QBookApp(QWidget* parent = 0, Qt::WFlags flags = 0);
    virtual ~QBookApp();

    // Singleton instance
    static QBookApp* m_spInstance;
    QSettingLinkPc *link;
    DeviceInfoDialog* deviceInfoDialog;

    SettingsQuickSettingsPopup*             quickSettingsPopup;

public:
    // Singleton methods: access, creation and deletion
    static QBookApp*    instance            () { return m_spInstance; }
    static void         staticInit          ();
    static void         staticDone          ();

    void                setupSSLconfig      ( const QString& );
    void                syncModel           ();


    void                pushTopForm         ( QBookForm* form );
    void                pushForm            ( QBookForm* form );
    void                popForm             ( QBookForm* form );
    void                popAllForms         ();

    Keyboard*           showKeyboard        ( const QString& text,bool showUp = false, Keyboard::MODE keyboardMode = Keyboard::NORMAL );
    inline StatusBar*   getStatusBar        ();
    Viewer*             getViewer           ();

    SettingsWiFiList *      m_wifiSelection;
    inline Library*         getLibrary              () const { return m_library; }
#ifndef HACKERS_EDITION
    bqDeviceKey*            getBqDeviceKey          () const { return m_bqDeviceKey; }
    bqDeviceServices* getDeviceServices() const { return m_pServices; }
    WebStore* browserStore();
    WebWizard* browserWizard();
#endif

    static bool isImage                 ( const QString& );
    Browser*    browser                 ();
    void        emitSwipe               (int);
    bool        isLinked                ( );
    bool        isActivated             ( );

    Keyboard* getKeyboard() const { return m_pKeyboard; }

    Home* m_home;

    void setOTAUpdating( bool value) { b_isOTAUpdating = value; }
    bool userEventsBlocked() { return b_userEventsBlocked; }
    bool isViewerCurrentForm();
    bool isSleeping(){ return b_isSleeping;}
    bool isSynchronizing();
    void fillBookInfoWithDataMap(BookInfo *bookinfo, const QVariantMap &locations);
    bool poweringOff() {return isPoweringOff;}
    void setPoweringOff(bool value) { isPoweringOff = value;}
    void cancelSync();
    bool isPoweringOffDevice() { return b_powerOffOngoing; }
    bool isLinkToPc()          { return mass_storage; }

    QBookForm*                                                  getCurrentForm                                  () const;
    inline Model*                                               getModel                                        () const { return m_pModel; }
    void                                                        setBuying                                       ( bool buying ) { b_isBuying = buying; }
    bool                                                        isBuying                                        () { return b_isBuying; }
    void                                                        prepareViewerToSync                             ();
    BookInfo*                                                   openDocWithoutActivateForm                      ( const BookInfo* );
#ifndef HACKERS_EDITION
    QList<DictionaryParams>                                     getAvailableDicts                               (){return availableDictsList;}
#endif
    void                                                        stopThumbnailGeneration                         ( );
    void                                                        resumeThumbnailGeneration                       ( );
    QSize                                                       getHomeThumbnailSize                            ( );
    bool                                                        isResumingWifi                                  ( );
    void                                                        closeSleep                                      ( );
    void                                                        enablePowerKeyWatcher                           (bool);
    void                                                        shuttingDown                                    ( );
    QString                                                     getImageResource                                (const QString& path, bool isLine = false);

signals:
    void popupForm();
    void startSleep();
    void powerButtonPress();
    void moveSyncDialog();
    void finishRefresh();
    void menuKeyPressed();
    void userEvent();
    void passwordRequested();
    void passwordSent();
    void wrongPassword(QString);
    void cancelPassword();
    void connectionFailed(QString, QString);
    void hideLibrary();
    void swipe(int);
    void wifiConnectionCanceled();
    void askingPowerOff();
    void periodicSyncFinished();
    void syncHelperCreated();
    void cancelSleep();
    void linked();
    void syncModelFinished();
    void powerKeyLongPressed();
    void connmandRestarted();
    void resumingWifiFailed();
    void reportStoreLink();
    void closedWizard();
    void openImage(const QString&);

#ifdef Q_WS_QWS
    void grabScreen(QImage* image);
#endif    

public slots:
    void goToManual();
    void goToHome();
    void goToLibrary();
    void goToSettings();
    void goToBrowser();
#ifndef HACKERS_EDITION
    void closeWizard();
    void syncRequesterServices();
    void goToWizard();
    void goToShop();
    void checkSubscriptionExpired();
#endif
    void checkLink();
    void goToSearch();
    void goToSleep();
    void openContent(const BookInfo*);
    void openLastContent();     // Open last book readed
    void restartWifiSleepTimer();

    virtual int requestConnection(bool allowedToRequestInteraction = true, bool wizard = false);
    void                                loadDocError                        ();
    void checkFwVersion();

    void partitionUmountFailed(StoragePartition*);
    void deviceRemoved(StorageDevice*);
    void lostConnection();
    void resumeWifi();
    void powerOffWifiAsync();
    void showWifiSelection(SettingsWiFiList::eBehavior = SettingsWiFiList::NO_ACTION_BEHAVIOR);
    void showQuickSettingsPopup();
    void showDeviceInfoDialog();
    void disablePowerKey();
    void lockAutoconnectOnWifiPower(bool);
    void showLoadingBooksDialog(int);
    void hideLoadingBooksDialog();

    void checkRebootFromRestore();
    void checkLanguageChanged();
    void tabChange(int cmd);
#ifndef HACKERS_EDITION
    void openStoreWithUrl(const QString& = "");
#endif
    void openBrowser(const QString&);

    Keyboard *hideKeyboard();

    void enableUserEvents();
    void disableUserEvents();
    void requestSilentConnection();
    void showRestoringImage();
    void setWifiStatusAsConnected();
    void releaseAutoconnectPowerLock();
#ifndef HACKERS_EDITION
    void                                                        synchronization                                 (bool lazyDelete = true);
    void                                                        synchronizationEnd                              (int, bool);
#endif
    SyncHelper*                                                 getSyncHelper                                   ();
    void                                                        createSyncHelper                                ();
    void                                                        initSyncHelper                                  ();
    void                                                        setScreensaverCover                           (const BookInfo* inputBookInfo);
    void                            finishedResumingWifi                    ();
    void                            startedResumingWifi                     ();

protected slots:
    void handleChargerChanged(bool state);
    void showChargerDialog();
    void handleUsbStateChanged(bool state);
    void handleRemoveableChanged(bool state, const QString& path);

    void handleLowBattery(int);
    void handleCriticalBattery();
    void handleFullBattery();

    void confirmFwUpgrade();
    void askPowerOffDevice();
    void powerLight();
    void checkStartupDebugOpts();
    void firstChecksAfterStartup();
    void connectDialogSlots();
    void activateConnectedPowerLock();
    void releaseConnectedPowerLock();
    void checkConnectedPowerLock();
    void viewRecentBooks();
#ifndef HACKERS_EDITION
    void launchSync();
    void syncOnlyViewerBook();
#endif


    void checkScreensaverCover();
    void checkScreensaverCoverAfterSync();
    void handleViewerConf();
    void handleGoToViewer();


protected:
    virtual void init();
    virtual void initConnectManager();
    virtual void initViewer();
    virtual void initLibrary();
    virtual void initWebView();
#ifndef HACKERS_EDITION
    virtual void initWebStore();
    virtual void initWebWizard();
#endif
    virtual void release();
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    void firseInit();
    void takeScreenShot();

    virtual bool eventFilter(QObject* watched, QEvent* event);

private slots:
    void handlePartitionMounted(StoragePartition*);
    void handlePartitionUmounted(StoragePartition*);
    void handlePartitionCorrupted(StoragePartition *);
    void handleUnknownStorage(StorageDevice *);
    void unlinkForced();
    void prepareToSleep();
    void resumeAfterSleep();
    void WifiLevelChanged(int);
    void gotOTACheckResult(int);
    void setHwClock();
    void doUpdateBook(const QVariantMap&, const QVariantMap&);
    void doAddBook(const QVariantMap&, const QVariantMap&);
    void doRemoveBook(const QString&);
    void dbusServicesChanged( const QString & name, const QString & oldOwner, const QString & newOwner );
    void connectedAfterSleepForSync();
    void enablePowerKey();
    void wifiProblem();
    void closePcLink();
#ifndef HACKERS_EDITION
    void requestHackersInstallation();
    void                            checkNewDictionaries                    ();
    void                            checkTrialTimePeriod                    ();
    void                            checkUserSubscription                   ();
    void                            connectWifiObserverActivateStore        ();
    void                            disconnectWifiObserverActivateStore     ();
    void                            askForActivationDevice                  ();
#endif
    void                            checkTimeSubscriptionToExpire           ();
    void                            prepareToShutdown                       ();
    void                            connectWifiObserverGoToShop             ();
    void                            disconnectWifiObserverGoToShop          ();
    void                            checkLongPressPowerOff                  ();

private:
    void                            doActivateForm                          ( QBookForm* form );
    void                            doDeactivateForm                        ( QBookForm* form );
    void                            doDeleteForm                            ( QBookForm* form );
    void                            doAddForm                               ( QBookForm* form );

    void closeNetworkPower();
    void updateVersionFromOTA();
    void powerOffWifi();
    void cancelOnGoingSleep();
    void showSleep();
    bool mass_storage;
    bool b_charger_state;
    bool b_synchronizing;
    bool isPoweringOff;
    bool b_keyBlocked;
    bool b_lazyDelete;
    bool b_isResumingWifi;


    void updateBookInfoWithDataMap(BookInfo *bookinfo, const QVariantMap &map);
    void fillBookInfoWithLocationMap(BookInfo *bookinfo, const QVariantMap &locations);
    bool waitForConnection();
    void waitForSyncFinished();
    void removeExpiredBooks();
    void hackersInit();
    void                            resetServiceSettings                    ( );
    bool                            isLedDebuggingOff                       ( );
    void                            renewDictionaryToken                    ( const QString& );

    Model*                  m_pModel;

    Library*                m_library;
    Settings*               m_settingsMenu;
    Search*                 m_search;
    Browser*                m_webView;
#ifndef HACKERS_EDITION
    WizardWelcome*          m_welcomeWizard;
    bqDeviceServices*       m_pServices;
    bqDeviceKey*            m_bqDeviceKey;
    QList<DictionaryParams>                         availableDictsList;
    WebStore* m_webStore;
    WebWizard* m_webWizard;
    bool checkSDForHackers();
    bool askForHackersInstallation();
    void allowHackersInstallation();
    QString getHackersAuth();
    bool deviceHackAuthorized (QString);
    bool checkDebugMagicFile(const QString& magicFileName, const QString& code);
    bool checkDebugMagicSignature(QByteArray line, QByteArray signature);
    void                            linkFailed                      ( );
    void                            checkOTAFile                    ( );
#endif

    QList<QBookForm*>       m_forms;
    QList<QBookForm*>       m_activeForms;

    QBookListBrowser* m_book;



    ConnectionManager* m_connectManager;
    PasswordRequester* m_passwordRequester;
    OpenBooks* m_openBooksLayer;
    Keyboard *m_pKeyboard;
    Viewer *m_viewer;
    int res;
    QTimer m_wifiSleepTimer;
    QTimer m_timer_powerButton;
    QTimer m_sleepSyncTimer;
    QTimer m_timer_light;
    QTimer m_connectAfterSleepTimer;
    QTimer m_afterWifiOnTimer;
    QTimer m_powerKeyWatcher;

#ifdef __QBOOK_ARCHIVE__
	QArchiveBrowser *m_ArchiveBrowser;
#endif

    bool b_powerOffOngoing;
    int i_powerKeySamplesCounter;

    QFwUpgradeOTA *fwDwld;
    InfoDialog* m_OTAdownloadingDialog;
    InfoDialog* m_syncDialog;
    InfoDialog* readingFilesdialog;
    InfoDialog* b_batteryLevel;
    ProgressDialog* loadingBooksDialog;


    QBookDebugDialog* m_debugDialog;

    PowerManagerLock* m_autoconnect_lock;
    PowerManagerLock* m_connected_lock;
    PowerManagerLock* m_usbChangeLock;
    PowerManagerLock* m_powerKeyLock;
    PowerManagerLock* m_homeKeyLock;
    PowerManagerLock* m_syncLock;
    NetworkService* m_connectingNetwork;
    QDateTime m_lastOTACheck;
    QDateTime m_lastLinkCheck;
    QDateTime m_lastActivatedCheck;
    SyncHelper *m_sync;
    QBookScreenSaver *sleepScreen;
    bool bLoadWizard;
    bool m_openDocError;
    bool networkStartTimerFlag;
    bool m_isDialogShow;
    bool m_mmcState;
    bool b_userEventsBlocked;
    bool b_isOTAUpdating;
    bool b_blockBtns;
    bool b_wifiWasConnected;
    bool b_cancelSleep;
    bool b_isSleeping;
    bool b_isBuying;
    bool b_wizardFromStore;
    QProcess *offlineHelper;
};

#endif
