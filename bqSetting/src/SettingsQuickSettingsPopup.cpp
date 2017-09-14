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

#include "SettingsQuickSettingsPopup.h"
#include "SettingsWiFiDetail.h"
#include "QBookApp.h"
#include "QBookForm.h"
#include "InfoDialog.h"
#include "PowerManager.h"
#include "Screen.h"
#include "Wifi.h"
#include "Keyboard.h"
#include "FrontLight.h"
#include "DeviceInfo.h"
#include <QPainter>

SettingsQuickSettingsPopup::SettingsQuickSettingsPopup(QWidget *parent) : QWidget(parent)
{
        qDebug() << Q_FUNC_INFO;

        setupUi(this);
        m_powerLock = PowerManager::getNewLock(this);

        connect(closePopupBtn, SIGNAL(clicked()), this, SLOT(handleClosePopupBtn()));
        connect(wifiStatusBtn, SIGNAL(clicked()), this, SLOT(handleWifiStatusBtn()));

        connect(screenLightStatusBtn, SIGNAL(clicked()), this, SLOT(handleScreenLightStatusBtn()));

        // Brightness connections
        connect(brightnessMinBtn, SIGNAL(pressed()), intensityLightCont, SLOT(decreaseItemSelected()));
        connect(brightnessMinBtn, SIGNAL(longPressed()), intensityLightCont, SLOT(selectMinItem()));
        connect(brightnessMaxBtn, SIGNAL(pressed()), intensityLightCont, SLOT(increaseItemSelected()));
        connect(brightnessMaxBtn, SIGNAL(longPressed()), intensityLightCont, SLOT(selectMaxItem()));
        connect(brightnessRestBtn, SIGNAL(pressed()), intensityLightCont, SLOT(decreaseItemSelected()));
        connect(brightnessRestBtn, SIGNAL(longPressed()), intensityLightCont, SLOT(selectMinItem()));
        connect(brightnessSumBtn, SIGNAL(pressed()), intensityLightCont, SLOT(increaseItemSelected()));
        connect(brightnessSumBtn, SIGNAL(longPressed()), intensityLightCont, SLOT(selectMaxItem()));

        // OptimaLight connections
        connect(colourAutoBtn, SIGNAL(clicked()), this, SLOT(handleOptimaLightAutoBtn()));
        connect(colourMinBtn, SIGNAL(pressed()), colourTemperatureCont, SLOT(decreaseItemSelected()));
        connect(colourMinBtn, SIGNAL(longPressed()), colourTemperatureCont, SLOT(selectMinItem()));
        connect(colourMaxBtn, SIGNAL(pressed()), colourTemperatureCont, SLOT(increaseItemSelected()));
        connect(colourMaxBtn, SIGNAL(longPressed()), colourTemperatureCont, SLOT(selectMaxItem()));
        connect(optimaSleepTimeBtn, SIGNAL(clicked()),this,SLOT(handleClosePopupBtn()));
        connect(optimaSleepTimeBtn, SIGNAL(clicked()),SIGNAL(lighSettingsConf()));

        connect(FrontLight::getInstance(), SIGNAL(frontLightPower(bool)), this, SLOT(paintLight(bool)));
        connect(QBookApp::instance(), SIGNAL(startSleep()), this, SLOT(handleClosePopupBtn()));

        QFile fileSpecific(":/res/settings_styles.qss");
        QFile fileCommons(":/res/settings_styles_generic.qss");
        fileSpecific.open(QFile::ReadOnly);
        fileCommons.open(QFile::ReadOnly);

        QString styles = QLatin1String(fileSpecific.readAll() + fileCommons.readAll());
        setStyleSheet(styles);
}

SettingsQuickSettingsPopup::~SettingsQuickSettingsPopup()
{
        qDebug() << Q_FUNC_INFO;
}

void SettingsQuickSettingsPopup::showEvent ( QShowEvent * )
{
    qDebug() << Q_FUNC_INFO;

    paintWifi(!QBook::settings().value("wifi/disabled",true).toBool());
    paintLight(FrontLight::getInstance()->isFrontLightActive());
    setFocus(Qt::ActiveWindowFocusReason);
}

void SettingsQuickSettingsPopup::focusInEvent ( QFocusEvent * )
{
    qDebug() << Q_FUNC_INFO;

    paintWifi(!QBook::settings().value("wifi/disabled",true).toBool());
    paintLight(FrontLight::getInstance()->isFrontLightActive());
    setFocus(Qt::ActiveWindowFocusReason);
}

void SettingsQuickSettingsPopup::handleClosePopupBtn()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    this->hide();
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void SettingsQuickSettingsPopup::handleScreenLightStatusBtn()
{
    qDebug() << Q_FUNC_INFO;

    bool current = FrontLight::getInstance()->isFrontLightActive();
    int brightness = FrontLight::getInstance()->getBrightness();
    if(!current)
        FrontLight::getInstance()->setBrightness(brightness);
    FrontLight::getInstance()->setFrontLightActive(!current);
}

void SettingsQuickSettingsPopup::handleOptimaLightAutoBtn()
{
    qDebug() << Q_FUNC_INFO;

    bool current = FrontLight::getInstance()->isOptimaLightAutoActive();
    FrontLight::getInstance()->setOptimaLightAutoMode(!current);
    paintLight(FrontLight::getInstance()->isFrontLightActive());
}


void SettingsQuickSettingsPopup::keyReleaseEvent(QKeyEvent* event)
{
        qDebug() << Q_FUNC_INFO;
        handleClosePopupBtn();
        QWidget::keyReleaseEvent(event);
}

void SettingsQuickSettingsPopup::paintWifi(bool on)
{
    qDebug() << Q_FUNC_INFO;
    if(on)
    {
        Screen::getInstance()->queueUpdates();
        wifiStatusBtn->setChecked(true);
        wifiStatusBtn->setStyleSheet("background-image: url(:res/on_btn.png);");
        Screen::getInstance()->setMode(Screen::MODE_QUICK, true, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
    }
    else
    {
        Screen::getInstance()->queueUpdates();
        wifiStatusBtn->setChecked(false);
        wifiStatusBtn->setStyleSheet("background-image: url(:res/off_btn.png);");
        Screen::getInstance()->setMode(Screen::MODE_QUICK, true, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
    }
}

void SettingsQuickSettingsPopup::paintLight(bool lightOn)
{
    qDebug() << Q_FUNC_INFO << "Light enabled" << lightOn;
    Screen::getInstance()->queueUpdates();

    setBrightnessMenuEnabled(lightOn);
    setOptimaLightMenuEnabled(lightOn);

    Screen::getInstance()->flushUpdates();
}

void SettingsQuickSettingsPopup::setBrightnessMenuEnabled(bool lightOn)
{
    qDebug() << Q_FUNC_INFO;

    if(lightOn)
    {
        screenLightStatusBtn->setChecked(true);
        screenLightStatusBtn->setStyleSheet("background-image: url(:res/on_btn.png);");
        brightnessCont->setStyleSheet("background-color:#FFF;");
        brightnessLbl->setStyleSheet("color:#000; background-color:#FFF;");
        intensityLightCont->switchOn();
    }
    else
    {
        screenLightStatusBtn->setChecked(false);
        screenLightStatusBtn->setStyleSheet("background-image: url(:res/off_btn.png);");
        brightnessCont->setStyleSheet("background-color:#E0E0E0;");
        brightnessLbl->setStyleSheet("color:#707070; background-color:#E0E0E0;");
        intensityLightCont->switchOff();
    }
}

void SettingsQuickSettingsPopup::setOptimaLightMenuEnabled(bool lightOn)
{
    qDebug() << Q_FUNC_INFO;

    if(!DeviceInfo::getInstance()->hasOptimaLight())
        optimaLightCont->setVisible(false);

    QString optimaLightTimes(tr("Start Time - ")
                             + FrontLight::getInstance()->getOptimaLightSunsetTime().toString("hh:mm")
                             + tr(" End Time - ") + FrontLight::getInstance()->getOptimaLightSunriseTime().toString("hh:mm"));
    optimaSleepTimeBtn->setText(optimaLightTimes);

    // Enable bottom container when wifi is shown (menus)
    if(wifiCont->isVisible())
        quickSettingsBottomCont->setEnabled(true);

    // Set general light on/off depending settings
    if(lightOn){
        optimaLightCont->setEnabled(true);

        // Enable bottom container when wifi is NOT shown (viewer)
        if(!wifiCont->isVisible())
            quickSettingsBottomCont->setEnabled(true);

        // Set optimaLight auto mode settings
        if(FrontLight::getInstance()->isOptimaLightAutoActive()){
            colourAutoBtn->setChecked(true);
            colourAutoBtn->setStyleSheet("background-image: url(:res/on_btn.png);");
            colourTemperatureCont->switchOff();
            sleepTimeCont->setEnabled(true);
        }
        else{
            colourAutoBtn->setChecked(false);
            colourAutoBtn->setStyleSheet("background-image: url(:res/off_btn.png);");
            colourTemperatureCont->switchOn();
            sleepTimeCont->setEnabled(false);
        }


    }
    else{
        colourTemperatureCont->switchOff();
        optimaLightCont->setEnabled(false);
        if(!wifiCont->isVisible()) // Disable bottom container when wifi NOT shown (viewer)
            quickSettingsBottomCont->setEnabled(false);
    }
}

void SettingsQuickSettingsPopup::handleWifiStatusBtn()
{
    qDebug() << Q_FUNC_INFO;

    if(QBook::settings().value("wifi/disabled",true).toBool())
    {
        paintWifi(true);
        powerWifi(true);

    }
    else
    {
        paintWifi(false);
        powerWifi(false);
    }

}

void SettingsQuickSettingsPopup::paintEvent (QPaintEvent *)
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void SettingsQuickSettingsPopup::enabledWifi(bool on)
{
    wifiCont->setVisible(on);
}

void SettingsQuickSettingsPopup::powerWifi(bool on)
{
    qDebug() << Q_FUNC_INFO;

    if(on)
    {
        QBook::settings().setValue("wifi/disabled",false);
        QBookApp::instance()->startedResumingWifi();
        Wifi::getInstance()->powerOn();
        QBookApp::instance()->setPoweringOff(false);
        ConnectionManager::getInstance()->powerWifi(true);
        ConnectionManager::getInstance()->setOnline();
        QBookApp::instance()->finishedResumingWifi();
    }
    else
    {
        QBookApp::instance()->finishedResumingWifi();
        ConnectionManager::getInstance()->setOffline();
        ConnectionManager::getInstance()->powerWifi(false);
        Wifi::getInstance()->powerOff();
        QBook::settings().setValue("wifi/disabled",true);
    }
}

void SettingsQuickSettingsPopup::mouseReleaseEvent(QMouseEvent* event)
{
        qDebug() << Q_FUNC_INFO;

        int yMin = QuickSettingsCont->y();
        //Remove the height of the vertical spacer.
        int yMax = yMin + headerPopupCont->height() + contentPopup->height();

        //Hide popUp only when press outside.
        if(yMin > event->y() || yMax < event->y())
        {
            handleClosePopupBtn();
            event->accept();
        }
}
