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

#include "MouseFilter.h"

#include "QBook.h"

#include <QDebug>
#include <QWidget>
#include <QEvent>
#include <QObject>
#include <QMouseEvent>
#include <QObject>
#include <QCoreApplication>
#include <sys/time.h>
#include <QEventLoop>
#include <QTimer>
#include <QWheelEvent>
#include <Qt>
#include <QElapsedTimer>

MouseFilter::MouseFilter(QObject *parent):
    QObject(parent),
    b_isLongPressing(false),
    i_swipe_min_length(SWIPE_MIN_LENGTH),
    m_LongpressTimer(this)
{
    qDebug() << "--->" << Q_FUNC_INFO;

    connect(&m_LongpressTimer,SIGNAL(timeout()),this,SLOT(processLongpress()));

    m_LongpressTimer.setInterval(LONG_PRESS_INTERVAL);

    switch(QBook::getInstance()->getResolution()){
    case QBook::RES600x800:
        i_swipe_min_length = SWIPE_MIN_LENGTH;
        break;
    case QBook::RES758x1024:
        i_swipe_min_length = SWIPE_MIN_LENGTH_HD;
        break;
    case QBook::RES1072x1448:
        i_swipe_min_length = SWIPE_MIN_LENGTH_FHD;
        break;
    default:
        i_swipe_min_length = SWIPE_MIN_LENGTH;
    }

}

MouseFilter::~MouseFilter()
{
    /// Do nothing
    qDebug() << "--->" << Q_FUNC_INFO;
}

bool MouseFilter::eventFilter(QObject *watchedObj, QEvent *event)
{

    QMouseEvent *mouseEvent;

    switch (event->type()){

    case QEvent::MouseButtonPress:
    {

        // Store event data
        mouseEvent = static_cast<QMouseEvent*>(event);
        m_initialPoint = mouseEvent->pos();
        m_currentPoint = mouseEvent->pos();
        m_pCurrentObj = watchedObj;

        // Start timers
        m_LongpressTimer.start();
        m_elapsedTimer.start();

        return true;

        break;
    }

    case QEvent::MouseButtonRelease:
    {
        mouseEvent = static_cast<QMouseEvent*>(event);
        m_currentPoint = mouseEvent->pos();

        if(m_LongpressTimer.isActive()){ // Long press timer not expired
            m_LongpressTimer.stop();

            TouchEventType evType = getTouchType();

            switch(evType){
            case SWIPE_R2L:
            case SWIPE_L2R:
            case SWIPE_D2U:
            case SWIPE_U2D:{
                QPoint registeredMovement = m_currentPoint - m_initialPoint;
                QEvent* myEvent = new TouchEvent(m_initialPoint,evType,registeredMovement);
                QCoreApplication::postEvent(watchedObj,myEvent);
                break;
            }
            case UNKNOWN_GESTURE:
                qWarning() << Q_FUNC_INFO << "UNKNOWN_GESTURE";
                break;
            default:{
                QEvent* myEvent = new TouchEvent(m_initialPoint,evType);
                QCoreApplication::postEvent(watchedObj,myEvent);

            }
            }
            qDebug() << Q_FUNC_INFO << "RELEASE-TAP/SWIPE";
        }
        else if (b_isLongPressing) { // Long press ongoing
            qDebug() << "<---" << Q_FUNC_INFO << "RELEASE-LONGPRESS END";

            QEvent* myEvent = new TouchEvent(m_currentPoint,MouseFilter::LONGPRESS_END);
            b_isLongPressing = false;
            QCoreApplication::postEvent(watchedObj,myEvent);
        }
        else{ // Others
            qDebug() << Q_FUNC_INFO << "RELEASE FILTERED";
        }

        return true;
        break;
    }

    case QEvent::MouseMove:

        mouseEvent = static_cast<QMouseEvent*>(event);

        if(m_LongpressTimer.isActive()){ // Long pressing process NOT started

            // Determine whether the movement is long enough to be considered as a change
            QPoint newPoint = mouseEvent->pos();
            QPoint movement = newPoint - m_currentPoint;

            // less than 10 pixels is ignored
            if (movement.manhattanLength() > 10){
                m_LongpressTimer.start(); // Restart count
                m_currentPoint =  newPoint; // Point reassigned
            }
            return true;
        }
        else if (b_isLongPressing){ // forward event when longpressing
            return QObject::eventFilter(watchedObj,event); // Allow
        }
        else{ // filter otherwise
            qWarning() << Q_FUNC_INFO << "MOVE-IGNORE";
            return true;
        }

        break;

    default:
        return QObject::eventFilter(watchedObj,event);

    }
}

void MouseFilter::processLongpress()
{
    qDebug() << "--->" << Q_FUNC_INFO << "LONGPRESS START" << m_currentPoint;

    //Don't allow long press on footer
    if(m_pCurrentObj->objectName()=="pageWindow")
    {
        qDebug() << "--->" << Q_FUNC_INFO << "Don't allow long press on footer";
        return;
    }

    m_LongpressTimer.stop();
    b_isLongPressing = true;

    //Send Event
    QEvent *myEvent = new TouchEvent(m_currentPoint,MouseFilter::LONGPRESS_START);
    QCoreApplication::postEvent(m_pCurrentObj,myEvent);
}

MouseFilter::TouchEventType MouseFilter::getTouchType(){

    int elapsedTime = m_elapsedTimer.elapsed();
    QPoint moveVector = m_currentPoint - m_initialPoint;
    int absX = qAbs(moveVector.x());
    int absY = qAbs(moveVector.y());

    qDebug() << Q_FUNC_INFO << "ELAPSED" << elapsedTime
             << "ms, MOVED " << moveVector;

    int maxAxisMove = qMax(absX,absY);
    if(maxAxisMove >= i_swipe_min_length){

        if(elapsedTime > SWIPE_MAX_TIME) // Too slow movement
            return UNKNOWN_GESTURE;

        if(absX > absY) // Horizontal
            if(moveVector.x() < 0)
                return SWIPE_R2L;
            else
                return SWIPE_L2R;

        else // Vertical
            if(moveVector.y() < 0)
                return SWIPE_D2U;
            else
                return SWIPE_U2D;
    }
    else
        return TAP;
}


//#######################################################//

TouchEvent::TouchEvent(QPoint pos,int type, QPoint movement):
    QEvent(QEvent::Type(MouseFilter::TOUCH_EVENT))
{
    //qDebug() << "--->" << Q_FUNC_INFO;

    m_pos = pos;
    i_touchType = type;
    m_movement = movement;
}

TouchEvent::~TouchEvent()
{
    //qDebug() << "--->" << Q_FUNC_INFO;

}

QPoint TouchEvent::pos(){
    //qDebug() << "--->" << Q_FUNC_INFO;

    return m_pos;
}

QPoint TouchEvent::movement(){
    //qDebug() << "--->" << Q_FUNC_INFO;

    return m_movement;
}

int TouchEvent::touchType(){
    //qDebug() << "--->" << Q_FUNC_INFO;

    return i_touchType;
}

