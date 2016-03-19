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


#include "ViewerSearchContextMenuLandscape.h"

#include "Screen.h"

#include <QDebug>
#include <QWidget>


ViewerSearchContextMenuLandscape::ViewerSearchContextMenuLandscape(QWidget *parent) :
    PopUp(parent)
  , ui_indexResult(0)
  , ui_totalResults(0)
{
    qDebug() << "--->" << Q_FUNC_INFO;
    setupUi(this);

    connect(closeBtn,    SIGNAL(clicked()),SIGNAL(close()));
    connect(listBtn,     SIGNAL(pressed()),SIGNAL(backToList()));
    connect(previousBtn, SIGNAL(clicked()),SIGNAL(previousResult()));
    connect(nextBtn,     SIGNAL(clicked()),SIGNAL(nextResult()));
}

ViewerSearchContextMenuLandscape::~ViewerSearchContextMenuLandscape()
{
    /// Do nothing
    qDebug() << "--->" << Q_FUNC_INFO;
}

void ViewerSearchContextMenuLandscape::setCurrentResultIndex(uint index)
{
    ui_indexResult = index;
    resultCountLbl->setText(tr("Result %1 of %2").arg(ui_indexResult).arg(ui_totalResults));
}

void ViewerSearchContextMenuLandscape::setTotalResults(uint total)
{
    qDebug() << Q_FUNC_INFO << "Total: " << total;
    ui_totalResults = total;
    resultCountLbl->setText(tr("Result %1 of %2").arg(ui_indexResult).arg(ui_totalResults));
}

void ViewerSearchContextMenuLandscape::mousePressEvent(QMouseEvent *)
{
    qDebug() << Q_FUNC_INFO;
    emit close();
}
