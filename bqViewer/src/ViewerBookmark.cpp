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

#include "ViewerBookmark.h"

#include <QDebug>
#include <QMouseEvent>
#include <QPainter>

ViewerBookmark::ViewerBookmark(QWidget* parent) :
    QWidget(parent)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);
}

ViewerBookmark::~ViewerBookmark()
{
    qDebug() << Q_FUNC_INFO;
}

void ViewerBookmark::mousePressEvent( QMouseEvent* event )
{
    qDebug() << Q_FUNC_INFO;

    event->accept();

    emit changeBookmark();
}

void ViewerBookmark::updateDisplay( bool toCheck )
{
    qDebug() << Q_FUNC_INFO << toCheck;

    if (toCheck) bookmarkImg->show();
    else         bookmarkImg->hide();
}

void ViewerBookmark::setLandscapeMode( bool isLandscape )
{
    if (isLandscape) bookmarkImg->setStyleSheet("background-image:url(:/res/bookmark_landscape.png) no-repeat center right");
    else             bookmarkImg->setStyleSheet("background:url(:/res/bookmark.png) no-repeat top center");
}

void ViewerBookmark::paintEvent( QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
