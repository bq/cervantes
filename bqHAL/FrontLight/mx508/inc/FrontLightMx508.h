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

#ifndef FRONTLIGHTMX508_H
#define FRONTLIGHTMX508_H


#include "FrontLight.h"
#include <QObject>
#include <QTime>
#include <QTimer>

#define POST_SETTING_AWAKE_TIME     5
#define MAX_TIME_FLSWITCHON_MS      5000
#define I2C_ACCESS_REATT_PERIOD_MS  50
#define DEFAULT_OPTIMALIGHT_VALUE   5
#define DEFAULT_OPTIMALIGHT_ID      3
#define OPTIMALIGHT_SUNRISE_BASE    "05:00"
#define OPTIMALIGHT_SUNSET_BASE     "20:00"
#define OPTIMALIGHT_DAY_PRESET_ID   8
#define OPTIMALIGHT_NIGHT_PRESET_ID 2
#define OPTIMALIGHT_TIME_INTERVAL   1800 // Seconds

class PowerManagerLock;

class FrontLightMx508 : public FrontLight
{
    Q_OBJECT

public:

    FrontLightMx508();

    bool switchFrontLight(bool state);
    void setFrontLightActive(bool on);
    bool isFrontLightActive() {return b_active;}
    bool setBrightness(int);
    int getBrightness();
    bool setOptimaLightAutoMode(bool);
    bool isOptimaLightAutoActive() {return b_optimaLightAutoMode;}
    bool setOptimaLightValue(int);
    int getOptimaLightValue() {return i_optimaLightValue;}
    int getOptimaLightSunriseId() {return i_optimaLightSunriseId;}
    int getOptimaLightSunsetId() {return i_optimaLightSunsetId;}
    QTime getOptimaLightSunriseTime() {return m_optimaLightSunriseTime;}
    QTime getOptimaLightSunsetTime() {return m_optimaLightSunsetTime;}

public slots:
    bool setOptimaLightSunrise(int /*id*/);
    bool setOptimaLightSunset(int /*id*/);
    void checkOptimaLightAutoSetting();

signals:

    void frontLightPower(bool);

protected slots:
    void saveSettings();

private:
    bool setSleep(bool);
    bool setOptimaLightSunriseTime(int);
    bool setOptimaLightSunsetTime(int);

    bool b_active;
    int i_brightnessValue;
    bool b_optimaLightAutoMode;
    int i_optimaLightValue;
    int i_optimaLightSunriseId;
    int i_optimaLightSunsetId;
    QTime m_optimaLightSunriseTime;
    QTime m_optimaLightSunsetTime;
    QTimer m_optimaLightAutoTimer;
    PowerManagerLock* m_powerLock;

};

#endif // FrontLightMX508
