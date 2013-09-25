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

#include "LibraryGridViewerItem.h"
#include "QBookApp.h"
#include "Model.h"

#include <QDebug>

LibraryGridViewerItem::LibraryGridViewerItem(QWidget* parent) :
    GestureWidget(parent)
{
    connect(this,   SIGNAL(tap()),              this, SLOT(processTap()));
    connect(this,   SIGNAL(longPressStart()),   this, SLOT(processLongpress()));
}

LibraryGridViewerItem::~LibraryGridViewerItem()
{}

void LibraryGridViewerItem::processTap()
{
    qDebug() << Q_FUNC_INFO;

    emit itemClicked(m_path);
}

void LibraryGridViewerItem::processLongpress ()
{
    qDebug() << Q_FUNC_INFO;
    const BookInfo* bookInfo = QBookApp::instance()->getModel()->getBookInfo(m_path);
    if(bookInfo)
        emit itemLongPress(m_path);
    else
        emit fileLongPressed(m_path);
}
