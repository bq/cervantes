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

#include "ViewerTextActionsMenu.h"

#include "QBookApp.h"
#include "StatusBar.h"
#include "Viewer.h"
#include "Screen.h"
#include <QDebug>
#include <QPainter>

ViewerTextActionsMenu::ViewerTextActionsMenu(QWidget *parent) : QWidget(parent)
{
    qDebug() << Q_FUNC_INFO;

    setupUi(this);

    connect(notesActionBtn,         SIGNAL(pressed()), SIGNAL(notesActionClicked()));
    connect(underlineActionBtn,     SIGNAL(pressed()), SIGNAL(highlightActionClicked()));
    connect(searchActionBtn,        SIGNAL(pressed()), SIGNAL(searchActionClicked()));
    connect(deleteActionBtn,        SIGNAL(pressed()), SIGNAL(deleteActionClicked()));
    connect(defineActionBtn,        SIGNAL(pressed()), SIGNAL(dictioActionClicked()));
    connect(wikipediaMenuBtn,       SIGNAL(clicked()), this, SIGNAL(wikipediaClicked()));
}

void ViewerTextActionsMenu::setup( bool newMark, bool oneWord, const QPoint& hiliPos, const QRect& bbox, const int delimiterHeight )
{
    qDebug() << Q_FUNC_INFO << "HiliPos: " << hiliPos;
    qDebug() << Q_FUNC_INFO << "Parent: " << parentWidget()->size();
    qDebug() << Q_FUNC_INFO << "Widget height: " << height();

    if(newMark)
    {
        notesActionBtn->setText(tr("Crear nota"));
        underlineActionBtn->show();
        deleteActionBtn->hide();
    }
    else
    {
        notesActionBtn->setText(tr("Añadir nota"));
        underlineActionBtn->hide();
        deleteActionBtn->show();
    }

    if(oneWord)
        defineActionBtn->show();
    else
        defineActionBtn->hide();

    // NOTE: These lines allow us to ensure the popupCont has the proper width
    show();
    layout()->invalidate();
    hide();

    // Calculate placement of widget.
    int screenHeight = Screen::getInstance()->screenHeight();
    int yO = bbox.top();
    int yF = bbox.bottom();
    int topSpace = yO;
    int bottomSpace = screenHeight - (yF + delimiterHeight);
    bool topBroader = topSpace > bottomSpace && topSpace > height();
    bool bottomBroader = bottomSpace > height();
    m_above = topBroader;
    m_below = bottomBroader;

    m_bbox = bbox;
    m_delimiterHeight = delimiterHeight;
}

void ViewerTextActionsMenu::showEvent( QShowEvent* )
{
    qDebug() << Q_FUNC_INFO << width();

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
    // Below highlight
    else if(m_below)
    {
        int posY = qMin(parentWidget()->height() - height(), m_bbox.bottom() + m_delimiterHeight);
        pos.setY(posY);
    }
    else //pos the widget inside the hili in the top position.
    {
        int posY = m_bbox.top() + m_delimiterHeight;
        pos.setY(posY);
    }

    topArrowLbl->hide();
    bottomArrowLbl->hide();

    move(pos);
}

void ViewerTextActionsMenu::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
