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

#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QObject>
#include <QHash>

class QString;
class QStringList;
class QSettings;

class DeviceInfo : public QObject
{
    Q_OBJECT

public:

    enum hwModelId {
        UNKNOWN_HW_MODEL,
        E606A2,     // Cervantes Touch, iMX5
        E60672,     // Cervantes Touch Light, iMX5
        E60Q22      // Cervantes 2013
    };

    static DeviceInfo* getInstance();
    static void staticInit();
    static void staticDone();

    virtual QString getSerialNumber()=0;
    virtual QHash<QString, QString> getSpecificDeviceInfo()=0;
    virtual bool hasFrontLight()=0;
    virtual int getHwId()=0;

private:
    static DeviceInfo* _instance;

};

#endif // DEVICEINFO_H
