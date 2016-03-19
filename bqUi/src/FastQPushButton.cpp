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

#include "FastQPushButton.h"
#include "Screen.h"
#include <QDebug>
#include <QWidget>
#include <QMouseEvent>
#include <QPushButton>
#include <QApplication>
#include "PowerManager.h"


FastQPushButton::FastQPushButton (QWidget* parent) : QPushButton(parent){
        qDebug() << Q_FUNC_INFO;
        m_powerLock = PowerManager::getNewLock(this);
}

FastQPushButton::~FastQPushButton (){
        qDebug() << Q_FUNC_INFO;
        delete m_powerLock;
        m_powerLock = NULL;
}

void FastQPushButton::mousePressEvent(QMouseEvent * event){
        qDebug() << Q_FUNC_INFO;
        event->accept();
        m_powerLock->activate();
        Screen::getInstance()->setMode(Screen::MODE_FASTEST,true,FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
        setDown(true);
        QApplication::flush();
        emit pressed();

}

void FastQPushButton::mouseReleaseEvent(QMouseEvent * event){
        qDebug() << Q_FUNC_INFO;
        event->accept();
        Screen::getInstance()->setMode(Screen::MODE_FASTEST,true,FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
        setDown(false);
        m_powerLock->release();
        emit released();
        emit clicked();
}
