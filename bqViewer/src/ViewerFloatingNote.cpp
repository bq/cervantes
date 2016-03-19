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

#include "ViewerFloatingNote.h"

#include <QPainter>
#include <QMouseEvent>

ViewerFloatingNote::ViewerFloatingNote(QWidget* parent) : QWidget(parent)
  , m_noteIndex(0)
{
    setupUi(this);
}

ViewerFloatingNote::~ViewerFloatingNote()
{}

void ViewerFloatingNote::setNumber( int number )
{
    m_noteIndex = number;
//    notesCount->setText(QString::number(number));
}

//void ViewerFloatingNote::mousePressEvent(QMouseEvent* event)
//{
//    event->accept();
//    emit noteClicked(m_noteIndex);
//}

void ViewerFloatingNote::paintEvent( QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
