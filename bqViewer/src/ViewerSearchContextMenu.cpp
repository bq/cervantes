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


#include "ViewerSearchContextMenu.h"

#include "Screen.h"

#include <QDebug>
#include <QWidget>


ViewerSearchContextMenu::ViewerSearchContextMenu(QWidget *parent) :
    PopUp(parent)
{
    qDebug() << "--->" << Q_FUNC_INFO;
    setupUi(this);
    connect(closeBtn,SIGNAL(clicked()),SIGNAL(close()));
    connect(listBtn,SIGNAL(pressed()),SIGNAL(backToList()));
    connect(previousBtn,SIGNAL(clicked()),SIGNAL(previousResult()));
    connect(nextBtn,SIGNAL(clicked()),SIGNAL(nextResult()));

}

ViewerSearchContextMenu::~ViewerSearchContextMenu()
{
    /// Do nothing
    qDebug() << "--->" << Q_FUNC_INFO;
}

void ViewerSearchContextMenu::setCurrentResultIndex(int index)
{
    resultIndexLbl->setText(QString::number(index));
}

void ViewerSearchContextMenu::setTotalResults(int total)
{
    qDebug() << Q_FUNC_INFO << "Total: " << total;
    numResultsLbl->setText(QString::number(total));
}

void ViewerSearchContextMenu::mousePressEvent(QMouseEvent *)
{
    qDebug() << Q_FUNC_INFO;
    emit close();
}
