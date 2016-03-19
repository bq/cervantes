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

#ifndef MOUSEFILTER_H
#define MOUSEFILTER_H

#include <QWidget>
#include <QTimer>
#include <QPoint>
#include <QEvent>
#include <QElapsedTimer>

#define LONG_PRESS_INTERVAL     300
#define SWIPE_MIN_LENGTH        32      // Set by tuning
#define SWIPE_MIN_LENGTH_HD     40      // Set by tuning
#define SWIPE_MAX_TIME          600

class QObject;

class MouseFilter : public QObject
{
    Q_OBJECT

public:
    MouseFilter(QObject*);
    virtual ~MouseFilter();

    enum TouchEventType{
        TAP,
        LONGPRESS_START,
        LONGPRESS_END,
        SWIPE_R2L,
        SWIPE_L2R,
        SWIPE_D2U,
        SWIPE_U2D,
        UNKNOWN_GESTURE
    };

    enum EventNumber{
        TOUCH_EVENT = 6000
    };

protected slots:
    void processLongpress();

protected:
    virtual bool eventFilter(QObject*, QEvent*);
    TouchEventType getTouchType();

    bool b_isLongPressing;

    int i_swipe_min_length;

    QObject* m_pCurrentObj;

    QPoint m_initialPoint;
    QPoint m_currentPoint;

    QTimer m_LongpressTimer;
    QElapsedTimer m_elapsedTimer;


};

//#######################################################//

class TouchEvent : public QEvent
{
    // Q_OBJECT

 public :
    TouchEvent(QPoint pos, int type, QPoint movement = QPoint(0,0));
     virtual ~TouchEvent();

     QPoint pos();
     QPoint movement();
     int touchType();

 protected:
     QPoint m_pos, m_movement;
     int i_touchType;
};
#endif // MOUSEFILTER_H
