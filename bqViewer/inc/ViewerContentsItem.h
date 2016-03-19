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

#ifndef VIEWERCONTENTSITEM_H
#define VIEWERCONTENTSITEM_H

#include "ui_ViewerContentsItem.h"

#include "QDocView.h"
#include <QString>
#include "GestureWidget.h"

class ViewerContentsItem : public GestureWidget,
                           public Ui::ViewerContentsItem
{
    Q_OBJECT

public:
    ViewerContentsItem(QWidget * parent = 0);
    virtual ~ViewerContentsItem();

    void                                                            paint                               ();
    bool                                                            setItem                             (QDocView::Location*, int bookmarks, int notes, int highligths);
    bool                                                            currentChapter;

public slots:
    void                                                            handleTap                           ();

signals:
    void                                                            chapterPressed                      ( const QString& );

//protected:
//    void                                                            showEvent                           (QShowEvent* event);

private:
    QDocView::Location*                                             m_location;// Weak ptr. Do no delete it.
    int                                                             m_iNotes;
    int                                                             m_iHighlights;
    int                                                             m_iBookmarks;

    void                                                            paintEvent                          ( QPaintEvent* );


};
#endif // VIEWERCONTENTSITEM_H
