/*************************************************************************

bq Cervantes e-book reader application
Copyright (C) 2011-2016  Mundoreader, S.L

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

#include <QFile>
#include <QDebug>

/* Needed for calculating uptime values
 * If used in a second place move to generic location
 */ 
#include <time.h>

#define MSEC_PER_SEC    1000L
#define USEC_PER_MSEC   1000L
#define NSEC_PER_MSEC   1000000L

static qint64 monotonic_to_msecs()
{
	struct timespec tv;

	clock_gettime(CLOCK_MONOTONIC, &tv);
	return (qint64) tv.tv_sec * MSEC_PER_SEC + tv.tv_nsec / NSEC_PER_MSEC;
}

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
bool Power::doSpecialTestAction(bool slept){

    qDebug() << Q_FUNC_INFO << "#### BATTERY_TEST"<< BATTERY_TEST << "####";

    bool suspendDisabled = Battery::getInstance()->getLevel() > START_POINT;

    if(BATTERY_TEST == TEST_SLEEP && slept){
        while(Battery::getInstance()->getLevel() > START_POINT){
            sleep(1);
            qDebug() << Q_FUNC_INFO << "Current:" << Battery::getInstance()->getLevel()
                        << "% > threshold" << QString::number(START_POINT) << "%";
            Screen::getInstance()->refreshScreen(); // Refresh screen to empty battery faster
        }
        return true;
    }

    // Empty battery waiting for start level
    for(int i = 0; i < 5 && suspendDisabled ; i++){ // Refresh screen to empty battery faster
        qDebug() << Q_FUNC_INFO << "Current:" << Battery::getInstance()->getLevel()
                    << "% > threshold" << QString::number(START_POINT) << "%";
        Screen::getInstance()->refreshScreen();
        sleep(1);
    }

    if(!slept)
        checkAndResetLight();

    return suspendDisabled;
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

bool Power::getWakeOnHome() {
	return b_wakeOnHome;
}

void Power::setWakeOnHome(bool on) {
	b_wakeOnHome = on;
}

bool Power::wakeupActive(QString *sources, int numSources) {
	QString wakeup_data;
	QFile *wakeDataFile;
	int i;

	for (i = 0; i < numSources; i++) {
		QFile *wakeDataFile;

		qDebug() << Q_FUNC_INFO << "checking wakeup source " << sources[i];

		/* Check 1: is the wakeup active at _this_ moment */

		wakeDataFile = new QFile(sources[i] + "/device/power/wakeup_active");
		if (wakeDataFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
			wakeup_data = QString(wakeDataFile->readLine()).trimmed();
			wakeDataFile->close();
		} else {
			qDebug() << Q_FUNC_INFO << "read of wakeup_last failed";
			wakeup_data = "0";
		}
		delete wakeDataFile;

		if (wakeup_data.toInt()) {
			qDebug() << Q_FUNC_INFO << "wakeup event for " << sources[i] << " in progress, not suspending";
			return true;
		}

		/* Check 2: was the wakeup event less than a second ago */

		wakeDataFile = new QFile(sources[i] + "/device/power/wakeup_last_time_ms");
		if (wakeDataFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
			wakeup_data = QString(wakeDataFile->readLine()).trimmed();
			wakeDataFile->close();
			qDebug() << "read wakeup_last" << wakeup_data.toLongLong();
		} else {
			qDebug() << Q_FUNC_INFO << "read of wakeup_last failed";
			wakeup_data = "0";
		}
		delete wakeDataFile;

		if (monotonic_to_msecs() - wakeup_data.toLongLong() < 1000) {
			qDebug() << Q_FUNC_INFO << "recent wakeup event for " << sources[i] << ", not suspending";
			return true;
		}
	}

	return false;
}
