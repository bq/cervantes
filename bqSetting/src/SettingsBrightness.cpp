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

#include "SettingsBrightness.h"
#include "QBook.h"
#include "SelectionDialog.h"
#include "InfoDialog.h"
#include "FrontLight.h"
#include "Screen.h"
#include "DeviceInfo.h"

#include <QDebug>
#include <QTimer>
#include <QKeyEvent>
#include <QPainter>
#include <QButtonGroup>

SettingsBrightness::SettingsBrightness(QWidget *parent) : FullScreenWidget(parent)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);

    connect(backBtn,SIGNAL(clicked()), this, SIGNAL(hideMe()));
    connect(screenLightStatusBtn, SIGNAL(clicked()), this, SLOT(handleScreenLightStatusBtn()));
    connect(brightnessMinBtn, SIGNAL(pressed()), intensityLightCont, SLOT(decreaseItemSelected()));
    connect(brightnessMinBtn, SIGNAL(longPressed()), intensityLightCont, SLOT(selectMinItem()));
    connect(brightnessMaxBtn, SIGNAL(pressed()), intensityLightCont, SLOT(increaseItemSelected()));
    connect(brightnessMaxBtn, SIGNAL(longPressed()), intensityLightCont, SLOT(selectMaxItem()));
    connect(brightnessRestBtn, SIGNAL(pressed()), intensityLightCont, SLOT(decreaseItemSelected()));
    connect(brightnessRestBtn, SIGNAL(longPressed()), intensityLightCont, SLOT(selectMinItem()));
    connect(brightnessSumBtn, SIGNAL(pressed()), intensityLightCont, SLOT(increaseItemSelected()));
    connect(brightnessSumBtn, SIGNAL(longPressed()), intensityLightCont, SLOT(selectMaxItem()));
    connect(FrontLight::getInstance(), SIGNAL(frontLightPower(bool)), this, SLOT(paintLight(bool)));

    if(!DeviceInfo::getInstance()->hasOptimaLight()){
        optimaLightCont->setVisible(false);
    }
    else{

        optimaLightCont->setVisible(true);

        // OptimaLight connections
        connect(colourAutoBtn, SIGNAL(clicked()), this, SLOT(handleOptimaLightAutoBtn()));
        connect(colourMinBtn, SIGNAL(pressed()), colourTemperatureCont, SLOT(decreaseItemSelected()));
        connect(colourMinBtn, SIGNAL(longPressed()), colourTemperatureCont, SLOT(selectMinItem()));
        connect(colourMaxBtn, SIGNAL(pressed()), colourTemperatureCont, SLOT(increaseItemSelected()));
        connect(colourMaxBtn, SIGNAL(longPressed()), colourTemperatureCont, SLOT(selectMaxItem()));

        // Setup OptimaLight auto mode buttons
        m_buttonGroupSunsetTime = new QButtonGroup(this);
        m_buttonGroupSunsetTime->addButton(optimaStartTime00, 0);
        m_buttonGroupSunsetTime->addButton(optimaStartTime01, 1);
        m_buttonGroupSunsetTime->addButton(optimaStartTime02, 2);
        m_buttonGroupSunsetTime->addButton(optimaStartTime03, 3);
        m_buttonGroupSunsetTime->addButton(optimaStartTime04, 4);
        m_buttonGroupSunsetTime->addButton(optimaStartTime05, 5);
        m_buttonGroupSunsetTime->addButton(optimaStartTime06, 6);
        connect(m_buttonGroupSunsetTime, SIGNAL(buttonClicked(int)),FrontLight::getInstance(), SLOT(setOptimaLightSunset(int)));

        m_buttonGroupSunriseTime = new QButtonGroup(this);
        m_buttonGroupSunriseTime->addButton(optimaEndTime00, 0);
        m_buttonGroupSunriseTime->addButton(optimaEndTime01, 1);
        m_buttonGroupSunriseTime->addButton(optimaEndTime02, 2);
        m_buttonGroupSunriseTime->addButton(optimaEndTime03, 3);
        m_buttonGroupSunriseTime->addButton(optimaEndTime04, 4);
        m_buttonGroupSunriseTime->addButton(optimaEndTime05, 5);
        m_buttonGroupSunriseTime->addButton(optimaEndTime06, 6);
        connect(m_buttonGroupSunriseTime, SIGNAL(buttonClicked(int)),FrontLight::getInstance(), SLOT(setOptimaLightSunrise(int)));

        m_buttonGroupSunriseTime->button(FrontLight::getInstance()->getOptimaLightSunriseId())->setChecked(true);
        m_buttonGroupSunsetTime->button(FrontLight::getInstance()->getOptimaLightSunsetId())->setChecked(true);

        connect(FrontLight::getInstance(),SIGNAL(frontLightChanged(bool)),this, SLOT(paintLight(bool)));
    }
}

SettingsBrightness::~SettingsBrightness()
{
    qDebug() << Q_FUNC_INFO;
}

void SettingsBrightness::paintLight(bool lightOn)
{
    qDebug() << Q_FUNC_INFO << "Light enabled" << lightOn;
    Screen::getInstance()->queueUpdates();

    setBrightnessMenuEnabled(lightOn);
    if(DeviceInfo::getInstance()->hasOptimaLight())
        setOptimaLightMenuEnabled(lightOn);

    Screen::getInstance()->flushUpdates();
}

void SettingsBrightness::setBrightnessMenuEnabled(bool lightOn)
{
    if(lightOn)
    {
        screenLightStatusBtn->setChecked(true);
        screenLightStatusBtn->setStyleSheet("background-image: url(:res/on_btn.png);");
        brightnessLbl->setEnabled(true);
        brightnessCont->setStyleSheet("background-color:#FFF;");
        intensityLightCont->switchOn();
    }
    else
    {
        screenLightStatusBtn->setChecked(false);
        screenLightStatusBtn->setStyleSheet("background-image: url(:res/off_btn.png);");
        brightnessLbl->setEnabled(false);
        brightnessCont->setStyleSheet("background-color:#E0E0E0;");
        intensityLightCont->switchOff();
    }
}

void SettingsBrightness::handleScreenLightStatusBtn()
{
    qDebug() << Q_FUNC_INFO;

    bool current = FrontLight::getInstance()->isFrontLightActive();
    int brightness = FrontLight::getInstance()->getBrightness();
    if(!current)
        FrontLight::getInstance()->setBrightness(brightness);
    FrontLight::getInstance()->setFrontLightActive(!current);
}

void SettingsBrightness::paintEvent(QPaintEvent*)
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

// NOTE: Top widget should have Strong Focus policy
void SettingsBrightness::showEvent ( QShowEvent * )
{
    qDebug() << Q_FUNC_INFO;
    paintLight(FrontLight::getInstance()->isFrontLightActive());
}

void SettingsBrightness::handleOptimaLightAutoBtn()
{
    qDebug() << Q_FUNC_INFO;

    bool current = FrontLight::getInstance()->isOptimaLightAutoActive();
    FrontLight::getInstance()->setOptimaLightAutoMode(!current);
    paintLight(FrontLight::getInstance()->isFrontLightActive());

}

void SettingsBrightness::setOptimaLightMenuEnabled(bool lightOn)
{
    qDebug() << Q_FUNC_INFO;

    // Set general light on/off depending settings
    if(lightOn){
        optimaLightCont->setEnabled(true);

        // Set optimaLight auto mode settings
        if(FrontLight::getInstance()->isOptimaLightAutoActive()){
            colourAutoBtn->setChecked(true);
            colourAutoBtn->setStyleSheet("background-image: url(:res/on_btn.png);");
        }
        else{
            colourAutoBtn->setChecked(false);
            colourAutoBtn->setStyleSheet("background-image: url(:res/off_btn.png);");
        }

        setOptimaLightAutoSettingsEnabled(FrontLight::getInstance()->isOptimaLightAutoActive());

        m_buttonGroupSunriseTime->button(FrontLight::getInstance()->getOptimaLightSunriseId())->setChecked(true);
        m_buttonGroupSunsetTime->button(FrontLight::getInstance()->getOptimaLightSunsetId())->setChecked(true);

    }
    else{
        colourTemperatureCont->switchOff();
        optimaLightCont->setEnabled(false);
    }
}

void SettingsBrightness::setOptimaLightAutoSettingsEnabled(bool autoModeOn)
{
    qDebug() << Q_FUNC_INFO << autoModeOn;

    if(autoModeOn){
        colourTemperatureCont->switchOff();
        optimaStartTime->setEnabled(true);
        optimaEndTime->setEnabled(true);
    }
    else{
        colourTemperatureCont->switchOn();
        optimaStartTime->setEnabled(false);
        optimaEndTime->setEnabled(false);
    }

}
