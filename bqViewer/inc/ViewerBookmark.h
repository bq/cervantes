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

#ifndef VIEWERBOOKMARK_H
#define VIEWERBOOKMARK_H

#include "ui_Bookmark.h"
#include <QPaintEvent>

class QWidget;
class QLabel;

class ViewerBookmark : public QWidget, protected Ui::Bookmark
{
    Q_OBJECT

public:
    ViewerBookmark(QWidget*);
    virtual ~ViewerBookmark();
    void mousePressEvent (QMouseEvent*);

protected:
    virtual void                        paintEvent                          ( QPaintEvent* );

public slots:
    void                                updateDisplay                       ( bool );
    void                                setLandscapeMode                    ( bool isLandscape = true);

signals:
    void changeBookmark();

};

#endif // ViewerBOOKMARK_H
