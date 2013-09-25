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

#include "DegreesLabel90.h"

#include <QDebug>
#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QPainter>
#include <QFontMetrics>
#include <QTextLayout>
#include <QStyleOption>

#define ROTATE90 270

DegreesLabel90::DegreesLabel90(QWidget *parent)
     : QLabel(parent)
 {
     setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
 }

DegreesLabel90::~DegreesLabel90()
{
    /// Do nothing
    qDebug() << "--->" << Q_FUNC_INFO;
}

void DegreesLabel90::setText(const QString &newText)
 {
     content = newText;
     update();
 }

void DegreesLabel90::paintEvent( QPaintEvent* event )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QTextLayout textLayout(content, p.font());
    textLayout.beginLayout();
    drawRotatedText(&p, ROTATE90, width(), height()/2, content);
}

void DegreesLabel90::drawRotatedText(QPainter *painter, float degrees, int x, int y, const QString &text)
 {
   painter->save();
   painter->translate(x, y);
   painter->rotate(degrees);
   painter->drawText(-x, -y, height(), width(), Qt::AlignCenter, text);
   painter->restore();
 }
