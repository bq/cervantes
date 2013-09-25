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

#include "ViewerDelimiter.h"

#include "QBookApp.h"
#include "Viewer.h"
#include "Screen.h"
#include <QDebug>
#include <QPainter>

int ViewerDelimiter::m_viewUpperMargin = 0;

ViewerDelimiter::ViewerDelimiter(QWidget *parent) :
    QWidget(parent)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);
    m_topLeftOffsetToText = QPoint(width()/2, 0);
}

void ViewerDelimiter::showIn(const QRect& bbox, int const highlightHeight, bool const isOnTheLeft)
{
    qDebug() << Q_FUNC_INFO;

    m_bbox = bbox;
    m_highlightHeight = highlightHeight;
    m_isOnTheLeft = isOnTheLeft;
    m_FineAdjustFactor = m_isOnTheLeft? 1: 0.5;

    // Memorize the highlight point.
    if(m_isOnTheLeft)
        m_highlightPoint = m_bbox.topLeft() + QPoint(0, m_highlightHeight/2);
    else
        m_highlightPoint = m_bbox.bottomRight() - QPoint(0, m_highlightHeight/2);

    // Place the widget
    place();

    if(!isVisible()) show();
}

void ViewerDelimiter::place()
{
    QPoint point;

    if(m_isOnTheLeft)
        point = QPoint(m_bbox.left() - width()/2, m_bbox.top() + m_viewUpperMargin);
    else
        point = QPoint(m_bbox.right() - width()/2, m_bbox.bottom() - m_highlightHeight + m_viewUpperMargin);

    resize(width(), m_highlightHeight + teardrop->height());
    move(point);
}

void ViewerDelimiter::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    event->accept();
}

void ViewerDelimiter::mousePressEvent ( QMouseEvent* event )
{
    m_grabPoint = event->pos();
    emit pressEvent(m_highlightPoint);

    event->accept();
}

void ViewerDelimiter::mouseReleaseEvent ( QMouseEvent* event)
{
    m_highlightPoint = mapToParent(event->pos() - m_grabPoint + m_FineAdjustFactor*m_topLeftOffsetToText + QPoint(0, m_highlightHeight/2));
    emit releaseEvent(m_highlightPoint - QPoint(0, m_viewUpperMargin));

    event->accept();
}

void ViewerDelimiter::mouseMoveEvent( QMouseEvent* event )
{
    m_highlightPoint = mapToParent(event->pos() - m_grabPoint + m_FineAdjustFactor*m_topLeftOffsetToText + QPoint(0, m_highlightHeight/2));
    emit moveEvent(m_highlightPoint - QPoint(0, m_viewUpperMargin));

    event->accept();
}

QPoint ViewerDelimiter::getHighlightPoint()
{
    return m_highlightPoint;
}

void ViewerDelimiter::viewUpperMarginUpdate(int newUpperMargin)
{
    m_viewUpperMargin = newUpperMargin;
}
