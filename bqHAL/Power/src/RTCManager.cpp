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
#include "RTCManager.h"

#define DEVICE_RTC                  "/dev/rtc0"
#define REQCODE_RTC_SETALARM        _IOW('p', 0x07, struct rtc_time) /* Set alarm time  */ //TODO: deprecated? its name was RTC_ALM_SET
#define REQCODE_RTC_ALARMS_ON       _IO('p', 0x01)	/* Alarm interruptions enabled */
#define REQCODE_RTC_ALARMS_OFF      _IO('p', 0x02)	/* Alarm interruptions disabled */
#define REQCODE_RTC_SETWAKEALARM    _IOW('p', 0x0f, struct rtc_wkalrm)/* Set wakeup alarm*/
#define REQCODE_RTC_SETTIME         _IOW('p', 0x0a, struct rtc_time_generic) /* Set RTC time    */
#define REQCODE_RTC_READTIME        _IOR('p', 0x09, struct rtc_time_generic) /* Read RTC time   */

void RTCManager::setRTCAlarm(int secs)
{
    qDebug() << "--->" << Q_FUNC_INFO;

#ifdef Q_WS_QWS

    int fd_rtc, retval;
    struct rtc_time rtc_tm;
    struct rtc_wkalrm rtc_alrm;

    if(secs <= 0)
        return;

    // Set RTC interrupt
    fd_rtc = open (DEVICE_RTC, O_WRONLY);
    if (fd_rtc == -1)
    {
        perror(DEVICE_RTC);
    }
    ioctl (fd_rtc, REQCODE_RTC_READTIME, &rtc_tm);

    time_t t_secs = mymktime((struct tm *)(void *)&rtc_tm);

    //qDebug("[RTC]read rtc %d/%d/%d  %d:%d:%d", rtc_tm.tm_year, rtc_tm.tm_mon, rtc_tm.tm_mday,rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
    //qDebug("[RTC] t_secs= %d, secs = %d", (int)t_secs, secs);
    t_secs = t_secs+secs;
    rtc_tm=*( (struct rtc_time*)mygmtime(&t_secs));

    //qDebug("[RTC] t_secs= %d", (int)t_secs);
    //qDebug("[RTC] set rtc %d/%d/%d  %d:%d:%d", rtc_tm.tm_year, rtc_tm.tm_mon, rtc_tm.tm_mday,rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);


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

bool RTCManager::checkRTCTime()
{
#ifdef Q_WS_QWS
    struct rtc_time rtc_tm;
    int fd_rtc;

    fd_rtc = open (DEVICE_RTC, O_WRONLY);
    if (fd_rtc == -1)
    {
        perror(DEVICE_RTC);
    }
    ioctl (fd_rtc, REQCODE_RTC_READTIME, &rtc_tm);
    close(fd_rtc);

    if( (rtc_tm.tm_year<100 || rtc_tm.tm_year>=200) ||
            (rtc_tm.tm_sec >= 60 || rtc_tm.tm_sec < 0) ||
            (rtc_tm.tm_min >= 60 || rtc_tm.tm_min < 0) ||
            (rtc_tm.tm_mon > 11 || rtc_tm.tm_mon < 0) ||
            (rtc_tm.tm_hour >= 24 || rtc_tm.tm_hour < 0) ||
            rtc_tm.tm_mday < 1 || rtc_tm.tm_mday > rtc_month_days(rtc_tm.tm_mon, rtc_tm.tm_year + 1900))
    {
        return false;
    }
#endif

    return true;
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

/** ---- Supporting functions for RTC ---- **/

static struct tm _mytm;

static int _DMonth[13] = { 0,31,59,90,120,151,181,212,243,273,304,334,365 };
static int _monthCodes[12] = { 6, 2, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };

static int
mycalcDayOfWeek(const struct tm* nTM)
{
    int day;

    day = (nTM->tm_year%100);
    day += day/4;
    day += _monthCodes[nTM->tm_mon];
    day += nTM->tm_mday;
    while(day>=7)
        day -= 7;

    return day;
}


static struct tm *
mygmtime(const time_t *timer)
{
    unsigned long x = *timer;
    int imin, ihrs, iday, iyrs;
    int sec, min, hrs, day, mon, yrs;
    int lday, qday, jday, mday;


    imin = x / 60;							// whole minutes since 1/1/70
    sec = x - (60 * imin);					// leftover seconds
    ihrs = imin / 60;						// whole hours since 1/1/70
    min = imin - 60 * ihrs;					// leftover minutes
    iday = ihrs / 24;						// whole days since 1/1/70
    hrs = ihrs - 24 * iday;					// leftover hours
    iday = iday + 365 + 366; 				// whole days since 1/1/68
    lday = iday / (( 4* 365) + 1);			// quadyr = 4 yr period = 1461 days
    qday = iday % (( 4 * 365) + 1);			// days since current quadyr began
    if(qday >= (31 + 29))					// if past feb 29 then
        lday = lday + 1;					// add this quadyrs leap day to the
    // # of quadyrs (leap days) since 68
    iyrs = (iday - lday) / 365;				// whole years since 1968
    jday = iday - (iyrs * 365) - lday;		// days since 1 /1 of current year.
    if(qday <= 365 && qday >= 60)			// if past 2/29 and a leap year then
        jday = jday + 1;					// add a leap day to the # of whole
    // days since 1/1 of current year
    yrs = iyrs + 1968;						// compute year
    mon = 13;								// estimate month ( +1)
    mday = 366;								// max days since 1/1 is 365
    while(jday < mday)						// mday = # of days passed from 1/1
    {										// until first day of current month
        mon = mon - 1;						// mon = month (estimated)
        mday = _DMonth[mon];					// # elapsed days at first of mon?
        if((mon > 2) && (yrs % 4) == 0)		// if past 2/29 and leap year then
            mday = mday + 1;				// add leap day
        // compute month by decrementing
    }										// month until found

    day = jday - mday + 1;					// compute day of month

    _mytm.tm_sec = sec;
    _mytm.tm_min = min;
    _mytm.tm_hour = hrs;
    _mytm.tm_mday = day;
    _mytm.tm_mon = mon;
    _mytm.tm_year = yrs - 1900;

    _mytm.tm_wday = mycalcDayOfWeek(&_mytm);
    _mytm.tm_yday = jday;
    _mytm.tm_isdst = 0;

    return &_mytm;
}

static int _month_to_day[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

time_t mymktime(struct tm *t)
{
    short  month, year;
    time_t result;

    month = t->tm_mon;
    year = t->tm_year + month / 12 + 1900;
    month %= 12;
    if (month < 0)
    {
        year -= 1;
        month += 12;
    }
    result = (year - 1970) * 365 + (year - 1969) / 4 + _month_to_day[month];
    result = (year - 1970) * 365 + _month_to_day[month];
    if (month <= 1)
        year -= 1;
    result += (year - 1968) / 4;
    result -= (year - 1900) / 100;
    result += (year - 1600) / 400;
    result += t->tm_mday;
    result -= 1;
    result *= 24;
    result += t->tm_hour;
    result *= 60;
    result += t->tm_min;
    result *= 60;
    result += t->tm_sec;
    return(result);
}

static const unsigned char rtc_days_in_month[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

#define LEAP_YEAR(year) ((!(year % 4) && (year % 100)) || !(year % 400))
int rtc_month_days(unsigned int month, unsigned int year)
{
        return rtc_days_in_month[month] + (LEAP_YEAR(year) && month == 1);
}

