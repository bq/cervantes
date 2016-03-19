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

#ifndef SILENTQPUSHBUTTON_H
#define SILENTQPUSHBUTTON_H

#include <QPushButton>
#include <QMouseEvent>

class SilentQPushButton  : public QPushButton{

    Q_OBJECT

public:
                    SilentQPushButton                (QWidget*);
                    virtual ~SilentQPushButton               ();

protected:

 virtual void	mousePressEvent(QMouseEvent * e);
 virtual void	mouseReleaseEvent(QMouseEvent * e);

};

#endif // SILENTQPUSHBUTTON_H
