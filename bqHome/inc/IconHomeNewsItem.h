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

#ifndef ICONHOMENEWSITEM_H
#define ICONHOMENEWSITEM_H

#include "ui_IconHomeNewsItem.h"
#include <QWidget>

class IconHomeNewsItem : public QWidget,
                    protected Ui::IconNewsHomeItem
{
    Q_OBJECT

public:
    IconHomeNewsItem(QWidget * parent = 0);
    virtual ~IconHomeNewsItem();

    const Product* _product;

    void paint();
    void setProduct(const Product* p);

signals:
    void openStore(QString);

protected:
    QString _idProducto;

    virtual void mousePressEvent(QMouseEvent *);

};
#endif // ICONHOMENEWSITEM_H
