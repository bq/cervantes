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

#ifndef POWERMANAGER_H
#define POWERMANAGER_H

#include <QObject>
#include <QBasicTimer>
#include <QDateTime>

#include "PowerManagerLock.h"

class QTimerEvent;
class QBasicTimer;
class QBookScreenSaver;
class QMutex;

class PowerManager : public QObject
{
    Q_OBJECT

public:    
    static void initialize();
    static void start();
    static PowerManager* getInstance();
    static void reviewAutoFrequencies();
    static PowerManagerLock* getNewLock(QObject* requester = NULL);
    static QList<PowerManagerLock*> getCurrentLocks(bool justActive = false);
    static bool isLockActive(PowerManagerLock*);
    static void activateLock(PowerManagerLock*);
    static void releaseLock(PowerManagerLock*);
    static void removeLock(PowerManagerLock*);
    void cancelSleep();

public slots:
    static void powerOffDevice(bool defaultSettings = false);
    static void goToSleep();
    void goToSuspend();
    inline void setTimeLatestUserEvent();

private slots:
    void returnFromSleep();

signals:
    void backFromSuspend();
    void goingToSleep();
    void backFromSleep();
    void releaseConnectedPowerLock();
    void shuttingDown();

private:    
    PowerManager();
    virtual ~PowerManager();
    PowerManager(const PowerManager &); // hide copy constructor
    virtual void timerEvent(QTimerEvent *event);
    PowerManager& operator=(const PowerManager &); // hide assign op
    void rollbackFromSuspend();

    static QList<PowerManagerLock*> m_activeLocks;
    static QList<PowerManagerLock*> m_allLocks;
    static PowerManager* m_instance;    
    QBasicTimer m_timer_reviewStatus;
    static int i_timeToWakeUp;
    static int i_timeSleepSecs;
    static int i_timePowerOffSecs;
    static QBookScreenSaver* screenLoader;
    static PowerManagerLock* m_internalLock;
    static PowerManagerLock* m_failedSuspendLock;
    static QMutex m_mutex;
    QDateTime m_scheduledSleepTime;
    QDateTime m_scheduledRTCalarm;
    bool b_awakeDueToUser;
    bool b_sleepCanceled;

};

#endif // POWERMANAGER_H
