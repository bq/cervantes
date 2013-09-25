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

#ifndef HOVERWIDGET_H
#define HOVERWIDGET_H

#include <QWidget>

class HoverWidget : public QWidget
{
    Q_OBJECT

public:
    HoverWidget(QWidget*);
    virtual ~HoverWidget();

protected:
    virtual bool eventFilter(QObject*, QEvent*);
    virtual void mousePressEvent(QMouseEvent*);
//    virtual void dragEnterEvent(QDragEnterEvent*);
    void paintEvent(QPaintEvent *event);

signals:
    void    over    (int);

private:
    bool m_on;

};

#endif // HOVERWIDGET_H
