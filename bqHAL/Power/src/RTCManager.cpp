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

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>

#include <QDateTime>
#include <QDebug>
#include <QProcess>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include "RTCManager.h"

#define DEVICE_RTC                  "/dev/rtc0"
#define REQCODE_RTC_SETALARM        _IOW('p', 0x07, struct rtc_time) /* Set alarm time  */ //TODO: deprecated? its name was RTC_ALM_SET
#define REQCODE_RTC_ALARMS_ON       _IO('p', 0x01)	/* Alarm interruptions enabled */
#define REQCODE_RTC_ALARMS_OFF      _IO('p', 0x02)	/* Alarm interruptions disabled */
#define REQCODE_RTC_SETWAKEALARM    _IOW('p', 0x0f, struct rtc_wkalrm)/* Set wakeup alarm*/
#define REQCODE_RTC_SETTIME         _IOW('p', 0x0a, struct rtc_time_generic) /* Set RTC time    */
#define REQCODE_RTC_READTIME        _IOR('p', 0x09, struct rtc_time_generic) /* Read RTC time   */

// NOTE Precondition
// RTC months: January is 0, December is 11
// RTC years: start at 1900

void RTCManager::setRTCAlarm(int secs)
{
    qDebug() << "--->" << Q_FUNC_INFO << secs;

#ifdef Q_WS_QWS

    int fd_rtc, retval;
    struct rtc_time rtc_tm;
    struct rtc_wkalrm rtc_alrm;
    QDateTime dateTime;
    if(secs <= 0)
        return;

    // Get RTC Time
    fd_rtc = open (DEVICE_RTC, O_WRONLY);
    if (fd_rtc == -1)
    {
        perror(DEVICE_RTC);
    }
    ioctl (fd_rtc, REQCODE_RTC_READTIME, &rtc_tm);

    qDebug() << Q_FUNC_INFO << "Current RTC Time: " << rtc_tm.tm_year << rtc_tm.tm_mon << rtc_tm.tm_mday
             << "-" << rtc_tm.tm_hour << rtc_tm.tm_min << rtc_tm.tm_sec
             << "- wDay " << rtc_tm.tm_wday << "yDay " << rtc_tm.tm_yday;

    // Convert to QDateTime and add seconds
    dateTime.setDate(QDate(rtc_tm.tm_year + 1900, rtc_tm.tm_mon + 1, rtc_tm.tm_mday));
    dateTime.setTime(QTime(rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec));
    dateTime = dateTime.addSecs(secs);

    // Convert to RTC format
    rtc_tm.tm_year = dateTime.date().year() - 1900;
    rtc_tm.tm_mon = dateTime.date().month()-1;
    rtc_tm.tm_mday = dateTime.date().day();
    rtc_tm.tm_wday = dateTime.date().dayOfWeek();
    rtc_tm.tm_yday = dateTime.date().dayOfYear();
    rtc_tm.tm_hour = dateTime.time().hour();
    rtc_tm.tm_min = dateTime.time().minute();
    rtc_tm.tm_sec = dateTime.time().second();

    qDebug() << Q_FUNC_INFO << "Alarm RTC Time: " << rtc_tm.tm_year << rtc_tm.tm_mon << rtc_tm.tm_mday
             << "-" << rtc_tm.tm_hour << rtc_tm.tm_min << rtc_tm.tm_sec
             << "- wDay " << rtc_tm.tm_wday << "yDay " << rtc_tm.tm_yday;

    rtc_alrm.enabled = 1;
    rtc_alrm.pending = 0;
    rtc_alrm.time = rtc_tm;

    retval = ioctl(fd_rtc, REQCODE_RTC_SETWAKEALARM, &rtc_alrm);
    if (retval == -1)
    {
        perror("ioctl - set the alarm");
    }

    //	Enable alarm interrupts
    retval = ioctl(fd_rtc, REQCODE_RTC_ALARMS_ON, 1);
    if (retval == -1)
    {
        perror("ioctl - enable alarm interrupts");
    }

    close(fd_rtc);
#endif

}

void RTCManager::turnOffRTCTimer()
{
#ifdef Q_WS_QWS
    int fd_rtc, retval;

    fd_rtc = open (DEVICE_RTC, O_WRONLY);
    if (fd_rtc == -1)
    {
        perror(DEVICE_RTC);
    }
    retval = ioctl(fd_rtc, REQCODE_RTC_ALARMS_ON, 0);
    if (retval == -1) {
        perror("ioctl - diable alarm interrupts");
    }

    close(fd_rtc);
#endif
}

QDateTime RTCManager::rtcDateTime()
{
    QDate date;
    QTime time;

#ifdef Q_WS_QWS
        int fd = open(DEVICE_RTC, O_RDONLY);
        struct rtc_time rtc_tm;

        if (fd != -1) {
                int retval = ioctl(fd, REQCODE_RTC_READTIME, &rtc_tm);
                if (retval == -1) {
                        perror("REQCODE_RTC_READTIME ioctl");
                }

        date.setDate(rtc_tm.tm_year + 1900,rtc_tm.tm_mon + 1,rtc_tm.tm_mday);
        time.setHMS(rtc_tm.tm_hour,rtc_tm.tm_min,rtc_tm.tm_sec);
        }
        close(fd);
#endif
        return QDateTime(date, time);
}

void RTCManager::setRtcDateTime(const QDateTime& dateTime)
{
    qDebug() << "--->" << Q_FUNC_INFO;
    qDebug() << "DateTime" << dateTime;

#ifdef Q_WS_QWS

    QDate date = dateTime.date();
    QTime time = dateTime.time();

    int fd = open(DEVICE_RTC, O_RDONLY);
    if (fd != -1) {
        struct rtc_time rtc_tm;
        rtc_tm.tm_mday = date.day();
        rtc_tm.tm_mon = date.month() - 1;
        rtc_tm.tm_year = date.year() - 1900;
        rtc_tm.tm_hour = time.hour();
        rtc_tm.tm_min = time.minute();
        rtc_tm.tm_sec = time.second();

        int err = ioctl(fd, REQCODE_RTC_SETTIME, &rtc_tm);
        if (err == -1) {
            qWarning("RTC_SET_TIME failed");
        }
        close(fd);
    }
#endif
}

bool RTCManager::setHardwareDateFromSystem()
{
    qDebug() << Q_FUNC_INFO << ": Setting system date to hwclock";

    QProcess systemDate;
    systemDate.start("hwclock", QStringList() << "--systohc" << "-u");
    systemDate.waitForFinished();

    return !systemDate.exitCode();
}

bool RTCManager::setSystemDateFromHw()
{
    qDebug() << Q_FUNC_INFO << ": Setting system date from hwclock";

#ifndef Q_WS_QWS // No action if emulator
    return true;
#endif

    QProcess systemDate;
    systemDate.start("hwclock", QStringList() << "--hctosys" << "-u");
    systemDate.waitForFinished();

    return !systemDate.exitCode();
}

void RTCManager::syncClock()
{
    system("hwclock");
}
