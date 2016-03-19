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

#ifndef VIEWERSEARCHCONTEXTMENULANDSCAPE_H
#define VIEWERSEARCHCONTEXTMENULANDSCAPE_H

#include "ui_ViewerSearchContextMenuLandscape.h"
#include "PopUp.h"


class ViewerSearchContextMenuLandscape :  public PopUp
                                         ,protected Ui::ViewerSearchContextMenuLandscape
{
    Q_OBJECT

public:
    ViewerSearchContextMenuLandscape(QWidget*);
    virtual ~ViewerSearchContextMenuLandscape();

public slots:
    void setCurrentResultIndex(uint);
    void setTotalResults(uint);

signals:
    void close();
    void previousResult();
    void nextResult();
    void backToList();

protected:
    void mousePressEvent(QMouseEvent *);

    uint ui_totalResults, ui_indexResult;

};

#endif // VIEWERSEARCHCONTEXTMENULANDSCAPE_H
