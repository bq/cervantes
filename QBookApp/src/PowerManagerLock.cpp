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

#include "PowerManager.h"
#include "PowerManagerLock.h"

#include <QBasicTimer>
#include <QTimerEvent>

PowerManagerLock::PowerManagerLock(QObject* requester)
{
    m_requester = requester;
    i_timeOutMsecs = 0;
}

PowerManagerLock::~PowerManagerLock()
{
    PowerManager::removeLock(this);
}

void PowerManagerLock::setTimeOut(int seconds)
{
    i_timeOutMsecs = seconds * 1000;
}

void PowerManagerLock::activate()
{
    PowerManager::activateLock(this);

    if(i_timeOutMsecs > 0)
        m_timer_timeout.start(i_timeOutMsecs,this);
}

void PowerManagerLock::release()
{
    m_timer_timeout.stop();
    PowerManager::releaseLock(this);
}

bool PowerManagerLock::isActive()
{
    return PowerManager::isLockActive(this);
}

void PowerManagerLock::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer_timeout.timerId()) {        
        event->accept();
        emit lockTimedOut();
        release();
    } else
        QObject::timerEvent(event);
}
