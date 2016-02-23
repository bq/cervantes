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

#ifndef VIEWERCOLLECTIONLAYER_H
#define VIEWERCOLLECTIONLAYER_H

#include <QWidget>
#include "GestureWidget.h"
#include "ui_ViewerCollectionLayer.h"

class ViewerCollectionItem;

class ViewerCollectionLayer : public GestureWidget, protected Ui::ViewerCollectionLayer
{
    Q_OBJECT
public:
                                        ViewerCollectionLayer              ( QWidget* parent );
    virtual                             ~ViewerCollectionLayer             ();
    void                                setup                              ( QHash<QString, double> collectionList );
    void                                paint                              ();


protected slots:
    void                                showNextPage                        ();
    void                                showPreviousPage                    ();
    void                                changeCollection                    (int idItem);

signals:
    void                                addCollection                       (QString collectionName, double index);
    void                                removeCollection                    (QString collectionName);
    void                                createCollection                    ();

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                        paintEvent                          ( QPaintEvent* );
    QHash<QString, double>              m_bookCollections;
    QStringList                         m_modelCollections;
    QList<ViewerCollectionItem*>        items;
    int                                 m_page;

};

#endif // VIEWERCOLLECTIONLAYER_H
