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

#ifndef VIEWERANNOTATIONITEM_H
#define VIEWERANNOTATIONITEM_H

#include "ui_ViewerAnnotationItem.h"
#include "GestureWidget.h"

class BookLocation;
class ViewerAnnotationActions;
class QPaintEvent;

class ViewerAnnotationItem : public GestureWidget,
                              protected Ui::ViewerAnnotationItem
{
    Q_OBJECT

public:
    ViewerAnnotationItem(QWidget*);
    virtual ~ViewerAnnotationItem();

    void                                    setItem                                             (const BookLocation*);
    BookLocation*                           getLocation                                         () { return m_location; }

public slots:
    void                                    handleTap                                           ();
    void                                    handleLongPress                                     ();

signals:
    void                                    handleTap                                           (const QString&);
    void                                    longPress                                           (BookLocation*);

protected:
    virtual void                            paintEvent                                          (QPaintEvent*);

private:
    QString                                 s_markupRef;
    ViewerAnnotationActions*                m_actions;
    BookLocation*                           m_location;

};

#endif // VIEWERANNOTATIONITEM_H
