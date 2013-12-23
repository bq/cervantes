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

#ifndef POWERMANAGERDEFS_H
#define POWERMANAGERDEFS_H

#define POWERMANAGER_AWAKE_MINIMUM_TIME     2000    // ms, awake due to any awake reaon
#define POWERMANAGER_AWAKE_USER_TIME        2000    // ms, after any user interaction
#define POWERMANAGER_UPDATE_CLOCK_TIME      60      // seconds, auto-wake up time to update clock
#define POWERMANAGER_TIME_AUTOSLEEPSECS     600     // seconds, default auto-sleep value
#define POWERMANAGER_TIME_POWEROFFSECS      259200  // seconds, default auto-poweroff value
#define POWERMANAGER_SLEEP_WAKEUP_PERIOD    7200    // seconds, periodic wake up in sleep
#define POWERMANAGER_PWROFF_LONGPRESS       3000    // miliseconds, duration to poweroff with key
#define POWERMANAGER_SLEEP_MAX_REATTEMPTS   5       // reattempts, number of reattempts when faulty sleep
#define MAX_TIME_RTC_REATTS                 5000    // ms, maximum time reattempting to set time after sleep

#endif // POWERMANAGERDEFS_H
