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

#ifndef LONGPRESSEDQPUSHBUTTON_H
#define LONGPRESSEDQPUSHBUTTON_H

#include <QPushButton>
#include <QTimer>

class LongPressedQPushButton : public QPushButton
{
    Q_OBJECT

public:
    LongPressedQPushButton(QWidget*);
    virtual ~LongPressedQPushButton();

    void setLongPressDelay(int msec);
    void setLongPressInterval(int msec);

signals:
    void longPressed();

protected slots:
    void handleLongPressTimerEvent ();

protected:
    virtual void mousePressEvent (QMouseEvent*);
    virtual void mouseReleaseEvent (QMouseEvent*);

    QTimer m_longPressTimer;
    int m_longPressTimerDelay;
};

#endif // LONGPRESSEDQPUSHBUTTON_H
