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

#include "ViewerNoteActionsPopup.h"

#include "Viewer.h"
#include "Screen.h"

#include <QPainter>
#include <QDebug>

ViewerNoteActionsPopup::ViewerNoteActionsPopup(QWidget* parent) : PopUp(parent)
{
    setupUi(this);

    connect(closeBtn,           SIGNAL(pressed()), SIGNAL(hideMe()));
    connect(definitionBtn,      SIGNAL(pressed()), SIGNAL(dictioActionClicked()));
    connect(searchBtn,          SIGNAL(pressed()), SIGNAL(searchActionClicked()));
    connect(editNoteBtn,        SIGNAL(pressed()), SIGNAL(editActionClicked()));
    connect(deleteNoteBtn,      SIGNAL(pressed()), SIGNAL(deleteActionClicked()));
    connect(noteTextBrowser,    SIGNAL(pressed()), SIGNAL(editActionClicked()));

    noteTextBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void ViewerNoteActionsPopup::setup( const QString& noteText, bool oneWord, const QPoint& hiliPos, const QRect& bbox, const int delimiterHeight)
{
    qDebug() << Q_FUNC_INFO << "HiliPos: " << hiliPos;
    qDebug() << Q_FUNC_INFO << "Parent: " << parentWidget()->size();

    noteTextBrowser->setText(noteText);

    if(oneWord)
        definitionBtn->show();
    else
        definitionBtn->hide();

    // NOTE: These lines allow us to ensure the popupCont has the proper width
    show();
    layout()->invalidate();
    hide();

    m_above = hiliPos.y() > (Screen::getInstance()->screenHeight() >> 1);
    m_bbox = bbox;
    m_delimiterHeight = delimiterHeight;
}

void ViewerNoteActionsPopup::showEvent( QShowEvent* )
{
    // Place the menu
    QPoint pos;

    int posX = ((parentWidget()->width() - popupCont->width()) >> 1);
    pos.setX(posX);

    // Above highlight
    if(m_above)
    {
        int posY = qMax(0, m_bbox.top() - height());
        pos.setY(posY);
    }
    else // Below highlight
    {
        int posY = qMin(parentWidget()->height() - height(), m_bbox.bottom() + m_delimiterHeight);
        pos.setY(posY);
    }

    topArrowLbl->hide();
    bottomArrowLbl->hide();

    move(pos);
}

void ViewerNoteActionsPopup::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
