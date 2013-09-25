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

#ifndef BATTERYHANDLER_H
#define BATTERYHANDLER_H

#include <QObject>
#include <QTimer>

#define BATTERY_CHECK_TIME              20000 // Every 20 seconds awake
#define BATTERY_LEVEL_FULL              100
#define BATTERY_LEVEL_LOW               10

class QTimerEvent;

class Battery : public QObject
{
    Q_OBJECT

public:
    static Battery* getInstance();
    static void staticInit();
    static void staticDone();

public slots:
    virtual int getLevel() = 0;

protected:
    Battery();

signals:
    void batteryLevelFull();
    void batteryLevelLow();
    void batteryLevelCritical();
    void batteryLevelChanged(int);

private:
    static Battery* _instance;
    QTimer m_timer_checkLevel;
    int _currentLevel;

private slots:
    void checkBatteryLevel();

};

#endif // BATTERYHANDLER_H
