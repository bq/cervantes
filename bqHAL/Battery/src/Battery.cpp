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

#include "Battery.h"

#include "Power.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifndef Q_WS_QWS
#include "BatteryEmu.h"
#elif MX508
#include "BatteryMx508.h"
#endif

#include <QWidget>
#include <QDebug>
#include <QTimer>

Battery* Battery::_instance = NULL;

Battery::Battery()
{

    connect(&m_timer_checkLevel,SIGNAL(timeout()),this,SLOT(checkBatteryLevel()));
    m_timer_checkLevel.setSingleShot(false); // Periodic
    m_timer_checkLevel.setInterval(BATTERY_CHECK_TIME);
    m_timer_checkLevel.start();

}

Battery* Battery::getInstance()
{
    return _instance;
}

void Battery::staticInit()
{
    if (_instance == NULL)
    {
#ifndef Q_WS_QWS
            _instance = new BatteryEmu();
#elif MX508
            _instance = new BatteryMx508();
#endif
    }
}

void Battery::staticDone()
{
    if(_instance)
    {
        delete _instance;
        _instance = NULL;
    }
}

void Battery::checkBatteryLevel()
{
    qDebug() << Q_FUNC_INFO;

    /*
      Due to mutex mechanism for I2C access, battery reading
      level must be skipped when power key pressed
    **/
    if(Power::getInstance()->isPowerKeyPressed())
    {
        qDebug() << Q_FUNC_INFO << "Not possible to check battery, power key down";
        return;
    }

    int newLevel = getLevel();
    if(newLevel != _currentLevel) {

        if(newLevel <= BATTERY_LEVEL_LOW && BATTERY_LEVEL_LOW < _currentLevel
          || newLevel <= BATTERY_LEVEL_LOW_2 && BATTERY_LEVEL_LOW_2 < _currentLevel)
        {
            qDebug() << Q_FUNC_INFO << "Battery low" << newLevel;
            emit batteryLevelLow(newLevel);
        }
        else if (newLevel == BATTERY_LEVEL_FULL
                   && newLevel > _currentLevel)
        {
            qDebug() << Q_FUNC_INFO << "Battery full" << newLevel;
            emit batteryLevelFull();
        }

        qDebug() << Q_FUNC_INFO << "Battery level change" << newLevel;
        emit batteryLevelChanged(newLevel);
        _currentLevel = newLevel;
    }
}

