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

#include "ViewerBookmarkLandscape.h"

#include <QDebug>
#include <QMouseEvent>
#include <QPainter>

ViewerBookmarkLandscape::ViewerBookmarkLandscape(QWidget* parent) :
    QWidget(parent)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);
}

ViewerBookmarkLandscape::~ViewerBookmarkLandscape()
{
    qDebug() << Q_FUNC_INFO;
}

void ViewerBookmarkLandscape::mousePressEvent( QMouseEvent* event )
{
    qDebug() << Q_FUNC_INFO;

    event->accept();

    emit changeBookmark();
}

void ViewerBookmarkLandscape::updateDisplay( bool toCheck )
{
    qDebug() << Q_FUNC_INFO << toCheck;

    if(toCheck)
    {
            bookmarkImg->show();
    }
    else
    {
            bookmarkImg->hide();
    }
}

void ViewerBookmarkLandscape::paintEvent( QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
