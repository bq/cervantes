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

#ifndef OPENBOOKS_H
#define OPENBOOKS_H

#include "PopUp.h"
#include "ui_CdlOpenBooks.h"
#include <QList>

class QBookApp;
class BookInfo;
class QPushButton;

class OpenBooks : public PopUp, protected Ui::CdlOpenBooks
{
    Q_OBJECT

public:
    OpenBooks(QBookApp* parent);
    ~OpenBooks();

private:
    QBookApp* m_parent;

public slots:
    void refreshOpenBooks(QList<const BookInfo*>);

signals:
    void openBook(const BookInfo*);

private:
    QList<QPushButton*> m_bookButtons;
    QList<const BookInfo*> m_bookList;
    void prepareBooks();

private slots:
    void selectedBook();
    void hideSlot();


};

#endif // OPENBOOKS_H
