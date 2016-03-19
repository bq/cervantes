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

#include "LibraryLineGridViewer.h"

#include "Library.h"

#include <QFile>
#include <QPainter>
#include <QDebug>

const int s_itemsPerPage = 6;

LibraryLineGridViewer::LibraryLineGridViewer(QWidget* parent) :
    LibraryGridViewer(parent)
{
    setupUi(this);
}

LibraryLineGridViewer::~LibraryLineGridViewer()
{}

void LibraryLineGridViewer::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

LibraryPageHandler* LibraryLineGridViewer::getPageHandler()
{
    return VerticalPagerHandler;
}

int LibraryLineGridViewer::getItemsPerPage()
{
    return s_itemsPerPage;
}

void LibraryLineGridViewer::getBookItems( LibraryGridViewerItem* books[] )
{
    books[0] = book1;
    books[1] = book2;
    books[2] = book3;
    books[3] = book4;
    books[4] = book5;
    books[5] = book6;
}
