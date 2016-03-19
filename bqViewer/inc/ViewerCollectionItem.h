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

#ifndef VIEWERCOLLECTIONITEM_H
#define VIEWERCOLLECTIONITEM_H

#include <QWidget>
#include "GestureWidget.h"
#include "ui_ViewerCollectionItem.h"

class QMouseEvent;

class ViewerCollectionItem : public GestureWidget, protected Ui::ViewerCollectionItem{

    Q_OBJECT

public:

                        ViewerCollectionItem                       (QWidget* parent = 0);
    virtual             ~ViewerCollectionItem                      ();
    const QString&      getText                                     () const { return m_itemText; }
    int                 getId                                       () { return m_id; }
    void                paintCollection                             (const QString &text, const int id);
    void setChecked(bool checked);

protected:
    /* http://qt-project.org/forums/viewthread/7340 */
    void                paintEvent                                  (QPaintEvent *);
    QString             m_itemText;
    int                 m_id;
    int                 m_checkWidth;

signals:
    void                tap                                         (int);

protected slots:
    virtual void        processTap                                  ();

};

#endif // VIEWERCOLLECTIONITEM_H
