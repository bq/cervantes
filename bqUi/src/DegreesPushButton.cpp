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

#include "DegreesPushButton.h"

#include <QDebug>
#include <QWidget>
#include <QPainter>
#include <QTextLayout>
#include <QStyleOption>

#define ROTATE270 90

DegreesPushButton::DegreesPushButton(QWidget *parent) :QPushButton(parent)
{
    alignment = Qt::AlignCenter;
}

DegreesPushButton::~DegreesPushButton()
{
    /// Do nothing
    qDebug() << "--->" << Q_FUNC_INFO;
}

void DegreesPushButton::paintEvent( QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QTextLayout textLayout(text(), p.font());
    textLayout.beginLayout();
    drawRotatedText(&p, ROTATE270, width(), height()/2, text());
}

void DegreesPushButton::drawRotatedText(QPainter *painter, float degrees, int x, int y, const QString &text)
 {
    painter->save();
    painter->rotate(degrees);
    painter->translate(0, -width());
    painter->drawText(0, 0, height(), width(), alignment, text);
    painter->restore();
 }
