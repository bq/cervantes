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
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "WifiMx508.h"
#include "DeviceInfo.h"

#include "QBook.h"
#include "PowerManager.h"

#include <QDebug>
#include <QFile>
#include <QList>
#include <QString>
#include <QStringList>
#include <QBasicTimer>
#include <QTimer>
#include <QEventLoop>
#include <QProcess>
#include <QNetworkInterface>
#include <QDateTime>
#include <QSettings>

WifiMx508::~WifiMx508()
{
    qDebug() << Q_FUNC_INFO;
}

void WifiMx508::powerOff()
{
    qDebug() << "--->" << Q_FUNC_INFO;

    /* circumvent high power consumption on realtek wifi for now */
    int hwid = DeviceInfo::getInstance()->getHwId();
    if(hwid == DeviceInfo::E60QP2) {
        systemCall("rmmod 8189fs");
    }
}

/* virtual */ QString WifiMx508::getMacAddress()
{
    QNetworkInterface wifiInterface = QNetworkInterface::interfaceFromName(QString("eth0"));
    return wifiInterface.hardwareAddress ();
}

void WifiMx508::powerOn()
{
    qDebug() << "--->" << Q_FUNC_INFO;

    /* circumvent high power consumption on realtek wifi for now */
    int hwid = DeviceInfo::getInstance()->getHwId();
    if(hwid == DeviceInfo::E60QP2) {
        systemCall("modprobe 8189fs rtw_channel_plan=0x21");
    }
}


bool WifiMx508::systemCall(const QString& command) const
{
    int returned = system(command.toAscii().constData());
    return returned == 0;
}
