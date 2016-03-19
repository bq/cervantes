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

#ifndef POWER_H
#define POWER_H

#include <QObject>

#if defined(BATTERY_TEST) || defined(SHOWCASE)
#include <QDateTime>

#define FRONT_LIGHT_BLINK_PERIOD    21600  // Re-switch on the light every 6 hours because
                                           // microcontroller auto switch off after certain time
#endif
struct input_event;

class Power : public QObject
{
    Q_OBJECT

public:

    enum PowerStatus {
        NORMAL,
        SUSPEND,
        SLEEP
    };

#ifdef BATTERY_TEST
    enum TestId {
        NA,                 // 0
        TEST_SLEEP,         // 1
        TEST_IDLE,          // 2
        TEST_PAGES_NORMAL,  // 3
        TEST_PAGES_QUICK,   // 4
    };

#endif

    static Power* getInstance();
    static void staticInit();
    static void staticDone();
    virtual bool suspend(int) = 0;
    virtual bool sleepCPU(int) = 0;
    virtual void powerOff() = 0;
    virtual void reboot() = 0;
    virtual void setDebug(bool) = 0;

    virtual void cancelOngoingProcess() { b_processCanceled = true; }
    virtual bool isRTCwakeUp() = 0;
    virtual bool isPowerKeyPressed() = 0;
    virtual void setLed(bool) = 0;

    virtual bool getWakeOnHome();
    virtual void setWakeOnHome(bool);

#if defined(BATTERY_TEST) || defined(SHOWCASE)
    bool doSpecialTestAction(bool slept);
    void checkAndResetLight();

    QDateTime m_scheduledFLBlinkTime;
#endif

protected:
    bool b_processCanceled;
    bool b_wakeOnHome;
    virtual bool wakeupActive(QString *sources, int numSources);

private:
    static Power* _instance;

};

#endif // POWER_H
