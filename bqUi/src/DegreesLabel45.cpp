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

#include "DegreesLabel45.h"

#include <QDebug>
#include <QLabel>
#include <QFrame>
#include <QPainter>
#include <QFontMetrics>
#include <QTextLayout>
#include <QStyleOption>

#define ROTATE45 45

DegreesLabel45::DegreesLabel45(QWidget *parent)
     : QLabel(parent)
 {
     setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
 }

DegreesLabel45::~DegreesLabel45()
{
    /// Do nothing
    qDebug() << "--->" << Q_FUNC_INFO;
}

void DegreesLabel45::setText(const QString &newText)
 {
     content = newText;
     update();
 }

void DegreesLabel45::paintEvent( QPaintEvent* event )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QTextLayout textLayout(content, p.font());
    textLayout.beginLayout();
    drawRotatedText(&p, -ROTATE45, 0, height()/2, content);
}

void DegreesLabel45::drawRotatedText(QPainter *painter, float degrees, int x, int y, const QString &text)
 {
   painter->save();
   painter->translate(x, y);
   painter->rotate(degrees);
   painter->drawText(-2, 0, width(), height()/2-18, Qt::AlignLeft | Qt::AlignVCenter, text);
   painter->restore();
 }
