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

#include "LongPressedQPushButton.h"

#include <QDebug>

LongPressedQPushButton::LongPressedQPushButton (QWidget* parent) : QPushButton(parent)
  , m_longPressTimerDelay(450)
{
    qDebug() << Q_FUNC_INFO;

    m_longPressTimer.setInterval(100);

    connect(&m_longPressTimer, SIGNAL(timeout()), this, SLOT(handleLongPressTimerEvent()));
}

LongPressedQPushButton::~LongPressedQPushButton()
{
    qDebug() << Q_FUNC_INFO;
}

void LongPressedQPushButton::setLongPressDelay(int msec)
{
    qDebug() << Q_FUNC_INFO;

    m_longPressTimerDelay = msec;
}

void LongPressedQPushButton::setLongPressInterval(int msec)
{
    qDebug() << Q_FUNC_INFO;

    m_longPressTimer.setInterval(msec);
}

void LongPressedQPushButton::handleLongPressTimerEvent()
{
    qDebug() << Q_FUNC_INFO;

    m_longPressTimer.stop();

    emit longPressed();
}

void LongPressedQPushButton::mousePressEvent( QMouseEvent* event )
{
    qDebug() << Q_FUNC_INFO;

    m_longPressTimer.start(m_longPressTimerDelay);

    QPushButton::mousePressEvent(event);
}

void LongPressedQPushButton::mouseReleaseEvent(QMouseEvent * event)
{
    qDebug() << Q_FUNC_INFO;

    m_longPressTimer.stop();

    QPushButton::mouseReleaseEvent(event);
}

