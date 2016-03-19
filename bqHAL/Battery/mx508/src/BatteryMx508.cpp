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

#include "BatteryMx508.h"

#include <QDebug>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "cmath"


int BatteryMx508::getLevel(){
    qDebug() << "--->" << Q_FUNC_INFO;

    /*  Range of values from the driver: 885 - 1023
        Critical level bit: 0x8000
    */

    int FHWhandle = open("/dev/ntx_io", O_RDWR);
    int value;
    ioctl(FHWhandle, CM_GET_BATTERY_STATUS, &value);
    close (FHWhandle);

    int level = value & 0x7FFF;
    int percentage = qBound(0,(level - MX508_BATTERY_VALUE_MIN)*100/(MX508_BATTERY_VALUE_MAX - MX508_BATTERY_VALUE_MIN),100);

    if (level < MX508_BATTERY_VALUE_MIN) // Check battery critical level
        emit batteryLevelCritical();

    else
        emit batteryLevelChanged(percentage);

    return percentage;
}
