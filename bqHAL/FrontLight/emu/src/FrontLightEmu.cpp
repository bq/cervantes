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

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <QDebug>

#include "FrontLightEmu.h"


FrontLightEmu::FrontLightEmu():
    b_active(false)
    ,i_brightnessValue(50)
{
    //Do Nothing
}

bool FrontLightEmu::switchFrontLight(bool state){
    qDebug() << Q_FUNC_INFO << state;
    emit frontLightPower(state);
    return true;
}

bool FrontLightEmu::setBrightness(int brightness)
{
    qDebug() << Q_FUNC_INFO << brightness;

    i_brightnessValue = brightness;
    return true;

}

void FrontLightEmu::setFrontLightActive(bool on)
{
    qDebug() << Q_FUNC_INFO << on;
    b_active = on;
}

bool FrontLightEmu::isFrontLightActive(){
    qDebug() << Q_FUNC_INFO;

    return b_active;
}

int FrontLightEmu::getBrightness()
{
    qDebug() << Q_FUNC_INFO;

    return i_brightnessValue;
}

