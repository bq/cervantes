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

#ifndef LIBRARYBOOKTOCOLLECTIONITEM_H
#define LIBRARYBOOKTOCOLLECTIONITEM_H

#include "GestureWidget.h"
#include "ui_LibraryBookToCollectionItem.h"

class BookInfo;

class LibraryBookToCollectionItem : public GestureWidget, protected Ui::LibraryBookToCollectionItem
{
    Q_OBJECT

public:
                                    LibraryBookToCollectionItem             (QWidget * parent = 0);
    virtual                          ~LibraryBookToCollectionItem           ();

    void                            setBook                                 ( const BookInfo *book );
    void                            setChecked                              (bool checked);
    bool                            isChecked                               ( ) { return m_checked; }
    void                            hideItem                                ();
    const BookInfo*                 getBook                                 () { return m_book; }


protected slots:

    /* http://qt-project.org/forums/viewthread/7340 */
    void                            paintEvent                              (QPaintEvent *);
    void                            handleClicked                           ();

private:

    QString                         m_collectionName;
    bool                            m_checked;
    const BookInfo*                 m_book;

signals:
    void                            changeStatus                            ();

};

#endif // LIBRARYBOOKTOCOLLECTIONITEM_H
