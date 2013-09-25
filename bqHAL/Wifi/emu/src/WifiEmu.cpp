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

#include "WifiEmu.h"

#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "QBook.h"
#include "PowerManager.h"

#include <QEventLoop>
#include <QProcess>
#include <QNetworkInterface>
#include <QTimer>
#include <QBasicTimer>
#include <QTimerEvent>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QString>
#include <QSettings>

QString WifiEmu::getMacAddress()
{
    return QString("2A:0F:11:21:85:DF");
}

WifiEmu::WifiEmu()
{
}

WifiEmu::~WifiEmu()
{
}

void WifiEmu::prepareWifiInterface()
{
}

void WifiEmu::powerOff()
{
}

void WifiEmu::powerOn()
{
}
