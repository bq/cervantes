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

#include "ViewerVerticalPagerPopup.h"

#include "QBook.h"
#include "MouseFilter.h"
#include "QBookApp.h"

#include <QPainter>
#include <QDebug>
#include <QWidget>

ViewerVerticalPagerPopup::ViewerVerticalPagerPopup(QWidget* parent) :
                          QWidget(parent)
{
    qDebug() <<Q_FUNC_INFO;
    setupUi(this);

    connect(downPageBtn,    SIGNAL(pressed()), this, SLOT(nextPage()));
    connect(upPageBtn,      SIGNAL(pressed()), this, SLOT(previousPage()));
    connect(QBookApp::instance(),SIGNAL(swipe(int)),this,SLOT(handleSwipe(int)));
}

ViewerVerticalPagerPopup::~ViewerVerticalPagerPopup()
{
    qDebug() << Q_FUNC_INFO;
}

void ViewerVerticalPagerPopup::setup(int totalPages, int currentPage, bool loopable, bool scrollBar)
{
    qDebug() << Q_FUNC_INFO;

    m_currentPage = currentPage;
    m_totalPages = (totalPages != 0) ? totalPages : 1;
    m_loopable = loopable;
    b_scrollBar = scrollBar;

    QString str = QString(tr("%1 of %2")).arg(m_currentPage).arg(m_totalPages);
    pagTextLbl->setText(str);

    if(totalPages > 1)
    {
        downPageBtn->show();
        upPageBtn->show();
    }
    else
    {
        downPageBtn->hide();
        upPageBtn->hide();
    }
}

void ViewerVerticalPagerPopup::nextPage()
{
    if(b_scrollBar)
      emit nextPageReq();

    if(m_currentPage < m_totalPages)
    {
        m_strPagesText = QString(tr("%1 of %2")).arg(++m_currentPage).arg(m_totalPages);
        emit nextPageReq();
    }
    else if(m_loopable)
    {
        m_currentPage = 1;
        m_strPagesText = QString(tr("1 of %1")).arg(m_totalPages);
        emit firstPage();
    }
}

void ViewerVerticalPagerPopup::previousPage()
{
    if(b_scrollBar)
      emit previousPageReq();

    if(m_currentPage > 1)
    {
        m_strPagesText = QString(tr("%1 of %2")).arg(--m_currentPage).arg(m_totalPages);
        emit previousPageReq();
    }
    else if(m_loopable)
    {
        m_currentPage = m_totalPages;
        m_strPagesText = QString(tr("%1 of %2")).arg(m_currentPage).arg(m_totalPages);
        emit lastPage();
    }
}

void ViewerVerticalPagerPopup::setPage()
{
    pagTextLbl->setText(m_strPagesText);
}

void ViewerVerticalPagerPopup::hideLabel()
{
    pagTextLbl->hide();
}

void ViewerVerticalPagerPopup::hideLabels()
{
    pagTextLbl->hide();
    upPageBtn->hide();
    downPageBtn->hide();
}

void ViewerVerticalPagerPopup::handleSwipe(int direction)
{
    qDebug()<< Q_FUNC_INFO << "Direction" << direction;

    if(isVisible())
    {
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
}

void ViewerVerticalPagerPopup::paintEvent (QPaintEvent *)
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
