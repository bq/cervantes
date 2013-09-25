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

#include <QWidget>
#include <QSettings>
#include <QTime>
#include <QBasicTimer>
#include <QTimerEvent>
#include <QPainter>

#include "StatusBar.h"
#include "Viewer.h"
#include "InfoDialog.h"
#include "QBook.h"
#include "Battery.h"
#include "ADConverter.h"
#include "Screen.h"
#include "QBookApp.h"
#include "DeviceInfo.h"
#include "FrontLight.h"
#include "PowerManagerDefs.h"

#define UPDATE_TIME_INTERVAL 10000
#define STATUSBAR_MAX_LENGTH 28
#define STATUSBAR_BUSY_INTERVAL 500

StatusBar::StatusBar(QWidget *parent) :
    QWidget(parent)
  , lastLevel(CHARGING)
  , m_dialog(NULL)
{
        setupUi(this);

        // battery
        connect(Battery::getInstance(), SIGNAL(batteryLevelChanged(int)), this, SLOT(setBatteryLevel(int)));
        connect(Battery::getInstance(), SIGNAL(batteryLevelLow()), this,SLOT(updateBatteryWarning()));

        // usb + charger
        connect(ADConverter::getInstance(), SIGNAL(pcPresentStatusChange(bool)), this, SLOT(updateUSBState(bool)));
        connect(ADConverter::getInstance(), SIGNAL(chargerStatusChange(bool)), this, SLOT(updateChargerState(bool)));

        //frontLight
        connect(FrontLight::getInstance(), SIGNAL(frontLightPower(bool)), this, SLOT(switchLight(bool)));

        m_timer_busy.setInterval(STATUSBAR_BUSY_INTERVAL);
        connect(&m_timer_busy,SIGNAL(timeout()),this,SLOT(updateBusyIcon()));

        connect(currentReadBtn,SIGNAL(clicked()),this,SLOT(openLastBook()), Qt::UniqueConnection);
        connect(wifiBtn, SIGNAL(clicked()), this, SLOT(wifiAndLightPopup()), Qt::UniqueConnection);
        connect(lightBtn, SIGNAL(clicked()), this, SLOT(wifiAndLightPopup()), Qt::UniqueConnection);
        connect(batteryLbl, SIGNAL(clicked()), this, SLOT(showBatteryDialog()));

        usbConnectedLbl->hide();

        qDebug() << Q_FUNC_INFO << ADConverter::getInstance()->getStatus();

        setBatteryLevel(Battery::getInstance()->getLevel());
        updateChargerState(ADConverter::getInstance()->getStatus() != ADConverter::ADC_STATUS_NO_WIRE);

        m_currentStatus = WIFI_PROCESSING; // Different from off to enter setwifistatus
        setWifiStatus(WIFI_OFF);
        setIconVisible(ICON_SYSTEM,false);
        setMenuTitle(QString(tr("Home")));
        updateTime();

        initWorkingImages();

        QFile fileSpecific(":/res/ui_styles.qss");
        QFile fileCommons(":/res/ui_styles_generic.qss");
        fileSpecific.open(QFile::ReadOnly);
        fileCommons.open(QFile::ReadOnly);

        QString styles = QLatin1String(fileSpecific.readAll() + fileCommons.readAll());
        setStyleSheet(styles);


        if(!DeviceInfo::getInstance()->hasFrontLight())
            lightBtn->hide();

        syncSpinnerLbl->hide();
}

StatusBar::~StatusBar()
{
    // do nothing
    delete m_dialog;
    m_dialog = NULL;
}

void StatusBar::initWorkingImages()
{
    QPixmap image;
    int i = 1;
    while (image.load(QString(":res/working-%1.png").arg(i))) {
        workingImages.append(QString(":res/working-%1.png").arg(i));
        i++;
    }

    i_currentImg = 0;
}

void StatusBar::setMenuTitle ( const QString& title){
        currentSectionLbl->setText(title);
}

void StatusBar::setBatteryLevel(int level){
        qDebug() << "--->" << Q_FUNC_INFO << "Level " << level << "Status" << ADConverter::getInstance()->getStatus();

        qDebug() << "--->" << Q_FUNC_INFO << "status no wire";

        int image = getBatteryImage(level);

        if(lastLevel != image)
        {
            lastLevel = image;
            QString imageUrl;
            if(image == EMPTY_BAR)
                imageUrl = QString(":/res/battery_warning_Ico.png");
            else
                imageUrl = QString(":/res/battery_%1.png").arg(image);
            batteryLbl->setStyleSheet("background:url(" + imageUrl + ") no-repeat center left;");
        }
}

int StatusBar::getBatteryImage(int level)
{
    int enumLevel;

    if(level >= SIX_BAR_BATTERY_THRESHOLD_ELF)
        enumLevel = SEVEN_BAR;
    else if(FIVE_BAR_BATTERY_THRESHOLD_ELF <= level && level < SIX_BAR_BATTERY_THRESHOLD_ELF)
        enumLevel = SIX_BAR;
    else if(FOUR_BAR_BATTERY_THRESHOLD_ELF <= level && level < FIVE_BAR_BATTERY_THRESHOLD_ELF)
        enumLevel = FIVE_BAR;
    else if(THREE_BAR_BATTERY_THRESHOLD_ELF <= level && level < FOUR_BAR_BATTERY_THRESHOLD_ELF)
        enumLevel = FOUR_BAR;
    else if(TWO_BAR_BATTERY_THRESHOLD_ELF <= level && level < THREE_BAR_BATTERY_THRESHOLD_ELF)
        enumLevel = THREE_BAR;
    else if(ONE_BAR_BATTERY_THRESHOLD_ELF <= level && level < TWO_BAR_BATTERY_THRESHOLD_ELF)
        enumLevel = TWO_BAR;
    else if(EMPTY_BATTERY_THRESHOLD_ELF <= level && level < ONE_BAR_BATTERY_THRESHOLD_ELF)
        enumLevel = ONE_BAR;
    else if(EMPTY_BATTERY_THRESHOLD_ELF >= level)
        enumLevel = EMPTY_BAR;

    return enumLevel;
}

void StatusBar::setIconVisible(int icon,bool visible)
{
        qDebug() << Q_FUNC_INFO;

        if(isVisible()){
            Screen::getInstance()->setMode(Screen::MODE_QUICK,true,Q_FUNC_INFO);
        }

        QWidget* iconWidget = NULL;

        switch(icon)
        {

        case ICON_SYSTEM:
            if(visible)
                m_timer_busy.start();
            else
                m_timer_busy.stop();
            iconWidget = spinnerLbl;
            break;

        case BUTTON_READ:
            iconWidget = currentReadBtn;
            break;

        default:
            break;
        }

        if(visible)
            iconWidget->show();
        else
            iconWidget->hide();

}

void StatusBar::setBusy(bool busy)
{
    qDebug() << Q_FUNC_INFO;

    if(busy)
    {
        if(!m_dialog)
        {
            m_dialog = new InfoDialog(this, tr("Work in progress"));
            Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
            m_dialog->show();
            QCoreApplication::processEvents();
            disconnect(currentReadBtn,SIGNAL(clicked()),this,SLOT(openLastBook()));
            disconnect(wifiBtn, SIGNAL(clicked()), this, SLOT(wifiAndLightPopup()));
        }
    }
    else
    {
        qDebug() << Q_FUNC_INFO << busy;
        delete m_dialog;
        m_dialog = NULL;
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        connect(currentReadBtn,SIGNAL(clicked()),this,SLOT(openLastBook()), Qt::UniqueConnection);
        connect(wifiBtn, SIGNAL(clicked()), this, SLOT(wifiAndLightPopup()), Qt::UniqueConnection);
    }
}

void StatusBar::setSpinner(bool busy)
{
    if(busy)
    {
        setIconVisible(ICON_SYSTEM, true);
        disconnect(currentReadBtn,SIGNAL(clicked()),this,SLOT(openLastBook()));
        disconnect(wifiBtn, SIGNAL(clicked()), this, SLOT(wifiAndLightPopup()));
    }
    else
    {
       setIconVisible(ICON_SYSTEM, false);
       connect(currentReadBtn,SIGNAL(clicked()),this,SLOT(openLastBook()), Qt::UniqueConnection);
       connect(wifiBtn, SIGNAL(clicked()), this, SLOT(wifiAndLightPopup()), Qt::UniqueConnection);;
    }
}

void StatusBar::hideReadbutton()
{
        qDebug() << Q_FUNC_INFO;

        currentReadBtn->hide();
}

void StatusBar::hideWifiCont()
{
        qDebug() << Q_FUNC_INFO;

        wifiBtn->hide();
}

void StatusBar::showButtons()
{
        qDebug() << Q_FUNC_INFO;

        currentReadBtn->show();
        wifiBtn->show();
}

void StatusBar::updateBatteryWarning()
{
        qDebug() << Q_FUNC_INFO;
        // TODO handle warning icon for battery
}

void StatusBar::updateChargerState(bool value)
{
        qDebug() << Q_FUNC_INFO;
        qDebug() << "Charger value:" << value;

        if(isVisible())
            Screen::getInstance()->setMode(Screen::MODE_QUICK,true,FLAG_PARTIALSCREEN_UPDATE,Q_FUNC_INFO);

        if(value)
        {
            qDebug() << Q_FUNC_INFO << "Showing batteryPlug";
            lastLevel = CHARGING;
            disconnect(Battery::getInstance(), SIGNAL(batteryLevelChanged(int)), this, SLOT(setBatteryLevel(int)));
            batteryLbl->setStyleSheet("background:url(:res/battery_plug_Ico.png) no-repeat center left;");
        }else
        {
            qDebug() << Q_FUNC_INFO << "Hiding batteryPlug";
            connect(Battery::getInstance(), SIGNAL(batteryLevelChanged(int)), this, SLOT(setBatteryLevel(int)));
            setBatteryLevel(Battery::getInstance()->getLevel());
        }

}

void StatusBar::updateUSBState(bool value)
{
        qDebug() << Q_FUNC_INFO;

        Screen::getInstance()->queueUpdates();
        updateChargerState(value);
        Screen::getInstance()->flushUpdates();
}

void StatusBar::handleSyncStart()
{
        qDebug() << Q_FUNC_INFO;

        syncSpinnerLbl->show();
}

void StatusBar::handleSyncEnd()
{
        qDebug() << Q_FUNC_INFO;

        syncSpinnerLbl->hide();
}

void StatusBar::updateLinkPcState(bool value)
{
        qDebug() << Q_FUNC_INFO;

        usbConnectedLbl->setVisible(value);
        if(value)
            QBookApp::instance()->goToHome();
}

void StatusBar::updateTime()
{
#ifdef Q_WS_QWS
        qDebug() << Q_FUNC_INFO;
#endif

        int timeMode = QBook::settings().value("setting/timeMode", 12).toInt();
        QString time = QDateTime::currentDateTime().toString("dd/MM, hh:mm");

        Screen::getInstance()->setMode(Screen::MODE_QUICK,true,Q_FUNC_INFO);
        updateTimeUI(time);
}

void StatusBar::updateTimeUI (const QString& timeStr)
{
#ifdef Q_WS_QWS
        qDebug() << Q_FUNC_INFO;
#endif

        timeLbl->setText(timeStr);
}

void StatusBar::wifiAndLightPopup()
{
        qDebug() << Q_FUNC_INFO;
        if(DeviceInfo::getInstance()->hasFrontLight())
            QBookApp::instance()->showQuickSettingsPopup();
        else
            QBookApp::instance()->showWifiSelection(SettingsWiFiList::CLOSE_BEHAVIOR);

}

void StatusBar::openLastBook(void)
{
        qDebug() << Q_FUNC_INFO;
        emit wifiConnectionCanceled();

        emit openLastContent();
}

void StatusBar::setVisible(bool visible)
{
        qDebug() << Q_FUNC_INFO;

        QWidget::setVisible(visible);
        if(visible)
        {
            m_timer.start(UPDATE_TIME_INTERVAL, this);
            updateTime();
        }
        else
        {
            m_timer.stop();
        }
}

void StatusBar::timerEvent(QTimerEvent *event)
{
        if (event->timerId() == m_timer.timerId()) {
            updateTime();
            event->accept();
        }
}

void StatusBar::setWifiStatus(WifiStatusEnum status)
{
        qDebug() << Q_FUNC_INFO;

        if(status == m_currentStatus)
            return;
        else
            m_currentStatus = status;

        Screen::getInstance()->queueUpdates();
        Screen::getInstance()->setMode(Screen::MODE_QUICK, true, FLAG_PARTIALSCREEN_UPDATE,Q_FUNC_INFO);
        switch(status)
        {
        case WIFI_PROCESSING:
            wifiBtn->setStyleSheet("background:url(:/res/wifi_connecting_Ico.png) no-repeat center left;");
            break;
        case WIFI_DISCONNECTED:
            wifiBtn->setStyleSheet("background:url(:/res/wifi_no_connected.png) no-repeat center left;");
            break;
        case WIFI_LEVEL_1:
            wifiBtn->setStyleSheet("background:url(:/res/wifi_1_Ico.png) no-repeat center left;");
            break;
        case WIFI_LEVEL_2:
            wifiBtn->setStyleSheet("background:url(:/res/wifi_2_Ico.png) no-repeat center left;");
            break;
        case WIFI_LEVEL_3:
            wifiBtn->setStyleSheet("background:url(:/res/wifi_3_Ico.png) no-repeat center left;");
            break;
        case WIFI_LEVEL_4:
            wifiBtn->setStyleSheet("background:url(:/res/wifi_4_Ico.png) no-repeat center left;");
            break;
        case WIFI_OFF:
            wifiBtn->setStyleSheet("background:url(:/res/wifi_off_ico.png) no-repeat center left;");
            break;
        }

        Screen::getInstance()->flushUpdates();
}

void StatusBar::updateBusyIcon()
{
        qDebug() << Q_FUNC_INFO;

        i_currentImg++;
        i_currentImg %= workingImages.size();
        QString imgFile = workingImages.at(i_currentImg);
        Screen::getInstance()->setMode(Screen::MODE_QUICK,true,Q_FUNC_INFO);
        spinnerLbl->setStyleSheet("background-image:url("+imgFile+");");
}

void StatusBar::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }

void StatusBar::switchLight(bool on)
{
        qDebug() << Q_FUNC_INFO;

        if(on)
            lightBtn->setStyleSheet("background-image:url(:/res/light_on_Ico.png);");
        else
            lightBtn->setStyleSheet("background-image:url(:/res/light_off_Ico.png);");
}

void StatusBar::showBatteryDialog()
{
        InfoDialog* batteryLevel = new InfoDialog(this, tr("Battery level = %1").arg(Battery::getInstance()->getLevel()));
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, Q_FUNC_INFO);
        batteryLevel->hideSpinner();
        batteryLevel->showForSpecifiedTime();
        delete batteryLevel;
}
