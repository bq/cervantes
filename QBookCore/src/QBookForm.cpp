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

#include "QBookForm.h"
#include "QBook.h"

#include <QKeyEvent>
#include <QDebug>

QBookForm::QBookForm(QWidget* parent)
    : QWidget(parent)
    , m_flags(0)
{
    QPalette p(palette());
    // Set background colour to black
    p.setColor(QPalette::Background, Qt::white);
    setPalette(p);
}

QBookForm::~QBookForm()
{}

void QBookForm::setFormFlags(uint flags, bool on)
{
    if (on) m_flags |= flags;
    else m_flags &= ~flags;
}

/*virtual*/ void QBookForm::keyReleaseEvent(QKeyEvent *event)
{
    qDebug() << Q_FUNC_INFO << event;
    switch(event->key())
    {
    case QBook::QKEY_BACK:
        emit hideMe();
        event->accept();
        break;
    default:
        QWidget::keyReleaseEvent(event);
    }
}

