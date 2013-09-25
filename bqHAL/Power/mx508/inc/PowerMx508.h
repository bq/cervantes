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

#ifndef POWERMX508_H
#define POWERMX508_H

#include <QObject>
#include "Power.h"

#define	POWER_OFF_COMMAND		0xC0	// 192
#define	CM_SYSTEM_RESET			124
#define APM_IOC_SUSPEND			_IO('A', 2)
#define CM_AUDIO_PWR			113
#define	CM_RTC_WAKEUP_FLAG      123
#define CM_LED                  101
#define CM_LED_BLINK 			127
#define CM_POWER_BTN            250
#define LED_POWEROFF            1
#define LED_POWERON             0

class PowerMx508 : public Power
{
    Q_OBJECT

public:
    PowerMx508();

    virtual bool suspend(int autoWakeUpsecs);
    virtual bool sleepCPU(int autoWakeUpSecs);
    virtual void powerOff();
    virtual void reboot();
    virtual void setDebug(bool);

    virtual bool isPowerKeyPressed();
    virtual void setLed(bool);

protected:
    virtual bool isRTCwakeUp();
    virtual void setSleepFlag(int sleepFlag);


    bool b_debugOn;


	
};

#endif // POWERMX508_H
