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

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <QDebug>
#include <QTimer>

#include "FrontLightMx508.h"
#include "Power.h"
#include "SleeperThread.h"
#include "QBook.h"
#include "PowerManager.h"

#define CM_FRONT_LIGHT_SET                241
#define CM_FRONT_LIGHT_SLEEP              245

FrontLightMx508::FrontLightMx508() :
    b_active(false)
{
    b_active = QBook::settings().value("light/power",false).toBool();

    /** Workaround to allow switch on with value 1
        because it is not possible because of HW limitation */
    if(QBook::settings().value("light/value", 50).toInt() == 1)
        QBook::settings().setValue("light/value", 2);

    i_brightnessValue = QBook::settings().value("light/value",50).toInt();

    m_powerLock = PowerManager::getNewLock(this);
    m_powerLock->setTimeOut(POST_SETTING_AWAKE_TIME);
    connect(m_powerLock, SIGNAL(lockTimedOut()), this, SLOT(saveSettings()),Qt::UniqueConnection);
}

void FrontLightMx508::setFrontLightActive(bool on)
{
    qDebug() << Q_FUNC_INFO << on;

    if(switchFrontLight(on))
    {
        b_active = on;
        m_powerLock->activate();
    }
}

bool FrontLightMx508::switchFrontLight(bool state){
    qDebug() << Q_FUNC_INFO << state;

    /* sleep argument to specify that is necessary
       to set just on/off register

       Due to mutex mechanism for I2C access, FL switching must
       be skipped when waking up because of power key
    **/

    int maxNumReatts = MAX_TIME_FLSWITCHON_MS/I2C_ACCESS_REATT_PERIOD_MS;
    int numAttempts = 0;
    while(Power::getInstance()->isPowerKeyPressed()
          && numAttempts < maxNumReatts)
    {
        numAttempts++;
        usleep(I2C_ACCESS_REATT_PERIOD_MS*1000);
    }

    while(!Power::getInstance()->isPowerKeyPressed()
          && numAttempts < maxNumReatts)
    {
        int ret = setSleep(state);
        if(ret)
        {
            emit frontLightPower(state);
            return true; // Successful setting
        }
        qDebug() << Q_FUNC_INFO << "FAULTY FL SWITCH ATTEMPT" << numAttempts;
        numAttempts++;
        usleep(I2C_ACCESS_REATT_PERIOD_MS*1000);
    }

    /**
        It was not possible to switch, either because
        of powerKey pressed or unknown failure
    */
    qWarning() << Q_FUNC_INFO << "FL SWITCHING TOTAL FAILURE";
    return false;
}

bool FrontLightMx508::setBrightness(int brightness)
{
    qDebug() << Q_FUNC_INFO << brightness;

    int FHWhandle = open("/dev/ntx_io", O_RDWR);
    if(!FHWhandle){
        qWarning() << Q_FUNC_INFO << "IO File opening fault";
        return false;
    }

    int result = !ioctl (FHWhandle, CM_FRONT_LIGHT_SET, brightness);
    close (FHWhandle);

    bool power = brightness > 0;

    if(result)
    {
        i_brightnessValue = brightness;
        m_powerLock->activate();
    }

    return result;
}

bool FrontLightMx508::setSleep(bool on)
{
    qDebug() << Q_FUNC_INFO << on;

    int FHWhandle = open("/dev/ntx_io", O_RDWR);
    if(!FHWhandle){
        qWarning() << Q_FUNC_INFO << "IO File opening fault";
        return false;
    }

    int result = !ioctl (FHWhandle, CM_FRONT_LIGHT_SLEEP, on);
    close (FHWhandle);

    return result;
}

int FrontLightMx508::getBrightness()
{
    qDebug() << Q_FUNC_INFO << i_brightnessValue;

    return i_brightnessValue;
}

void FrontLightMx508::saveSettings()
{
    qDebug() << Q_FUNC_INFO << b_active << i_brightnessValue;

    QBook::settings().setValue("light/power", b_active);
    QBook::settings().setValue("light/value", i_brightnessValue);
}
