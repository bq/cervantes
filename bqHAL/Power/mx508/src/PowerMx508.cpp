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

#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <QProcess>
#include <QDebug>
#include <QDateTime>
#include <QCoreApplication>
#include <QFile>
#include <QTimer>

#include "Power.h"
#include "RTCManager.h"
#include "PowerMx508.h"

#ifdef BATTERY_TEST
#include "Screen.h"
#include "Battery.h"
#endif

/* Needed for calculating uptime values
 * If used in a second place move to generic location
 */ 
#include <time.h>

#define MSEC_PER_SEC    1000L
#define USEC_PER_MSEC   1000L
#define NSEC_PER_MSEC   1000000L

static qint64 monotonic_to_msecs()
{
	struct timespec tv;

	clock_gettime(CLOCK_MONOTONIC, &tv);
	return (qint64) tv.tv_sec * MSEC_PER_SEC + tv.tv_nsec / NSEC_PER_MSEC;
}

PowerMx508::PowerMx508()
    : b_debugOn(false)
{
    FILE *output;

    /* enable the wakeup property of the touchscreen by default.
     * It gets toggled for sleep in setSleepFlag
     */
    output = fopen("/sys/class/input/input1/device/power/wakeup", "w");
    if (!output) {
        qDebug() << Q_FUNC_INFO << "wakeup file missing for input1";
        return;
    }
    fprintf(output, "enabled");
    fclose(output);
#if defined(BATTERY_TEST) || defined(SHOWCASE)
    m_scheduledFLBlinkTime = QDateTime::currentDateTime().addSecs(FRONT_LIGHT_BLINK_PERIOD);
#endif
}

void PowerMx508::powerOff()
{
    qDebug() << Q_FUNC_INFO;

    system("halt");
}

void PowerMx508::reboot()
{
    qDebug() << Q_FUNC_INFO;

    system("reboot");
}

bool PowerMx508::suspend(int autoWakeUpSecs)
{
    qDebug() << Q_FUNC_INFO;

    int ret;
    bool has_slept = false;
    QString wakeup_data;
    QFile *wakeDataFile;

    qDebug() << Q_FUNC_INFO << " autoWakeUpSecs: " << autoWakeUpSecs;
    b_processCanceled = false;

    // If power key is pressed no action to allow forced reset
    if(isPowerKeyPressed()){
        qDebug() << Q_FUNC_INFO << "Suspend prevented to ensure forced power off";
        return false;
    }

    /* Check 1: is the finger on the screen */

    wakeDataFile = new QFile("/sys/class/input/input1/device/power/wakeup_active");
    if (wakeDataFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
        wakeup_data = QString(wakeDataFile->readLine()).trimmed();
        wakeDataFile->close();
    } else {
        qDebug() << Q_FUNC_INFO << "read of wakeup_last failed";
        wakeup_data = "0";
    }
    delete wakeDataFile;

    if (wakeup_data.toInt()) {
        qDebug() << Q_FUNC_INFO << "wakeup event in progress, not suspending";
        return false;
    }

    /* Check 2: was the finger on the screen less than a second ago */

    wakeDataFile = new QFile("/sys/class/input/input1/device/power/wakeup_last_time_ms");
    if (wakeDataFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
        wakeup_data = QString(wakeDataFile->readLine()).trimmed();
        wakeDataFile->close();
        qDebug() << "read wakeup_last" << wakeup_data.toLongLong();
    } else {
        qDebug() << Q_FUNC_INFO << "read of wakeup_last failed";
        wakeup_data = "0";
    }
    delete wakeDataFile;

    if (monotonic_to_msecs() - wakeup_data.toLongLong() < 1000) {
        qDebug() << Q_FUNC_INFO << "recent wakeup event, not suspending";
        return false;
    }


    /* Initial read of the current wakeup_count
     * It blocks when a wakeup event is processed, so we should call processEvents after this
     */
    QString wakeup_count1;
    QString wakeup_count2;
    QFile *wakeFile = new QFile("/sys/power/wakeup_count");
    if (wakeFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
        wakeup_count1 = QString(wakeFile->readLine()).trimmed();
        wakeFile->close();
        qDebug() << "read wakeup_count" << wakeup_count1.toInt();
    } else {
        qDebug() << Q_FUNC_INFO << "read of wakeup_count failed";
    }

    // set autosleep alarm if requested
    if(autoWakeUpSecs > 0)
        RTCManager::setRTCAlarm(autoWakeUpSecs);

#ifdef BATTERY_TEST
    specialTestAction(false);
    if(b_suspendDisabled)
        return true;
#endif

#ifdef SHOWCASE
    /**
    Re-switch on light periodically when
    exposed in closed showcase
    */
    checkAndResetLight();
#endif

    // Check whether it was canceled and then rollback
    QCoreApplication::processEvents();

    /* must write the wake_count again, for the kernel to check for new ones during suspending */
    if (wakeFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
        wakeup_count2 = QString(wakeFile->readLine()).trimmed();
        wakeFile->close();
        qDebug() << "read wakeup_count" << wakeup_count2.toInt();

        if (wakeup_count1.toInt() != wakeup_count2.toInt()) {
            qDebug() << "wakeup_counts differ";
            b_processCanceled = true;
        } else if (wakeFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "writing wakeup_count" << wakeup_count2.toLatin1();
            /* if the write does not succeed, do not suspend */
            if(wakeFile->write(wakeup_count2.toLatin1()) < 0)
                b_processCanceled = true;
            wakeFile->close();
        }
    } else {
        qDebug() << Q_FUNC_INFO << "read of wakeup_count failed";
    }

    if(!b_processCanceled) {
        qDebug() << "really suspending";

        //suspend
        FILE *output = fopen("/sys/power/state","w");
        fprintf( output, "mem");

        /* Writing to sys/power/state generates a close-error
         * when the suspend was aborted by an error.
         */
        ret = fclose(output);
        qDebug() << "suspend fclose returned" << ret;
        has_slept = !ret;

/*-----------------SUSPENDED---------------------------------*/
/* has_slept is true, when the system really slept */
    }

    /* If the suspend was canceled or failed, simply rollback */
    if (b_processCanceled || !has_slept) {
        qDebug() << Q_FUNC_INFO << "Rolling back from suspend";
        if(b_debugOn)
            setLed(true); // Automatically off at suspend/sleep
        return false;
    }

    if(b_debugOn)
        setLed(true); // Automatically off at suspend/sleep
    return true;
}

/// @param autoWakeUpSecs device will auto power off after the time specified by this parameter. Default is 1800 secs
bool PowerMx508::sleepCPU(int autoWakeUpSecs)
{
    qDebug() << Q_FUNC_INFO;

    int ret;
    bool has_slept = false;

    qDebug() << Q_FUNC_INFO << " autoWakeUpSecs: " << autoWakeUpSecs;

    // If power key is pressed no action to allow forced reset
    if(isPowerKeyPressed())
    {
        qDebug() << Q_FUNC_INFO << "Sleep prevented to ensure forced power off";
        return false;
    }

    RTCManager::setRTCAlarm(autoWakeUpSecs);

#ifdef BATTERY_TEST
    specialTestAction(true);
    RTCManager::setRTCAlarm(SAMPLE_PERIOD);
#endif

    // set sleep flag
    setSleepFlag(1);

    //go to sleep
    FILE *output = fopen("/sys/power/state","w");
    fprintf( output, "mem");

    /* Writing to sys/power/state generates a close-error
     * when the suspend was aborted by an error.
     */
    ret = fclose(output);

    qDebug() << "suspend fclose returned" << ret;
    has_slept = !ret;
    
/*-----------------SLEPT---------------------------------*/
/* has_slept is true, when the system really slept */

    if(b_debugOn)
        setLed(true); // Automatically off at suspend/sleep

    setSleepFlag(0);

    /* If the suspend failed, simply rollback */
    if (!has_slept) {
        qDebug() << Q_FUNC_INFO << "Rolling back from sleep";
        return false;
    }

    return true;
}

bool PowerMx508::isRTCwakeUp()
{
    int wakeup_status = 0;
    int retval;
    int FHWhandle = open("/dev/ntx_io", O_RDWR);
    retval = ioctl(FHWhandle, CM_RTC_WAKEUP_FLAG, &wakeup_status);
    close (FHWhandle);

    if (retval < 0)
        return 0;

    qDebug() << Q_FUNC_INFO << wakeup_status;

    return wakeup_status;
}

void PowerMx508::setSleepFlag(int sleepFlag)
{
    qDebug() << Q_FUNC_INFO;

    FILE *output;
    output=fopen("/sys/power/state-extended","w");
    fprintf( output, "%d", sleepFlag);
    fclose(output);

    /* set wakeup property of the touchscreen */
    output = fopen("/sys/class/input/input1/device/power/wakeup", "w");
    if (!output) {
        qDebug() << Q_FUNC_INFO << "wakeup file missing for input1";
        return;
    }
    fprintf(output, sleepFlag ? "disabled" : "enabled");
    fclose(output);
}

void PowerMx508::setDebug(bool status){
    qDebug() << Q_FUNC_INFO;

    b_debugOn = status;
    setLed(status);
}

void PowerMx508::setLed(bool on){
    qDebug() << Q_FUNC_INFO << on;

    int FHWhandle = open("/dev/ntx_io", O_RDWR);
    if(on)
    {
        ioctl(FHWhandle, CM_LED, LED_POWERON);
    }
    else
    {
        ioctl(FHWhandle, CM_LED_BLINK, 0);
        ioctl(FHWhandle, CM_LED, LED_POWEROFF);
    }
    close (FHWhandle);
}

bool PowerMx508::isPowerKeyPressed(){

    int FHWhandle = open("/dev/ntx_io", O_RDWR);
    int value;
    ioctl(FHWhandle, CM_POWER_BTN, &value);
    close (FHWhandle);

//  enable for debugging
//  qDebug() << Q_FUNC_INFO << "value" << value;

    return value;

}
