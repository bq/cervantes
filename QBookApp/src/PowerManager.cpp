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

#include "PowerManager.h"
#include "PowerManagerLock.h"
#include "PowerManagerDefs.h"

#include "Power.h"
#include "Screen.h"
#include "Battery.h"
#include "RTCManager.h"
#include "DeleteLaterFile.h"

#include "QBookApp.h"
#include "QBook.h"
#include "QBookScreenSaver.h"
#include "ConnectionManager.h"
#include "Wifi.h"
#include "ConnectionManager.h"
#include "SelectionDialog.h"
#include "Storage.h"
#include "Viewer.h"
#include "ADConverter.h"

#include <QList>
#include <QBasicTimer>
#include <QDateTime>
#include <QMutex>
#include <QCoreApplication>

#include <unistd.h>

#ifdef Q_WS_QWS
extern "C" void QBookScreen_Sync(void);
extern "C" void QBookScreen_WaitPanelNotBusy(void);
#endif

/** INITIALIZATION **/
PowerManager* PowerManager::m_instance = NULL;
QBookScreenSaver* PowerManager::screenLoader = NULL;
QList<PowerManagerLock*> PowerManager::m_allLocks;
QList<PowerManagerLock*> PowerManager::m_activeLocks;
PowerManagerLock* PowerManager::m_internalLock = NULL;
#if defined(SHOWCASE) || defined (BATTERY_TEST)
int PowerManager::i_timeToWakeUp = SAMPLE_PERIOD - POWERMANAGER_AWAKE_MINIMUM_TIME/1000;
#else
int PowerManager::i_timeToWakeUp = POWERMANAGER_UPDATE_CLOCK_TIME;
#endif
int PowerManager::i_timePowerOffSecs = POWERMANAGER_TIME_POWEROFFSECS;
int PowerManager::i_timeSleepSecs = POWERMANAGER_TIME_AUTOSLEEPSECS;
QMutex PowerManager::m_mutex;


PowerManager::PowerManager()
{
    PowerManager::reviewAutoFrequencies();
    m_scheduledRTCalarm = QDateTime::currentDateTime();
}

PowerManager::~PowerManager()
{
    // Do nothing
}

void PowerManager::initialize()
{
    getInstance();
    m_internalLock = getNewLock(m_instance);
}

void PowerManager::start()
{
    getInstance()->setTimeLatestUserEvent();
    getInstance()->m_timer_reviewStatus.start(POWERMANAGER_AWAKE_USER_TIME,getInstance());
}

// Double checked locking + Singleton pattern.
PowerManager* PowerManager::getInstance()
{
    if(!PowerManager::m_instance) {
        m_mutex.lock();

        if(!PowerManager::m_instance)
            PowerManager::m_instance = new PowerManager();

        m_mutex.unlock();
    }

    return PowerManager::m_instance;
}

/** Reset the times for auto sleep and auto power off. Useful method to update them in case the user changes them in settings */
void PowerManager::reviewAutoFrequencies()
{
    i_timeSleepSecs = QBook::settings().value("setting/sleepTimeInSecs", POWERMANAGER_TIME_AUTOSLEEPSECS).toInt();
    i_timePowerOffSecs = QBook::settings().value("setting/powerOffTimeInSecs", POWERMANAGER_TIME_POWEROFFSECS).toInt();
}


/** POWER MANAGEMENT STATUS **/

void PowerManager::setTimeLatestUserEvent(){
    // Cancel ongoing suspension
    Power::getInstance()->cancelOngoingProcess();

    // Reset suspend timer
    if(m_timer_reviewStatus.isActive()){
        m_timer_reviewStatus.start(POWERMANAGER_AWAKE_USER_TIME,this);
        b_awakeDueToUser = true;
    }
}

void PowerManager::cancelSleep()
{
    b_sleepCanceled = true;
}

void PowerManager::goToSleep()
{
    qDebug() << Q_FUNC_INFO;

    // Initialize variables
    bool ret = false;
    int reattemptsCount = 0;
    int powerKeySample = 0;
    int wakeUpAlarmTimer = 0;

    PowerManager::m_internalLock->activate();
    getInstance()->b_sleepCanceled = false;

    // set autopoweroff alarm
    QDateTime m_scheduledPowerOffTime = QDateTime::currentDateTime().addSecs(i_timePowerOffSecs-2);// -2 to prevent hwclock delay effect
    qDebug() << Q_FUNC_INFO << "m_scheduledPowerOffTime:" << m_scheduledPowerOffTime.toString();

    emit getInstance()->goingToSleep();

    /**
        Process events to ensure execution of actions
        triggered by goingToSleep()
    */
    qApp->processEvents();

    // As goingToSleep does a lot of processing on the QBookApp side, user may have canceled the sleep pressing power back
    if (getInstance()->b_sleepCanceled) {
        getInstance()->returnFromSleep();
        return;
    }
    //*******************************************//
    emit getInstance()->releaseConnectedPowerLock();

    //********************************************//

    if(ADConverter::getInstance()->getStatus() != ADConverter::ADC_STATUS_NO_WIRE)
        wakeUpAlarmTimer = POWERMANAGER_PLUGGED_WAKEUP_PERIOD;
    else
        wakeUpAlarmTimer = POWERMANAGER_SLEEP_WAKEUP_PERIOD;

    ret = Power::getInstance()->sleepCPU(wakeUpAlarmTimer);

    // Reattempt sleep every 3 seconds
    while(!ret && reattemptsCount < POWERMANAGER_SLEEP_MAX_REATTEMPTS){ // Faulty attempt
        usleep(100000);
        powerKeySample++;

        // Check power key every 100ms to resume if pressed
        if(Power::getInstance()->isPowerKeyPressed())
            break;

        // Reattempt every 3 seconds
        if(powerKeySample > 30){
            reattemptsCount++;
            powerKeySample=0;
            qDebug() << Q_FUNC_INFO << "Reattempting sleep:" << reattemptsCount;

            ret = Power::getInstance()->sleepCPU(wakeUpAlarmTimer);
        }
    }

    /*-----------------SLEPT---------------------------------*/

    // Loop to handle auto-shutdown after configurable time of sleep
    while(Power::getInstance()->isRTCwakeUp()){
#ifdef BATTERY_TEST
        Power::getInstance()->sleepCPU(SAMPLE_PERIOD);
        continue;
#endif
        if(QDateTime::currentDateTime() >= m_scheduledPowerOffTime // Auto-poweroff feature
            || Battery::getInstance()->getLevel() <= 0) // Empty battery
        {
            powerOffDevice(QBook::settings().value("setting/initial_lang_selection", true).toBool());
            return;
        }

        // Do needed checks
        emit getInstance()->checkWhileSleeping();
        sleep(2); // In order to not to interrupt a panel refresh

        // Go to sleep again
        Power::getInstance()->sleepCPU(wakeUpAlarmTimer);
    }

    QBookApp::instance()->closeSleep();
    getInstance()->returnFromSleep();

    QBookApp::instance()->disablePowerKey();
    emit getInstance()->backFromSleep();
}

void PowerManager::goToSuspend()
{
    bool ret;
    int rtcAlarmSetting = 0;

    qDebug() << Q_FUNC_INFO << "Scheduled sleep:" << m_scheduledSleepTime.toString("hh:mm:ss.zzz");

    // Re-schedule sleep time if user interaction
    if(b_awakeDueToUser) // User interaction
        m_scheduledSleepTime = QDateTime::currentDateTime().addSecs(i_timeSleepSecs);

    b_awakeDueToUser = false;

#ifndef BATTERY_TEST // Prevent auto-sleep if Battery test
#ifndef SHOWCASE    // Prevent auto-sleep if Showcase build

    // Self sleep if no user activity
    if(QDateTime::currentDateTime() >= m_scheduledSleepTime){
        QBookApp::instance()->goToSleep();
        return;
    }

#endif
#endif

    /**
      RTC alarm to wake up is set only if last scheduled wake up
      time has already expired and has to be reset.
      Additionally, system time is synced with RTC time that is
      more accurate.
    */
    if(m_scheduledRTCalarm < QDateTime::currentDateTime())
    {
        if(!Power::getInstance()->isPowerKeyPressed())
            RTCManager::setSystemDateFromHw();

        rtcAlarmSetting = i_timeToWakeUp;
        m_scheduledRTCalarm = QDateTime::currentDateTime().addSecs(rtcAlarmSetting);
        qDebug() << Q_FUNC_INFO << "RTC wake up scheduled at:" << m_scheduledRTCalarm.toString("hh:mm:ss.zzz");
    }

    ret = Power::getInstance()->suspend(rtcAlarmSetting);

    /*-----------------SUSPENDED---------------------------------*/

    /** when suspend failed, assume there is some sort of
        userevent pending and wait some time to reattempt
    */
    if (!ret)
    {
        qDebug() << Q_FUNC_INFO << "SUSPEND FAILED, most likely a user event";
        m_scheduledRTCalarm = QDateTime::currentDateTime(); // Not to assume RTC as set
    }

    emit backFromSuspend();
}

void PowerManager::returnFromSleep()
{
    qDebug() << "--->" << Q_FUNC_INFO;

    qApp->processEvents();

    m_timer_reviewStatus.start(POWERMANAGER_AWAKE_USER_TIME,this);
    setTimeLatestUserEvent();
    m_scheduledRTCalarm = QDateTime::currentDateTime();
    PowerManager::m_internalLock->release();
}

void PowerManager::powerOffDevice(bool defaultSettings)
{
    qDebug() << Q_FUNC_INFO;

    PowerManager::m_internalLock->activate();

    if(defaultSettings)
    {
        Screen::getInstance()->flushColorScreen(Qt::black);
        Screen::getInstance()->setMode(Screen::MODE_QUICK, true, FLAG_WAITFORCOMPLETION, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
    }

    Screen::getInstance()->queueUpdates();
    Screen::getInstance()->flushColorScreen();
    Screen::getInstance()->setMode(Screen::MODE_QUICK, true, FLAG_WAITFORCOMPLETION, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    // Show power off screen. TODO: change for new class ScreenManager
    Screen::getInstance()->queueUpdates();
    screenLoader = new QBookScreenSaver();
    screenLoader->setFixedSize(Screen::getInstance()->screenWidth(), Screen::getInstance()->screenHeight());
    if(!defaultSettings)
        screenLoader->setScreenType(QBookScreenSaver::POWER_OFF);
    else
        screenLoader->setScreenType(QBookScreenSaver::DEFAULT_SETTINGS);

#ifdef Q_WS_QWS
    screenLoader->showFullScreen();
#else
    screenLoader->show();
#endif

    /*
     * we want to indicate that we're still working.
     * The led will turn itself off, when the whole system is off.
     */
    Power::getInstance()->setLed(true);

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_WAITFORCOMPLETION, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
    qApp->processEvents();

    emit getInstance()->shuttingDown();

    QApplication::exit(199);
}


/** LOCK RELATED METHODS **/

PowerManagerLock* PowerManager::getNewLock(QObject* requester)
{
    PowerManagerLock* newLock = new PowerManagerLock(requester);
    m_allLocks.append(newLock);

    qDebug() << "Created new lock" << newLock;

    return newLock;
}

QList<PowerManagerLock*> PowerManager::getCurrentLocks(bool justActive)
{
#ifdef Q_WS_QWS
    qDebug() << Q_FUNC_INFO << "active:" << m_activeLocks << "All locks size:" << m_allLocks.size();
#endif

    if(justActive)
        return m_activeLocks;
    else
        return m_allLocks;
}

bool PowerManager::isLockActive(PowerManagerLock *lock)
{
    return m_activeLocks.contains(lock);
}

void PowerManager::activateLock(PowerManagerLock *lock)
{
    qDebug() << Q_FUNC_INFO << lock;

    if(!isLockActive(lock)){
        getInstance()->m_timer_reviewStatus.stop();
        m_activeLocks.append(lock);
    }
}

void PowerManager::releaseLock(PowerManagerLock *lock)
{
    qDebug() << Q_FUNC_INFO << lock;
    m_activeLocks.removeAll(lock);

    if(getCurrentLocks(true).size() == 0 && !getInstance()->m_timer_reviewStatus.isActive())
        getInstance()->m_timer_reviewStatus.start(POWERMANAGER_AWAKE_USER_TIME,getInstance());
}

void PowerManager::removeLock(PowerManagerLock *lock)
{
    qDebug() << Q_FUNC_INFO << lock;

    releaseLock(lock);
    m_allLocks.removeAll(lock);
}

void PowerManager::timerEvent(QTimerEvent* event)
{    
    if(event->timerId() == m_timer_reviewStatus.timerId())
    {
        m_timer_reviewStatus.stop();

        event->accept();

        if(getCurrentLocks(true).size() > 0) //Safety check
            return;

        /**
            If power key is pressed no action is done to allow forced reset.
            If any RTC call is done the timer to check forced reset is restarted,
            leading to malfunction of the poweroff mechanism
        */
        if(!Power::getInstance()->isPowerKeyPressed())
            goToSuspend();

        m_timer_reviewStatus.start(POWERMANAGER_AWAKE_MINIMUM_TIME,this);

    } else
        QObject::timerEvent(event);

}
