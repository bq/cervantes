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

#ifndef DEGREESPUSHBUTTON_H
#define DEGREESPUSHBUTTON_H

#include <QPushButton>

class QString;

class DegreesPushButton : public QPushButton
{
    Q_OBJECT

public:
    DegreesPushButton(QWidget *parent);
    virtual ~DegreesPushButton();

protected:
    void paintEvent(QPaintEvent *);
    void drawRotatedText(QPainter *painter, float degrees, int x, int y, const QString &text);

private:
    uint    alignment;
};

#endif // DEGREESPUSHBUTTON_H
