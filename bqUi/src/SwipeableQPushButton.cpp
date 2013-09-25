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

#include "SwipeableQPushButton.h"
#include "Screen.h"
#include "QBook.h"
#include <QDebug>
#include <QWidget>
#include <QMouseEvent>
#include <QPushButton>
#include <QApplication>

#define MIN_MOVE_SD 25
#define MIN_MOVE_HD 33

SwipeableQPushButton::SwipeableQPushButton (QWidget* parent) : QPushButton(parent){
        qDebug() << Q_FUNC_INFO;
        setCheckable(true);
}

SwipeableQPushButton::~SwipeableQPushButton (){
        qDebug() << Q_FUNC_INFO;
}

void SwipeableQPushButton::mousePressEvent(QMouseEvent * event){
        qDebug() << Q_FUNC_INFO;
        event->accept();
        last_pos = event->pos().x();
        emit pressed();
}

void SwipeableQPushButton::mouseMoveEvent(QMouseEvent * event){
        qDebug() << Q_FUNC_INFO << isChecked();
        int minMove;
        if(QBook::getInstance()->getResolution() == QBook::RES758x1024)
            minMove = MIN_MOVE_HD;
        else
            minMove = MIN_MOVE_SD;
        if(event->pos().x() > last_pos + minMove && !isChecked()) {
            event->accept();
            emit clicked();
        }
        else if(event->pos().x() < last_pos - minMove && isChecked()) {
            event->accept();
            emit clicked();
        }
}

void SwipeableQPushButton::mouseReleaseEvent(QMouseEvent * event){
        qDebug() << Q_FUNC_INFO;
        int minMove;
        if(QBook::getInstance()->getResolution() == QBook::RES758x1024)
            minMove = MIN_MOVE_HD;
        else
            minMove = MIN_MOVE_SD;
        if(event->pos().x() < last_pos + minMove && event->pos().x() > last_pos - minMove ) {
            event->accept();
            emit clicked();
        }
}
