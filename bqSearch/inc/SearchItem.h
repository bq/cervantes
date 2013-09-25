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

#ifndef BQSEARCHLIBRARYITEM_H
#define BQSEARCHLIBRARYITEM_H
#include "GestureWidget.h"
#include "ui_SearchItem.h"

class BookInfo;
class QFileInfo;

class SearchItem :  public GestureWidget, Ui::SearchItem{

    Q_OBJECT

public:
                    SearchItem                          (QWidget*);
    virtual         ~SearchItem                         ();

    void            paint                               ();
    void            paintFile                           ();
    void            paintBook                           ();
    void            setBook                             (const BookInfo*);
    void            setFile                             (const QFileInfo*);
    const BookInfo* getBook                             () const;

private:

    const BookInfo* book;
    const QFileInfo* file;

signals:

    void            openPath                            (const QString&);

public slots:
    void            handleTap                           ();

    /* http://qt-project.org/forums/viewthread/7340 */
    void            paintEvent                          (QPaintEvent *);

};
#endif // BQSEARCHLIBRARYITEM_H
