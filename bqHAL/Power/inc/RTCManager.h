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

#ifndef RTCMANAGER_H
#define RTCMANAGER_H

#include <QObject>
#include <time.h>

class QDateTime;

class RTCManager : public QObject
{
    Q_OBJECT

    public:
        struct rtc_time {
            int tm_sec;
            int tm_min;
            int tm_hour;
            int tm_mday;
            int tm_mon;
            int tm_year;
            int tm_wday;
            int tm_yday;
            int tm_isdst;
        };

        static void setRTCAlarm(int secs);
        static void turnOffRTCTimer();
        static QDateTime rtcDateTime();
        static void setRtcDateTime(const QDateTime&);
        static bool setHardwareDateFromSystem();
        static bool setSystemDateFromHw();
        static void syncClock();

        struct rtc_time_generic {
            int tm_sec;
            int tm_min;
            int tm_hour;
            int tm_mday;
            int tm_mon;
            int tm_year;
            int tm_wday;
            int tm_yday;
            int tm_isdst;
        };


        struct rtc_wkalrm {
            unsigned char enabled;	/* 0 = alarm disabled, 1 = alarm enabled */
            unsigned char pending;  /* 0 = alarm not pending, 1 = alarm pending */
            struct rtc_time time;	/* time the alarm is set to */
        };
};

#endif // RTCMANAGER_H
