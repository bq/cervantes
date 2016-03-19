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

#include "ViewerTextActionsPopup.h"

#include "Viewer.h"
#include "bqUtils.h"
#include "QBookApp.h"
#include "MouseFilter.h"
#include "Screen.h"

#include <QDebug>
#include <QScrollBar>

#define MARGIN 10

ViewerTextActionsPopup::ViewerTextActionsPopup(QWidget *parent) : QWidget(parent)
{
    qDebug() << Q_FUNC_INFO;

    setupUi(this);

    connect(createNoteBtn,          SIGNAL(clicked()), SIGNAL(createNoteClicked()));
    connect(highlightedBtn,         SIGNAL(clicked()), SIGNAL(highlightedClicked()));
    connect(searchBtn,              SIGNAL(clicked()), SIGNAL(searchClicked()));
    connect(QBookApp::instance(),   SIGNAL(swipe(int)), this, SLOT(handleSwipe(int)));
    connect(definitionTextBrowser,  SIGNAL(wordClicked(const QString&, const QString&)), SIGNAL(wordToSearch(const QString&, const QString&)));
    connect(seeCompleteDefinition,  SIGNAL(clicked()), SIGNAL(completeDefinitionClicked()));
    connect(wikipediaPopupBtn,      SIGNAL(clicked()), this, SIGNAL(wikipediaClicked()));

    definitionTextBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vbar = definitionTextBrowser->verticalScrollBar();
    definitionTextBrowser->setTappable(true);
    if(QBook::getResolution() == QBook::RES758x1024)
        vbar->setSingleStep(190);
    else
        vbar->setSingleStep(160);
}

ViewerTextActionsPopup::~ViewerTextActionsPopup()
{
    // Do nothing
    qDebug() << Q_FUNC_INFO;
}

void ViewerTextActionsPopup::setup( const QString& dictionaryDefinition, const QPoint& hiliPos, const QRect& bbox, const int delimiterHeight)
{
    qDebug() << Q_FUNC_INFO << "HiliPos: " << hiliPos << ", bbox: " << bbox;

    definitionTextBrowser->applyDocument(dictionaryDefinition);
    // Note button
    createNoteBtn->setText(tr("Create note"));
    highlightedBtn->setText(tr("Highlight"));
    highlightedBtn->show();

    // NOTE: These lines allow us to ensure the popupCont has the proper width
    show();
    qDebug() << Q_FUNC_INFO << "Widget height" << height();
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

void ViewerTextActionsPopup::handleSwipe(int direction)
{
    qDebug()<< Q_FUNC_INFO << "Direction" << direction;

    switch(direction)
    {
    case MouseFilter::SWIPE_R2L:
    case MouseFilter::SWIPE_D2U:
        nextPage();
        break;

    case MouseFilter::SWIPE_L2R:
    case MouseFilter::SWIPE_U2D:
        previousPage();
        break;

    default:
        //ignore
        break;
    }
}

void ViewerTextActionsPopup::previousPage ()
{
    qDebug() << Q_FUNC_INFO;

    if (vbar)
    {
        vbar->triggerAction(QAbstractSlider::SliderSingleStepSub);
    }
}

void ViewerTextActionsPopup::nextPage ()
{
    qDebug() << Q_FUNC_INFO;

    if (vbar)
    {
        vbar->triggerAction(QAbstractSlider::SliderSingleStepAdd);
    }
}

void ViewerTextActionsPopup::showEvent( QShowEvent* )
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
    // Below highlight
    else if(m_below)
    {
        int posY = qMin(parentWidget()->height() - height(), m_bbox.bottom() + m_delimiterHeight);
        pos.setY(posY);
    }
    else //pos the widget inside the hili in the top position.
    {
        int posY;
        int bottomMargin = Screen::getInstance()->screenHeight() - MARGIN;
        if(m_bbox.top() + m_delimiterHeight + height() < bottomMargin)
            posY = m_bbox.top() + m_delimiterHeight;
        else
            posY = m_bbox.bottom() - height();
        pos.setY(posY);
    }

    topArrowLbl->hide();
    bottomArrowLbl->hide();

    move(pos);
}
