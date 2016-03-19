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

#ifndef VIEWERBOOKMARKLANDSCAPE_H
#define VIEWERBOOKMARKLANDSCAPE_H

#include "ui_BookmarkLandscape.h"
#include <QPaintEvent>

class QWidget;
class QLabel;

class ViewerBookmarkLandscape : public QWidget, protected Ui::BookmarkLandscape
{
    Q_OBJECT

public:
    ViewerBookmarkLandscape(QWidget*);
    virtual ~ViewerBookmarkLandscape();
    void mousePressEvent (QMouseEvent*);

protected:
    virtual void                        paintEvent                          ( QPaintEvent* );

public slots:
    void                                updateDisplay                       ( bool );

signals:
    void changeBookmark();

};

#endif // VIEWERBOOKMARKLANDSCAPE_H
