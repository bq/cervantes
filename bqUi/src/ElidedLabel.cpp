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

#include "ElidedLabel.h"

#include <QDebug>
#include <QLabel>
#include <QFrame>
#include <QPainter>
#include <QFontMetrics>
#include <QTextLayout>
#include <QStyleOption>

ElidedLabel::ElidedLabel(QWidget *parent)
     : QLabel(parent)
     , elided(false)
 {
     setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
 }


ElidedLabel::~ElidedLabel()
{
    /// Do nothing
    qDebug() << "--->" << Q_FUNC_INFO;
}

void ElidedLabel::setText(const QString &newText)
 {
     content = newText;
     update();
 }

void ElidedLabel::paintEvent( QPaintEvent* event )
{
    QFrame::paintEvent(event);

    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QFontMetrics fontMetrics = painter.fontMetrics();

    bool didElide = false;
    int lineSpacing = fontMetrics.ascent();
    int y = 0;

    QTextLayout textLayout(content, painter.font());
    textLayout.beginLayout();
    forever {
        QTextLine line = textLayout.createLine();

        if (!line.isValid())
            break;

        line.setLineWidth(width());
        if (content.mid(line.textLength()) == NULL) {
            QString text = fontMetrics.elidedText(content, Qt::ElideRight, width());
            painter.drawText(0, 0, width(), height(), Qt::AlignVCenter, text);
            break;
        }
        int nextLineY = y + lineSpacing;

        if (height() >= nextLineY + lineSpacing) {
            line.draw(&painter, QPoint(0, y));
            y = nextLineY;
        } else {
            QString lastLine = content.mid(line.textStart());
            QString elidedLastLine = fontMetrics.elidedText(lastLine, Qt::ElideRight, width());
            painter.drawText(QPoint(0, y + fontMetrics.ascent()), elidedLastLine);
            line = textLayout.createLine();
            didElide = line.isValid();
            break;
        }
    }
    textLayout.endLayout();
}
