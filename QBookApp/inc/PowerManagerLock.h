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

#ifndef POWERMANAGERLOCK_H
#define POWERMANAGERLOCK_H

#include <QObject>
#include <QBasicTimer>

class QBasicTimer;
class QTimerEvent;

class PowerManagerLock : public QObject
{
    Q_OBJECT

public:
    PowerManagerLock(QObject* requester = NULL);
    ~PowerManagerLock();
    bool isActive();
    void setTimeOut(int);

public slots:
    void activate();
    void release();

signals:
    void lockTimedOut();

private:
    virtual void timerEvent(QTimerEvent*);
    bool b_active;
    int i_timeOutMsecs;
    QBasicTimer m_timer_timeout;
    QObject* m_requester;
};

#endif // POWERMANAGERLOCK_H
