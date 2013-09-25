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
#include "PowerEmu.h"
#include <QDebug>

void PowerEmu::powerOff()
{
    qDebug() << "--->" << Q_FUNC_INFO;
}

void PowerEmu::reboot()
{
    qDebug() << "--->" << Q_FUNC_INFO;
}

bool PowerEmu::suspend(int autoSleepSecs)
{
    return true;
}

bool PowerEmu::sleepCPU(int autoWakeUpSecs)
{
    qDebug() << "--->" << Q_FUNC_INFO;
    return true;
}

void PowerEmu::setDebug(bool state){
    qDebug() << "--->" << Q_FUNC_INFO;
    Q_UNUSED(state);
}

bool PowerEmu::isRTCwakeUp(){
    qDebug() << "--->" << Q_FUNC_INFO;
    return false;
}

bool PowerEmu::isPowerKeyPressed(){
    qDebug() << "--->" << Q_FUNC_INFO;
    return false;
}

void PowerEmu::setLed(bool on){
    qDebug() << "--->" << Q_FUNC_INFO << on;
}
