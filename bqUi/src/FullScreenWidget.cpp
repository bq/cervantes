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

#include "QBook.h"
#include "FullScreenWidget.h"

#include <QDebug>
#include <QMouseEvent>
#include <QApplication>

FullScreenWidget::FullScreenWidget(QWidget *parent) :
    QWidget(parent)
  , m_navigationPrevious(NULL)
{
    qDebug() << "--->" << Q_FUNC_INFO;
}

FullScreenWidget::~FullScreenWidget()
{
    qDebug() << "--->" << Q_FUNC_INFO;

    m_navigationPrevious = NULL;
}

/*virtual*/ void FullScreenWidget::keyReleaseEvent(QKeyEvent *event)
{
    qDebug() << Q_FUNC_INFO << event << QApplication::focusWidget();
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
