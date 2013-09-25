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

#include "LongPressWidget.h"

#include "QBookApp.h"
#include "MouseFilter.h"

LongPressWidget::LongPressWidget(QWidget *parent) :
    QWidget(parent)
{
    qDebug() << "--->" << Q_FUNC_INFO;

    m_mouseFilter = new MouseFilter(this);
    this->installEventFilter(m_mouseFilter);
}

LongPressWidget::~LongPressWidget()
{
    /// Do nothing
    qDebug() << "--->" << Q_FUNC_INFO;
}

void LongPressWidget::customEvent(QEvent* received){

    if (received->type() != MouseFilter::TOUCH_EVENT){
        qDebug() << Q_FUNC_INFO << "UNEXPECTED TYPE";
        QWidget::customEvent(received);
        return;
    }

    TouchEvent *event = static_cast<TouchEvent*>(received);

    switch(event->touchType()){

    case MouseFilter::TAP:{
        qDebug() << Q_FUNC_INFO << "TAP";
        emit tap();
        return;
    }

    case MouseFilter::LONGPRESS_START:{
        qDebug() << Q_FUNC_INFO << "LONGPRESS_START";
        emit longPressStart();
        return;
    }

    case MouseFilter::LONGPRESS_END:{
        qDebug() << Q_FUNC_INFO << "LONGPRESS_END";
        longpressEnd();
        return;
    }

    default:
        qDebug() << "--->" << Q_FUNC_INFO << "UNEXPECTED EXIT";
        break;
    }
    QWidget::customEvent(received);
}
