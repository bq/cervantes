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

#include "ViewerSearchResultItem.h"

#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

ViewerSearchResultItem::ViewerSearchResultItem(QWidget* parent) : GestureWidget(parent)
  , m_location(NULL)
{
    connect(this, SIGNAL(tap()), this, SLOT(handleTap()));
    setupUi(this);
}

ViewerSearchResultItem::~ViewerSearchResultItem()
{
    m_location = NULL;
}

void ViewerSearchResultItem::setLocation( QDocView::Location* location )
{
    m_location = location;
    if(m_location)
    {
        searchResultPageLbl->setText(tr("Pág. %1").arg(m_location->page));
        searchResultTextLbl->setText(m_location->preview);
    }
    else
    {
        searchResultPageLbl->setText("");
        searchResultTextLbl->setText("");
    }
}

void ViewerSearchResultItem::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ViewerSearchResultItem::handleTap()
{
    qDebug() << Q_FUNC_INFO;
    if(m_location)
        emit goToUrl(m_location->ref);
}
