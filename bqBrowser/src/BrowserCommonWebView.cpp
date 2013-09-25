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

#include "BrowserCommonWebView.h"

#include "QBook.h"
#include "Screen.h"
#include "QBookApp.h"
#include "Keyboard.h"

#include <QDebug>
#include <QApplication>
#include <QTimer>
#include <QSettings>
#include <QWebView>
#include <QWidget>
#define DRAG_THRESHOLD 15
#define DRAG_MID_THRESHOLD 150
#define DRAG_DIS_MID 1200
#define DRAG_LONG_THRESHOLD 300
#define DRAG_DIS_LONG 12000
#define DRAG_DIS_UNIT 120

BrowserCommonWebView::BrowserCommonWebView(QWidget* parent):
    QWebView(parent)
  , mDragDistance(0)
  , mMouseDragDistance(0)
  , alreadyMoved(false)
  , alreadyPressed(false)
  , mMousePressEvent(NULL)
{}

BrowserCommonWebView::~BrowserCommonWebView()
{
    if(mMousePressEvent != NULL){
        delete mMousePressEvent;
        mMousePressEvent = NULL;
    }
}

void BrowserCommonWebView::resizeEvent(QResizeEvent * event)
{
    qDebug()<<"BrowserCommonWebView::resizeEvent";
    QSize aSize = event->size();
    emit newSize(aSize);
    QWebView::resizeEvent(event);
}

void BrowserCommonWebView::mousePressEvent(QMouseEvent * event )
{
    qDebug()<< Q_FUNC_INFO;

    alreadyMoved = false;
    alreadyPressed = false;
    if(mMousePressEvent)
    {
        qDebug()<< Q_FUNC_INFO << "not null";
        delete mMousePressEvent;
        mMousePressEvent = NULL;
    }
    mMousePressEvent = new QMouseEvent(*event);
    alreadyPressed = true;
}

void BrowserCommonWebView::mouseMoveEvent ( QMouseEvent * event )
{
    //qDebug()<< Q_FUNC_INFO << event->x() << " " << event->y();

    if(!mMousePressEvent || !alreadyPressed) return;

    QPoint pt(event->x(),event->y());
    QPoint pressPt(mMousePressEvent->x(),mMousePressEvent->y());

    int dx = pt.x() - pressPt.x();
    int dy = pt.y() - pressPt.y();

    if( abs(dx) > DRAG_THRESHOLD || abs(dy) > DRAG_THRESHOLD)
    {
        alreadyMoved = true;
        mDragStartPos = pressPt;
        mDragEndPos = pt;
        doWheel(getWheelWay());

        delete mMousePressEvent;
        mMousePressEvent = new QMouseEvent(*event);
    }
    else{
        qDebug() << Q_FUNC_INFO;
        event->accept();
        alreadyMoved = false;
    }
}

void BrowserCommonWebView::mouseReleaseEvent( QMouseEvent * event )
{
    qDebug() << Q_FUNC_INFO;

    if(alreadyMoved)
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);

    alreadyPressed = false;
    QPoint pressPt(mMousePressEvent->x(),mMousePressEvent->y());

    if(!alreadyMoved){
        newPressPos(pressPt);
        QWebView::mousePressEvent(mMousePressEvent);
    }

    emit newReleasePos(pressPt);
    QWebView::mouseReleaseEvent(event);
    qDebug() << Q_FUNC_INFO << " url" << url();

}

QWebView * BrowserCommonWebView::createWindow ( QWebPage::WebWindowType type )
{
    int nWebWindowType = type;
    qDebug()<<"BrowserCommonWebView::createWindow,type = "<< nWebWindowType;
    QWebView * pNewWebView = this;
    return pNewWebView;
}

void BrowserCommonWebView::doWheels()
{
    qDebug() <<  Q_FUNC_INFO;
    int nWay = mDragVertical ? Qt::Vertical : Qt::Horizontal;
    QWheelEvent e(mDragStartPos,mDragDistance,Qt::MidButton,Qt::NoModifier,(Qt::Orientation)nWay);
    QApplication::sendEvent(this, &e);  
}

void BrowserCommonWebView::wheelEvent(QWheelEvent* event)
{
    qDebug() << Q_FUNC_INFO << event->delta();
    QWebView::wheelEvent(event);

}

BrowserCommonWebView::WheelWay BrowserCommonWebView::getWheelWay()
{   
    qDebug() << Q_FUNC_INFO;
    WheelWay way = WHEEL_NONE;
    int dx = mDragEndPos.x() - mDragStartPos.x();
    int dy = mDragEndPos.y() - mDragStartPos.y();

    if(abs(dx) < DRAG_THRESHOLD && abs(dy) < DRAG_THRESHOLD)
        return way;

    mDragVertical = true;
    mMouseDragDistance = 0;
    mDragDistance = DRAG_DIS_UNIT;
    int nDis = 0;

    if(abs(dx)>abs(dy))
    {
        mDragVertical = false;
        nDis = abs(dx);
    }
    else{
        mDragVertical = true;
        nDis = abs(dy);
    }

    mMouseDragDistance = nDis;
    if(nDis > DRAG_LONG_THRESHOLD)
        mDragDistance = DRAG_DIS_LONG;

    else if(nDis > DRAG_MID_THRESHOLD)
        mDragDistance = DRAG_DIS_MID;

    qDebug()<< Q_FUNC_INFO << "mDragDistance=" << mDragDistance;
    if(mDragVertical == true )
    {
        if(dy < 0)
        {
            way = WHEEL_DOWN;
            mDragDistance = (0-mDragDistance);
        }
        else
        {
            way = WHEEL_UP;
            mMouseDragDistance = (0-mMouseDragDistance);
        }
    }
    else
    {
        if(dx < 0)
        {
            way = WHEEL_RIGHT;
            mDragDistance = (0-mDragDistance);
        }
        else
        {
            way = WHEEL_LEFT;
            mMouseDragDistance = (0-mMouseDragDistance);
        }
    }
    qDebug()<< Q_FUNC_INFO << "Way:"  << way;
    return way;
}

void BrowserCommonWebView::doWheel(WheelWay way)
{
    qDebug() << Q_FUNC_INFO;

    Keyboard * keyboard = QBookApp::instance()->getKeyboard();
    if(keyboard && keyboard->isVisible()){
        qDebug() << Q_FUNC_INFO << "keyboard shown";
        return;
    }
    switch(way)
    {
    case WHEEL_DOWN:
    case WHEEL_LEFT:
    case WHEEL_UP:
    case WHEEL_RIGHT:
        QTimer::singleShot(0, this, SLOT(doWheels()));
        qDebug() << Q_FUNC_INFO << "Qemit wheel";
        emit wheel(mDragVertical,mMouseDragDistance);
        break;
    default:
        qDebug() << Q_FUNC_INFO << "default,way = " << (int)way;
        break;
    }
}

void BrowserCommonWebView::keyReleaseEvent(QKeyEvent* event)
{
    qDebug() << Q_FUNC_INFO << "key " << event->key();
    if (event->key() == QBook::QKEY_BACK) {
        qDebug() << Q_FUNC_INFO << "ignore";
        event->ignore();
    } else {
        qDebug() << Q_FUNC_INFO;
        QWebView::keyReleaseEvent(event);
    }
}
