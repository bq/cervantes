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

#include "SettingsWiFiList.h"

#include "SettingsConfigureNetwork.h"
#include "FullScreenWidget.h"
#include "QFwUpgradeOTA.h"
#include "SettingsWiFiDetail.h"
#include "QBookApp.h"
#include "InfoDialog.h"
#include "PowerManager.h"
#include "PowerManagerDefs.h"
#include "Screen.h"
#include "Wifi.h"
#include "Keyboard.h"
#include "SelectionDialog.h"
#include "MyQLineEdit.h"

#include <QPainter>

#ifdef Q_WS_QWS
#define CONNECT_HELPER "/app/bin/Connect"
#else
#define CONNECT_HELPER "./ConnectionManager/connect/x86-bin/Connect"
#endif

#define CONNECTION_TIMEOUT 1000 * 15

const int ITEMS_PER_PAGE = 6;

// PUBLIC

SettingsWiFiList::SettingsWiFiList(QWidget* parent, SettingsWiFiList::eBehavior behaviour) : QBookForm(parent)
    , connectionRequested(false)
    , b_showWifiSetting(false)
    , deleted(false)
    , connectingTimer(this)
    , i_timeSleepMSecs(0)
    , i_currentLevel(-1)
    , i_lastWifiListCount(0)
{
        qDebug() << Q_FUNC_INFO << parent;

        setupUi(this);

        m_powerLock = PowerManager::getNewLock(this);

        connectingTimer.setInterval(CONNECTION_TIMEOUT);

        connectHelper = NULL;
        connectingService = NULL;
        // Set up buttons

        wifis.append(Item1);
        wifis.append(Item2);
        wifis.append(Item3);
        wifis.append(Item4);
        wifis.append(Item5);
        wifis.append(Item6);

        connect(Item1, SIGNAL(wifiSelected(Network)), this, SLOT(handleWifiSelected(Network)));
        connect(Item2, SIGNAL(wifiSelected(Network)), this, SLOT(handleWifiSelected(Network)));
        connect(Item3, SIGNAL(wifiSelected(Network)), this, SLOT(handleWifiSelected(Network)));
        connect(Item4, SIGNAL(wifiSelected(Network)), this, SLOT(handleWifiSelected(Network)));
        connect(Item5, SIGNAL(wifiSelected(Network)), this, SLOT(handleWifiSelected(Network)));
        connect(Item6, SIGNAL(wifiSelected(Network)), this, SLOT(handleWifiSelected(Network)));

	connect(Item1, SIGNAL(wifiLongPressed(Network)), this, SLOT(handleWifiLongPressed(Network)));
        connect(Item2, SIGNAL(wifiLongPressed(Network)), this, SLOT(handleWifiLongPressed(Network)));
        connect(Item3, SIGNAL(wifiLongPressed(Network)), this, SLOT(handleWifiLongPressed(Network)));
        connect(Item4, SIGNAL(wifiLongPressed(Network)), this, SLOT(handleWifiLongPressed(Network)));
        connect(Item5, SIGNAL(wifiLongPressed(Network)), this, SLOT(handleWifiLongPressed(Network)));
        connect(Item6, SIGNAL(wifiLongPressed(Network)), this, SLOT(handleWifiLongPressed(Network)));



        // Connect page handler
        connect(settingsPagerCont,SIGNAL(previousPageReq()),this,SLOT(back()));
        connect(settingsPagerCont,SIGNAL(nextPageReq()),this,SLOT(forward()));

        qDebug() << Q_FUNC_INFO << "wifi/disabled setting is set to " << QBook::settings().value("wifi/disabled",false).toBool();

        manager = ConnectionManager::getInstance();


        wifiSignalLbl->hide();
        wifiNameLbl->hide();
        spinnerWifiLbl->hide();

        setBehaviour(behaviour);
        connect(wifiStatusBtn,SIGNAL(clicked()),this,SLOT(powerOnOffWifi()));
        connect(updateWifiListBtn, SIGNAL(clicked()), this, SLOT(updateWifiList()));
	connect(addNetworksBtn,SIGNAL(clicked()),this,SLOT(addWifi()));

        connect(manager, SIGNAL(connected()), this, SLOT(connected()));
        connect(manager, SIGNAL(connected()), this, SLOT(handleConnected()));
        connect(manager, SIGNAL(disconnected()), this, SLOT(handleDisconnected()));
        connect(manager, SIGNAL(servicesChanged()),  this, SLOT(networksCountChanged()));
        connect(manager, SIGNAL(scanFinished()), this, SLOT(paintStatus()));

        connect(manager, SIGNAL(someThingHasChanged()), this, SLOT(paintWifiList()));


        // Connects signals from wifi items in list
        m_infoDialog = new InfoDialog(this);

        dialog = new SelectionDialog(this, tr("Add network"));
        edit = new MyQLineEdit();
        dialog->addWidget(edit);

        qApp->installEventFilter(this);

        if(!QBook::settings().value("wifi/disabled",false).toBool())
            powerOnWifi();
        else
            powerOffWifi();

        connect(QBookApp::instance(),SIGNAL(userEvent()),this,SLOT(resetSleepTimer()));

        QFile fileSpecific(":/res/settings_styles.qss");
        QFile fileCommons(":/res/settings_styles_generic.qss");
        fileSpecific.open(QFile::ReadOnly);
        fileCommons.open(QFile::ReadOnly);

        QString styles = QLatin1String(fileSpecific.readAll() + fileCommons.readAll());
        setStyleSheet(styles);

}

SettingsWiFiList::~SettingsWiFiList()
{
    qDebug() << Q_FUNC_INFO;
    delete m_powerLock;
    m_powerLock = NULL;

    delete m_infoDialog;
    m_infoDialog = NULL;

    hideAddNetworkDialog();

    manager = NULL;
    deleted = true;

    disconnect(QBookApp::instance(),SIGNAL(userEvent()),this,SLOT(resetSleepTimer()));
}

void SettingsWiFiList::showDialog(QString text)
{
    qDebug() << Q_FUNC_INFO;

    m_infoDialog->setText(text);
    qApp->processEvents();

}

void SettingsWiFiList::activateForm(){
    qDebug() << "--->" << Q_FUNC_INFO;
    if(!QBook::settings().value("wifi/disabled",false).toBool()){
        m_powerLock->activate();
        setUiPowerOnOff(true);
    }else{
        setUiPowerOnOff(false);
    }
    i_timeSleepMSecs = QBook::settings().value("setting/sleepTimeInSecs", POWERMANAGER_TIME_AUTOSLEEPSECS).toInt() * 1000;
    m_sleepTimer.start(i_timeSleepMSecs,this);
    b_showWifiSetting = false;

    setBehaviour(getBehaviour());

}

void SettingsWiFiList::deactivateForm(){
    qDebug() << "--->" << Q_FUNC_INFO;
    m_powerLock->release();
    m_sleepTimer.stop();
    if(dialog && dialog->isVisible())
        hideAddNetworkDialog();

}

void SettingsWiFiList::showConnectingMessage()
{
    qDebug() << "Showing connecting dialog";
    showDialog(tr("Conectando..."));
    wifiConnectMsgLbl->setText(tr("Conectando..."));

}

void SettingsWiFiList::showDisconnectingMessage()
{
    qDebug() << "Showing disconnecting dialog";
    showDialog(tr("Desconectando..."));
}

void SettingsWiFiList::addWifiEditClicked() {
    Keyboard *keyboard = QBookApp::instance()->showKeyboard(tr("Hide"), false, Keyboard::NORMAL);
    connect(keyboard, SIGNAL(actionRequested()), this, SLOT(addWifiEditHide()));
    MyQLineEdit *edit = (MyQLineEdit*)sender();
    keyboard->handleMyQLineEdit(edit);
}

void SettingsWiFiList::addWifiEditHide() {
    Keyboard *keyboard = QBookApp::instance()->getKeyboard();
    keyboard->hide();
}

void SettingsWiFiList::addWifiEditDialogAccepted() {
    QString networkName;
    dialog = (SelectionDialog*)sender();
    edit = (MyQLineEdit*)dialog->getWidget();

    networkName = edit->text();

    if (networkName.isEmpty())
        return;

    addWifiEditHide();
    hideAddNetworkDialog();

    foreach (Network n, m_available_wifis) {
	if (n.getName() == networkName) {
        InfoDialog *infoDialog = new InfoDialog(this,tr("Network already added."));
	    infoDialog->showForSpecifiedTime();
	    delete infoDialog;
            return;
	}
    }

    qDebug() << "FIXME: Show spinner here";
    qDebug() << "Scanning for: " << networkName;
    bool res = manager->scanForESSID(networkName);
    qDebug() << "FIXME: Close spinner here";
    qDebug() << "Result was: " << res;
    InfoDialog *infoDialog;
    if (res) {
        infoDialog = new InfoDialog(this,tr("Network added correctly to the list."));
    } else {
        infoDialog = new InfoDialog(this,tr("Network not found."));
    }
    infoDialog->showForSpecifiedTime();
    delete infoDialog;
}

void SettingsWiFiList::addWifiEditDialogRejected() {
    Screen::getInstance()->queueUpdates();
    dialog = (SelectionDialog*)sender();
    edit = (MyQLineEdit*)dialog->getWidget();
    addWifiEditHide();
    hideAddNetworkDialog();
    Screen::getInstance()->flushUpdates();
}

void SettingsWiFiList::addWifi()
{
    qDebug() << "--->" << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    if(!dialog) {
        dialog = new SelectionDialog(this, tr("Add network"));
        edit = new MyQLineEdit();
        dialog->addWidget(edit);
    }

    connect(edit, SIGNAL(clicked()), this, SLOT(addWifiEditClicked()));

    Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
    dialog->show();
    connect(dialog, SIGNAL(accepted()), this, SLOT(addWifiEditDialogAccepted()));
    connect(dialog, SIGNAL(rejected()), this, SLOT(addWifiEditDialogRejected()));
}

void SettingsWiFiList::updateWifiList()
{
    qDebug() << "--->" << Q_FUNC_INFO;

    if(dialog && dialog->isVisible()) {
        hideAddNetworkDialog();
        return;
    }

    if (!connectionRequested) {
        QBookApp::instance()->getStatusBar()->setWifiStatus(StatusBar::WIFI_PROCESSING);
        wifiConnectMsgLbl->setText(tr("Updating..."));
    }

    manager->scan();

}

void SettingsWiFiList::paintStatus(){

    qDebug() << Q_FUNC_INFO;
    if (connectionRequested) {
        qDebug() << Q_FUNC_INFO << "don't paint wifi status when we're connecting";
        return;
    }

    QString path = manager->getConnectingNetwork();

    if(manager->isConnected()){
        QBookApp::instance()->setWifiStatusAsConnected();
        wifiStatusBtn->setStyleSheet("background-image: url(:/res/on_btn.png);");
        wifiConnectMsgLbl->setText(tr("Connected"));
    }else if(path.isEmpty()){
        QBookApp::instance()->getStatusBar()->setWifiStatus(StatusBar::WIFI_OFF);

        if(QBook::settings().value("wifi/disabled",false).toBool())
        {
            wifiConnectMsgLbl->setText(tr("Turned OFF"));
            wifiStatusBtn->setStyleSheet("background-image: url(:res/off_btn.png);");
        }else{
            wifiStatusBtn->setStyleSheet("background-image: url(:/res/on_btn.png);");
            wifiConnectMsgLbl->setText(tr("Disconnected"));
        }

    }else{
        QBookApp::instance()->getStatusBar()->setWifiStatus(StatusBar::WIFI_PROCESSING);
        wifiStatusBtn->setStyleSheet("background-image: url(:/res/on_btn.png);");
        wifiConnectMsgLbl->setText(tr("Connecting..."));
    }


}

void SettingsWiFiList::paintWifiList()
{
    qDebug() << Q_FUNC_INFO;
    if (QBookApp::instance()->getKeyboard()->isVisible()) {
        qDebug() << Q_FUNC_INFO << "don't repaint wifi list when the keyboard is visible (refresh issue)";
        return;
    }

    paintStatus();

    m_available_wifis = manager->getNetworks();
    i_lastWifiListCount = m_available_wifis.size();
    setupWifis(m_available_wifis);
}

void SettingsWiFiList::networksCountChanged()
{
        qDebug() << "--->" << Q_FUNC_INFO;
        paintWifiList();
}

void SettingsWiFiList::connectHelperOutput()
{
        QString output = connectHelper->readAllStandardError();
        output += connectHelper->readAllStandardOutput();
        qDebug() << Q_FUNC_INFO << "Output from Connect helper: " << output;
}

void SettingsWiFiList::connectHelperFinished(int code, QProcess::ExitStatus status)
{
        qDebug() << Q_FUNC_INFO << "Connect helper finished with code " << code << "and status " << status;
        connectHelper->deleteLater();
        connectHelper = NULL;
}

void SettingsWiFiList::wifiSelected(QString path)
{
    Network network = ConnectionManager::getInstance()->getNetwork(path);
    qDebug() << "--->" << Q_FUNC_INFO << ": name " << network.getName();

    if (m_available_wifis.size() <= 0) return;

    // We run in a separate process because it is blocking and we want to get notificacions for Agent
    if (connectHelper != NULL)
        delete connectHelper;


    QString state = network.getState();
    qDebug() << Q_FUNC_INFO << "selected network state: " << state;

    qDebug() << "Connecting to selected network:" << network.getName();

    connect(QBookApp::instance(), SIGNAL(passwordRequested()), this, SLOT(passwordRequested()));
    connect(QBookApp::instance(), SIGNAL(passwordSent()), this, SLOT(passwordSent()));
    connect(QBookApp::instance(), SIGNAL(wrongPassword(QString)), this, SLOT(wrongPassword(QString)),Qt::UniqueConnection);
    connect(QBookApp::instance(), SIGNAL(cancelPassword()), this, SLOT(cancelPassword()));
    connect(QBookApp::instance(), SIGNAL(connectionFailed(QString, QString)), this, SLOT(connectionFailed(QString, QString)));



    // Timer to asume we cannot connect to the network (like powering off AP while associating)
    connect(&connectingTimer, SIGNAL(timeout()), this, SLOT(connectionTimeout()));
    connectingTimer.start();

    connectionRequested = true;

    connectHelper = new QProcess(this);
    QStringList args;
#ifdef Q_WS_QWS
    args << "-qws";
#endif
    args << network.getPath();
    qDebug() << Q_FUNC_INFO << "Launching Connect with args: " << args;
    connect(connectHelper, SIGNAL(readyReadStandardError()), this, SLOT(connectHelperOutput()));
    connect(connectHelper, SIGNAL(readyReadStandardOutput()), this, SLOT(connectHelperOutput()));
    connect(connectHelper, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(connectHelperFinished(int,QProcess::ExitStatus)));

    connectingService = manager->getNetworkService(network.getPath());
    if (!connectingService) {
        wifiError(tr("Error conectando"));
        paintWifiList();
        return;
    }
    connect(connectingService, SIGNAL(connected()), this, SLOT(connectedNetwork()));
    connect(connectingService, SIGNAL(error()), this, SLOT(connectingError()));

    connectHelper->start(CONNECT_HELPER, args);

}

void SettingsWiFiList::connectionTimeout() {
    qDebug() << Q_FUNC_INFO;
    wifiError(tr("Error conectando"));
    if(m_available_wifis.size() > 0){
        QString name = m_available_wifis[0].getName();
        name.replace(tr("Connecting to..."), "");
        m_available_wifis[0].setName(name);
        qApp->processEvents();
    }

    paintWifiList();
}

void SettingsWiFiList::passwordRequested() {
    qDebug() << Q_FUNC_INFO << "Stoping connectionTimer because we are wating for password from UI";
    connectingTimer.stop();
    m_infoDialog->hide();
}

void SettingsWiFiList::passwordSent() {
    qDebug() << Q_FUNC_INFO << "Restarting connectionTimer because we got password from UI";
    showConnectingMessage();
    connectingTimer.start();
}

void SettingsWiFiList::connectingError() {

    if(m_available_wifis.size() > 0){
        QString name = m_available_wifis[0].getName();
        name.replace(tr("Connecting to..."), "");
        m_available_wifis[0].setName(name);
    }

    // We only want to report unknown failures when not running the connectHelper, because we want to catch wrongPassword before failure
    if (connectHelper != NULL) {
        wifiError(tr("Error conectando"));
        paintWifiList();
    }

}

void SettingsWiFiList::connected(void)
{
    qDebug() << Q_FUNC_INFO;
    finishConnecting();

    if ((m_behavior == NO_ACTION_BEHAVIOR || m_behavior == AUTOMATIC_CLOSE_BEHAVIOUR) && isVisible())
        QBookApp::instance()->popForm(this);
    if(m_behavior == BACK_BEHAVIOR && isVisible())
        setFocus();

}

void SettingsWiFiList::connectedNetwork(void)
{
    qDebug() << Q_FUNC_INFO;
}

void SettingsWiFiList::finishConnecting() {
    qDebug() << Q_FUNC_INFO;

    connectionRequested = false;

    disconnect(&connectingTimer, SIGNAL(timeout()), this, SLOT(connectionTimeout()));

    disconnect(QBookApp::instance(), SIGNAL(wrongPassword(QString)), this, SLOT(wrongPassword(QString)));
    disconnect(QBookApp::instance(), SIGNAL(cancelPassword()), this, SLOT(cancelPassword()));
    disconnect(QBookApp::instance(), SIGNAL(connectionFailed(QString, QString)), this, SLOT(connectionFailed(QString, QString)));

    if (connectingService != NULL) {
    connectingService->deleteLater();
    connectingService = NULL;
    }

    if(m_infoDialog->isVisible())
        m_infoDialog->hide();

    if (connectHelper != NULL && connectHelper->state() != QProcess::NotRunning) {
        qDebug() << Q_FUNC_INFO << "Terminating connectHelper process";
        disconnect(connectHelper, SIGNAL(readyReadStandardError()), this, SLOT(connectHelperOutput()));
        disconnect(connectHelper, SIGNAL(readyReadStandardOutput()), this, SLOT(connectHelperOutput()));
        disconnect(connectHelper, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(connectHelperFinished(int,QProcess::ExitStatus)));
        connectHelper->terminate();
        connectHelper->waitForFinished(100);
        if (connectHelper->state() != QProcess::NotRunning)
            connectHelper->kill();
        delete connectHelper;
        connectHelper = NULL;
    }
}


void SettingsWiFiList::wifiError(QString text) {
    qDebug() << Q_FUNC_INFO << text;

    qApp->processEvents();
    finishConnecting();
    setFocus();
}

void SettingsWiFiList::wrongPassword(QString path) {
    qDebug() << Q_FUNC_INFO << path;
    m_infoDialog->setText(tr("Password incorrecto"));
    m_infoDialog->setTimeLasts(2000);

    Screen::getInstance()->queueUpdates();
    Screen::getInstance()->setMode(Screen::MODE_QUICK,true,FLAG_PARTIALSCREEN_UPDATE,Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
    Screen::getInstance()->resetQueue();
    m_infoDialog->hideSpinner();
    m_infoDialog->showForSpecifiedTime();

    m_infoDialog->accept();
    wifiError(tr("Password incorrecto"));

    QCoreApplication::processEvents();
    QCoreApplication::flush();
    setFocus();
    paintWifiList();
}

void SettingsWiFiList::cancelPassword(void) {
    qDebug() << Q_FUNC_INFO;
    finishConnecting();

    QCoreApplication::processEvents();
    QCoreApplication::flush();
    setFocus();
    paintWifiList();
}

void SettingsWiFiList::handleWifiLongPressed(Network network){
    qDebug() << Q_FUNC_INFO;

    if(dialog && dialog->isVisible()) {
        hideAddNetworkDialog();
        return;
    }

    QString path = network.getPath();
    SettingsWiFiDetail *wifiSetting = new SettingsWiFiDetail(path, (QBookForm*) parent());
    Screen::getInstance()->queueUpdates();

    connect(wifiSetting, SIGNAL(disconnectWifiSignal(QString)), this, SLOT(disconnectWifiRequest(QString)));
    connect(wifiSetting, SIGNAL(forgetWifiSignal(QString)), this, SLOT(forgetWifiRequest(QString)));
    connect(wifiSetting, SIGNAL(connectWifiSignal(QString)), this, SLOT(wifiSelected(QString)));

    b_showWifiSetting = true;
    wifiSetting->move(0, QBookApp::instance()->getStatusBar()->height());
    QBookApp::instance()->pushForm(wifiSetting);
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}



void SettingsWiFiList::connectionFailed(QString path, QString error) {
    qDebug() << Q_FUNC_INFO << path << error;
    wifiError(tr("Error conectando"));
    paintWifiList();


}

void SettingsWiFiList::backButtonPressed()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->popForm(this);
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
    emit wifiConnectionCanceled();

}

void SettingsWiFiList::disconnectWifi(QString path, bool keepPowered)
{
    qDebug() << Q_FUNC_INFO << path << " keeping powered = " << keepPowered;
    // We don't want to notify QBookApp about this disconnection

    showDisconnectingMessage();

    manager->disconnectFrom(path);
}

void SettingsWiFiList::forgetWifi(QString path, bool keepPowered)
{
    qDebug() << Q_FUNC_INFO << path << " keepPowered = " << keepPowered;
    // We don't want to notify QBookApp about this disconnection

    manager->remove(path);
    paintWifiList();
}

void SettingsWiFiList::powerOnOffWifi ()
{
    qDebug() << Q_FUNC_INFO << ": setting power to " << QBook::settings().value("wifi/disabled",false).toBool();
    if(!QBook::settings().value("wifi/disabled",false).toBool())
        powerOffWifi();
    else
        powerOnWifi();
}

void SettingsWiFiList::powerOffWifi()
{
    qDebug() << Q_FUNC_INFO << "Power Off wifi";
    m_powerLock->release();
    setUiPowerOnOff(false);
    Screen::getInstance()->resetQueue();
    QBook::settings().setValue("wifi/disabled",true);
    QBook::settings().sync();
    // We don't want to notify QBookApp about this disconnection
    m_infoDialog->hide();

    QBookApp::instance()->finishedResumingWifi();
    manager->setOffline();
    manager->powerWifi(false);
    Wifi::getInstance()->powerOff();
    m_available_wifis.clear();
    setupWifis(m_available_wifis);
}

void SettingsWiFiList::powerOnWifi()
{
    qDebug() << Q_FUNC_INFO << "Power on wifi";
    m_powerLock->activate();
    setUiPowerOnOff(true);
    QBookApp::instance()->getStatusBar()->setSpinner(true);
    QBook::settings().setValue("wifi/disabled",false);
    QBook::settings().sync();
    Wifi::getInstance()->powerOn();
    QBookApp::instance()->setPoweringOff(false);
    manager->powerWifi(true);
    manager->setOnline();
    if (deleted) {
        qDebug() << "Form was destructed (user pressing back?) not doing the scan/update wifi list";
        return;
    }
    paintWifiList();
    QBookApp::instance()->getStatusBar()->setSpinner(false);

}

void SettingsWiFiList::setUiPowerOnOff(bool powerOn)
{
    qDebug() << Q_FUNC_INFO << powerOn;

    if(!powerOn)
    {
        Screen::getInstance()->queueUpdates();
        wifiStatusBtn->setChecked(false);
        if(dialog && dialog->isVisible())
            hideAddNetworkDialog();
        wifiStatusBtn->setStyleSheet("background-image: url(:res/off_btn.png);");
        wifiConnectMsgLbl->setText(tr("Turned OFF"));
        Screen::getInstance()->resetQueue();
        Screen::getInstance()->setMode(Screen::MODE_QUICK, true, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
        updateWifiListBtn->hide();
        addNetworksBtn->hide();
        QBookApp::instance()->getStatusBar()->setWifiStatus(StatusBar::WIFI_OFF);
    }
    else
    {
        Screen::getInstance()->queueUpdates();
        wifiStatusBtn->setChecked(true);
        wifiStatusBtn->setStyleSheet("background-image: url(:/res/on_btn.png);");
        if(manager->isConnected())
            wifiConnectMsgLbl->setText(tr("Connected"));
        else
            wifiConnectMsgLbl->setText(tr("Disconnected"));
        Screen::getInstance()->resetQueue();
        Screen::getInstance()->setMode(Screen::MODE_QUICK, true, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
        updateWifiListBtn->show();
        addNetworksBtn->show();
    }
}

void SettingsWiFiList::setBehaviour(eBehavior behavior)
{
    qDebug() << Q_FUNC_INFO << behavior;
    m_behavior = behavior;
    disconnect(backBtn);
    disconnect(this, SIGNAL(hideMe()));
    switch(m_behavior)
    {
    case NO_ACTION_BEHAVIOR:
        backBtn->hide();
        break;
    case BACK_BEHAVIOR:
        connect(this, SIGNAL(hideMe()), this, SLOT(backButtonPressed()), Qt::UniqueConnection);
        connect(backBtn,SIGNAL(clicked()),this,SLOT(backButtonPressed()), Qt::UniqueConnection);
        backBtn->setText(tr("Back"));
        backBtn->show();
        break;
    case CLOSE_BEHAVIOR:
    case AUTOMATIC_CLOSE_BEHAVIOUR:
    default:
        connect(this, SIGNAL(hideMe()), this, SLOT(backButtonPressed()), Qt::UniqueConnection);
        connect(backBtn,SIGNAL(clicked()),this,SLOT(backButtonPressed()), Qt::UniqueConnection);
        backBtn->setText(tr("Close"));
        backBtn->show();
        break;
    }
}

bool SettingsWiFiList::isPowerOn () const
{
    qDebug() << Q_FUNC_INFO;
    return !QBook::settings().value("wifi/disabled",false).toBool();
}

void SettingsWiFiList::emulateLost()
{
    qDebug() << Q_FUNC_INFO;
    //disconnectWifi();
    QBookApp::instance()->lostConnection();
}

void SettingsWiFiList::noWifisAvailable()
{
        qDebug() << Q_FUNC_INFO;

        m_infoDialog->setText(tr("No se encontraron redes"));
        m_infoDialog->hideSpinner();
        m_infoDialog->showForSpecifiedTime();

        qApp->processEvents();
}

void SettingsWiFiList::disconnectWifiRequest (QString path)
{
        qDebug() << Q_FUNC_INFO << path;
        disconnectWifi(path, true);
        paintWifiList();
}
void SettingsWiFiList::forgetWifiRequest (QString path)
{
        qDebug() << Q_FUNC_INFO << path;
        forgetWifi(path, true);
        paintWifiList();
}

void SettingsWiFiList::handleDisconnected(){
        paintWifiList();
}
void SettingsWiFiList::handleConnected(){
        paintWifiList();
}

void SettingsWiFiList::handleWifiSelected(Network network){
	QString path = network.getPath();
    if(dialog && dialog->isVisible()) {
        hideAddNetworkDialog();
        return;
    }
        if (network.isConnected()) {
		handleWifiLongPressed(network);
	} else {
	    if (!connectionRequested) {
                QBookApp::instance()->getStatusBar()->setWifiStatus(StatusBar::WIFI_PROCESSING);
                wifiConnectMsgLbl->setText(tr("Connecting..."));
	    }
	    wifiSelected(path);
       }
}


void SettingsWiFiList::handleSomeThingHasChanged(){
        paintWifiList();

}

void SettingsWiFiList::resetSleepTimer()
{
        if(QBookApp::instance()->getCurrentForm() != this)
            return;

        qDebug() << "--->" << Q_FUNC_INFO;

        if(i_timeSleepMSecs == 0)
            i_timeSleepMSecs = QBook::settings().value("setting/sleepTimeInSecs", POWERMANAGER_TIME_AUTOSLEEPSECS).toInt() * 1000;

        m_sleepTimer.start(i_timeSleepMSecs,this);
}

void SettingsWiFiList::timerEvent(QTimerEvent *event)
{
        qDebug() << Q_FUNC_INFO;
        if(event->timerId() == m_sleepTimer.timerId()){
            event->accept();
            m_sleepTimer.stop();
            QBookApp::instance()->goToSleep();
            return;
        }

        QBookForm::timerEvent(event);
}

void SettingsWiFiList::setupWifis (QList<Network> _networks){
        qDebug() << "--->" << Q_FUNC_INFO << _networks.size() << "available wifis" << isVisible();
        networks = _networks;

        page = 0;

        int totalPages = networks.size() / wifis.size();
        if(networks.size() % wifis.size() != 0)
            totalPages++;

        settingsPagerCont->setup(totalPages);
        updateList();
}

void SettingsWiFiList::updateList ()
{
        qDebug() << Q_FUNC_INFO;
        const int wifisSize = wifis.size();
        const int posAux = page * wifisSize;
        const int availableWifisSize = (networks.size() > 0) ? networks.size() : 0;

        for(int i = 0; i < wifisSize; ++i)
        {
            int pos = posAux + i;
            if(pos < availableWifisSize)
            {
                SettingsWiFiListItem* wifi = wifis.at(i);
                Network network = networks.at(pos);

                wifi->setWiFi(network);
                wifi->paint();

                wifi->show();
            } else
                wifis.at(i)->hideLabels();
        }
}

void SettingsWiFiList::hideAddNetworkDialog(){
        qDebug() << Q_FUNC_INFO;

        delete edit;
        edit = NULL;
        delete dialog;
        dialog = NULL;

        Screen::getInstance()->queueUpdates();
        Keyboard *keyboard = QBookApp::instance()->getKeyboard();
        keyboard->hide();
        Screen::getInstance()->setMode(Screen::MODE_QUICK, true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
}

void SettingsWiFiList::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }

void SettingsWiFiList::forward ()
{
        qDebug() << Q_FUNC_INFO;
        if(dialog && dialog->isVisible())
            hideAddNetworkDialog();

        page++;
        updateList();

}

void SettingsWiFiList::back()
{
        qDebug() << Q_FUNC_INFO;
        if(dialog && dialog->isVisible())
            hideAddNetworkDialog();

        page--;

        if(page < 0)
            page = 0;

        updateList();
}
