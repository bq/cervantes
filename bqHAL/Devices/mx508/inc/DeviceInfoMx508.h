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

#ifndef DEVICEINFOMX508_H
#define DEVICEINFOMX508_H

#include "DeviceInfo.h"

class QString;
class QStringList;
class QSettings;

class DeviceInfoMx508 : public DeviceInfo
{
    Q_OBJECT

public:
    DeviceInfoMx508();
    QHash<QString, QString> getSpecificDeviceInfo();
    QString getSerialNumber() { return serial; }

    bool hasFrontLight();
    bool hasOptimaLight();
    int getHwId();

private:
    QString serial;
};

#endif // DEVICEINFOMX508_H
