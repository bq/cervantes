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

#ifndef POWEREMU_H
#define POWEREMU_H

#include "Power.h"
#include <QObject>

class PowerEmu : public Power
{
    Q_OBJECT

public:

    virtual bool suspend(int);
    virtual bool sleepCPU(int);
    virtual void powerOff();
    virtual void reboot();
    virtual void setDebug(bool);
    virtual bool isRTCwakeUp();

    virtual bool isPowerKeyPressed();
    virtual void setLed(bool);

};

#endif // POWEREMU_H
