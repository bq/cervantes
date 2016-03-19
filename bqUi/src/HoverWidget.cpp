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

#include "HoverWidget.h"

#include <QDebug>
#include <QEvent>
#include <QHoverEvent>
#include <QApplication>
#include <QPainter>
#include <QStyleOption>

HoverWidget::HoverWidget(QWidget *parent) :
    QWidget(parent)
  , m_on(false)
{
    qDebug() << "--->" << Q_FUNC_INFO;
    installEventFilter(this);
//    setAttribute(Qt::WA_Hover);
}

HoverWidget::~HoverWidget()
{
    qDebug() << "--->" << Q_FUNC_INFO;

}

/*virtual*/ bool HoverWidget::eventFilter(QObject* obj, QEvent* event)
{
    if(this == obj)
        qDebug() << Q_FUNC_INFO << obj << event->type();

    if(event->type() == QEvent::DragEnter)
    {
        m_on = !m_on;
        if(m_on)
        {
            setStyleSheet("background-color:#000000");
        }
        else
        {
            setStyleSheet("background-color:#FFFFFF");
        }
        return true;
    }
//    if(event->type() == QEvent::HoverMove) {
//        event->accept();
//        QHoverEvent *hoverEvent = static_cast<QHoverEvent*>(event);
//        qDebug() << "--->" << Q_FUNC_INFO << hoverEvent->pos();
//        emit over(hoverEvent->pos().x());
//        return false;
//    }
    return false;
}

void HoverWidget::mousePressEvent(QMouseEvent* event)
{
    qDebug() << Q_FUNC_INFO << this;

    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData();
    drag->setMimeData(mimeData);
    Qt::DropAction dropAction = drag->exec();
    event->accept();

    // TODO: Colorear de inicio todos los previos!!
}

//void HoverWidget::dragEnterEvent(QDragEnterEvent* event)
//{
//    qDebug() << Q_FUNC_INFO << this;

//    event->acceptProposedAction();
//}

void HoverWidget::paintEvent( QPaintEvent* event )
{
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

