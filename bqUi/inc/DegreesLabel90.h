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

#ifndef DEGREESLABEL90_H
#define DEGREESLABEL90_H

#include <QLabel>
#include <QFrame>

class QString;

class DegreesLabel90 : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)

public:
    DegreesLabel90(QWidget *parent);
    virtual ~DegreesLabel90();
    void setText(const QString &text);

protected:
    void paintEvent(QPaintEvent *);
    void drawRotatedText(QPainter *painter, float degrees, int x, int y, const QString &text);

private:
    QString content;
};

#endif // DEGREESLABEL90_H
