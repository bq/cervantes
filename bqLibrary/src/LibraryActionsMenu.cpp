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

#include "LibraryActionsMenu.h"

#include <QFile>
#include <QPainter>
#include <QDebug>
#include "QBook.h"

LibraryActionsMenu::LibraryActionsMenu( QWidget* parent ) :
    QWidget(parent)
{
    setupUi(this);

    connect(deleteActionBtn,  SIGNAL(clicked()), this, SLOT(deleteFile()));
    connect(copyActionBtn,    SIGNAL(clicked()), this, SLOT(copyFile()));
    arrowCont->hide();
    arrowCont2->hide();
}

LibraryActionsMenu::~LibraryActionsMenu()
{}

void LibraryActionsMenu::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void LibraryActionsMenu::setFile(QString path)
{
    qDebug() << Q_FUNC_INFO << path;
    m_path = path;
}

void LibraryActionsMenu::copyFile()
{
    emit copyFile(m_path);
}

void LibraryActionsMenu::deleteFile()
{
    emit deleteFile(m_path);
}
