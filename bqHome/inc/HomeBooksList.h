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

#ifndef HOMEBOOKSLIST_H
#define HOMEBOOKSLIST_H

#include "GestureWidget.h"
#include "ui_HomeBooksList.h"
#include "BookInfo.h"

// Predeclarations

class HomeBooksListItem;

class HomeBooksList : public GestureWidget, protected Ui::HomeBooksList{

    Q_OBJECT

public:
                                HomeBooksList                       (QWidget* parent);
    virtual                     ~HomeBooksList                      ();

    void                        setBooks                            (const QList<const BookInfo*>&);
    QList<const BookInfo*>      getBooks                            () {return books;}

private:

    QList<const BookInfo*>      books;
    int                         currentPage;
    QList<HomeBooksListItem*>   items;

private slots:

    void                        nextPage                            ();
    void                        previousPage                        ();
    void                        handleSwipe                         (int);
    void                        showPaginator                       ();

public slots:

    void                        showList                            ();
    void                        hide                                (int);
    void                        openContent                         (const BookInfo*);


protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    void                        paintEvent                          (QPaintEvent *);

signals:
    void                        openBook                            (const BookInfo*);
};

#endif // HOMEBOOKSLIST_H
