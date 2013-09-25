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

#ifndef DEVICEINFOEMU_H
#define DEVICEINFOEMU_H

#include "DeviceInfo.h"

class QString;
class QStringList;
class QSettings;

class DeviceInfoEmu : public DeviceInfo
{
    Q_OBJECT

public:
    QString getSerialNumber();
    QHash<QString, QString> getSpecificDeviceInfo();
    bool hasFrontLight();
    int getHwId();

private:
    static void readTouchVersion(char*);
    static QString getVCOM_EEPROM();
    static QString getVcom();
    static QString getLutVersion();

    static void setInitInfo(QSettings&);
    static QString getDeviceEnv(int,int);
    static void populateHWinfo(QStringList&, const QSettings&);
    static void populateSCRinfo(QStringList&);
};

#endif // DEVICEINFOSEMU_H
