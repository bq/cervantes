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

#include "BQDeviceInfoUtils.h"
#include <QDebug>

BQDeviceInfoUtils::BQDeviceInfoUtils()
{
}

BQDeviceInfoUtils::~BQDeviceInfoUtils()
{
}

QString BQDeviceInfoUtils::getSerialNumber()
{
    qDebug() << Q_FUNC_INFO;
#ifdef Q_WS_QWS

    int max = 3;

    for(int i=1; i<max; i++){
        char devicecode[23];
        // TODO freescale
        int value = 333; //bqsys_deviceId(devicecode,sizeof(devicecode));
        if(value >= 0)
            return QString::fromUtf8(devicecode);
    }

    return "";
#else

    return "SN-TEST000007";

#endif

}
