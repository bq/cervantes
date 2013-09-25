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

#include "Power.h"
#ifndef Q_WS_QWS
#include "PowerEmu.h"
#elif MX508
#include "PowerMx508.h"
#endif

#if defined(BATTERY_TEST) || defined(SHOWCASE)
#include "RTCManager.h"
#include "Battery.h"
#include "Screen.h"
#include "FrontLight.h"
#include "DeviceInfo.h"
#include "QBook.h"
#include <QString>
#include <QDebug>
#include <QDateTime>
#endif

Power* Power::_instance = NULL;

Power* Power::getInstance()
{
    return _instance;
}

void Power::staticInit()
{
    if (_instance == NULL)
    {
#ifndef Q_WS_QWS
            _instance = new PowerEmu();
#elif MX508
            _instance = new PowerMx508();
#endif
    }
}

void Power::staticDone()
{
    if(_instance)
    {
        delete _instance;
        _instance = NULL;
    }
}

#ifdef BATTERY_TEST
void Power::specialTestAction(bool slept){

    qDebug() << Q_FUNC_INFO << "#### BATTERY_TEST"<< BATTERY_TEST << "####";

    b_suspendDisabled = Battery::getInstance()->getLevel() > START_POINT;

    if(BATTERY_TEST == TEST_SLEEP && slept){
        while(Battery::getInstance()->getLevel() > START_POINT){
            sleep(1);
            qDebug() << Q_FUNC_INFO << "Current:" << Battery::getInstance()->getLevel()
                        << "% > threshold" << QString::number(START_POINT) << "%";
            Screen::getInstance()->refreshScreen(); // Refresh screen to empty battery faster
        }
        return;
    }

    else if(BATTERY_TEST == TEST_PAGES_QUICK)
        RTCManager::setRTCAlarm(TURNING_QUICK_PERIOD);

    else if(BATTERY_TEST == TEST_PAGES_NORMAL)
        RTCManager::setRTCAlarm(TURNING_NORMAL_PERIOD);


    // Empty battery waiting for start level
    for(int i = 0; i < 5 && b_suspendDisabled ; i++){ // Refresh screen to empty battery faster
        qDebug() << Q_FUNC_INFO << "Current:" << Battery::getInstance()->getLevel()
                    << "% > threshold" << QString::number(START_POINT) << "%";
        Screen::getInstance()->refreshScreen();
        sleep(1);
    }

    if(!slept)
        checkAndResetLight();
}


#endif

#if defined(BATTERY_TEST) || defined(SHOWCASE)
void Power::checkAndResetLight(){
    /**
    Function to Re-switch on the light every 6 hours because
    microcontroller auto switch off after certain time without change
    */
    qDebug() << Q_FUNC_INFO;

    if(DeviceInfo::getInstance()->hasFrontLight()
       && FrontLight::getInstance()->isFrontLightActive()
       && QDateTime::currentDateTime() > m_scheduledFLBlinkTime){

            qDebug() << Q_FUNC_INFO << "re-switching on light";
            FrontLight::getInstance()->switchFrontLight(false);
            sleep(1);
            int brightness = FrontLight::getInstance()->getBrightness();
            FrontLight::getInstance()->setBrightness(brightness);
            FrontLight::getInstance()->switchFrontLight(true);
            m_scheduledFLBlinkTime = QDateTime::currentDateTime().addSecs(FRONT_LIGHT_BLINK_PERIOD);
    }
}
#endif

