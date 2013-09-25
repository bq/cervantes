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

#include <QDebug>
#include <QTimer>
#include <QKeyEvent>
#include <QPainter>

SettingsBrightness::SettingsBrightness(QWidget *parent) : FullScreenWidget(parent)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);

    connect(backBtn,SIGNAL(clicked()), this, SIGNAL(hideMe()));
    connect(screenLightStatusBtn, SIGNAL(clicked()), this, SLOT(handleScreenLightStatusBtn()));
    connect(brightnessMinBtn, SIGNAL(pressed()), this, SLOT(handleBrightnessDecreasePressed()));
    connect(brightnessMinBtn, SIGNAL(longPressed()), this, SLOT(handleBrightnessDecreaseLongPressed()));
    connect(brightnessMaxBtn, SIGNAL(pressed()), this, SLOT(handleBrightnessIncreasePressed()));
    connect(brightnessMaxBtn, SIGNAL(longPressed()), this, SLOT(handleBrightnessIncreaseLongPressed()));
    connect(brightnessRestBtn, SIGNAL(pressed()), this, SLOT(handleBrightnessDecreasePressed()));
    connect(brightnessRestBtn, SIGNAL(longPressed()), this, SLOT(handleBrightnessDecreaseLongPressed()));
    connect(brightnessSumBtn, SIGNAL(pressed()), this, SLOT(handleBrightnessIncreasePressed()));
    connect(brightnessSumBtn, SIGNAL(longPressed()), this, SLOT(handleBrightnessIncreaseLongPressed()));
    connect(FrontLight::getInstance(), SIGNAL(frontLightPower(bool)), this, SLOT(paintLight(bool)));
}

SettingsBrightness::~SettingsBrightness()
{
    qDebug() << Q_FUNC_INFO;
}

void SettingsBrightness::paintLight(bool on)
{
    qDebug() << Q_FUNC_INFO << "Light enabled" << on;
    Screen::getInstance()->queueUpdates();
    if(on)
    {
        screenLightStatusBtn->setChecked(true);
        screenLightStatusBtn->setStyleSheet("background-image: url(:res/on_btn.png);");
//        brightnessCont->setEnabled(true);
        brightnessCont->setStyleSheet("background-color:#FFF;");
//        brightnessLbl->setStyleSheet("color:#000;");
        intensityLightCont->switchOn();
    }
    else
    {
        screenLightStatusBtn->setChecked(false);
        screenLightStatusBtn->setStyleSheet("background-image: url(:res/off_btn.png);");
//        brightnessCont->setEnabled(false);
        brightnessCont->setStyleSheet("background-color:#E0E0E0;");
//        brightnessLbl->setStyleSheet("color:#707070;");
        intensityLightCont->switchOff();
    }
    Screen::getInstance()->flushUpdates();
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

void SettingsBrightness::handleBrightnessDecreasePressed()
{
    qDebug() << Q_FUNC_INFO;

    intensityLightCont->decreaseItemSelected();
}

void SettingsBrightness::handleBrightnessDecreaseLongPressed()
{
    qDebug() << Q_FUNC_INFO;

    intensityLightCont->selectMinItem();
}

void SettingsBrightness::handleBrightnessIncreasePressed()
{
    qDebug() << Q_FUNC_INFO;

    intensityLightCont->increaseItemSelected();
}

void SettingsBrightness::handleBrightnessIncreaseLongPressed()
{
    qDebug() << Q_FUNC_INFO;

    intensityLightCont->selectMaxItem();
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
