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

#ifndef FRONTLIGHTMX508_H
#define FRONTLIGHTMX508_H


#include "FrontLight.h"
#include <QObject>

#define POST_SETTING_AWAKE_TIME     5
#define MAX_TIME_FLSWITCHON_MS      5000
#define I2C_ACCESS_REATT_PERIOD_MS  50


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

signals:

    void frontLightPower(bool);

protected slots:
    void saveSettings();

private:
    bool setSleep(bool);

    bool b_active;
    int i_brightnessValue;
    PowerManagerLock* m_powerLock;

};

#endif // FrontLightMX508
