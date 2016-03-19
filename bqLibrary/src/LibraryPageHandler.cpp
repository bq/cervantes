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

#include "LibraryPageHandler.h"

#include "QBook.h"
#include "MouseFilter.h"
#include "QBookApp.h"

#include <QFile>
#include <QPainter>
#include <QDebug>

#define INT_ROUND_VALUE 0.5
#define TIME_FOR_LONG_PRESS 700

LibraryPageHandler::LibraryPageHandler(QWidget* parent) :
    QWidget(parent), m_strPagesText(""),m_firstPageLaunched(false),m_lastPageLaunched(false),b_sliderMoved(false)
{
    setupUi(this);
    connect(forwardButton,    SIGNAL(pressed()),                 this,          SLOT(nextPagePressed()));
    connect(backButton,       SIGNAL(pressed()),                 this,          SLOT(previousPagePressed()));
    connect(forwardButton,    SIGNAL(released()),                this,          SLOT(nextPageReleased()));
    connect(backButton,       SIGNAL(released()),                this,          SLOT(previousPageReleased()));
    connect(pagerSlider,      SIGNAL(sliderMoved(int)),          this,          SLOT(sliderMoved(int)));
    connect(pagerSlider,      SIGNAL(jumpSliderPage(int)),       this,          SLOT(jumpToPage(int)));
    connect(QBookApp::instance(),SIGNAL(swipe(int)),             this,          SLOT(handleSwipe(int)));
}

LibraryPageHandler::~LibraryPageHandler()
{
}

void LibraryPageHandler::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void LibraryPageHandler::nextPage()
{
    nextPageTimer.stop();

    //We emit the signal because the limit is get by the scroll bar maximum value.
    if(b_scrollBar)
        emit nextPageReq();

    else if(m_currentPage + 1 < m_totalPages)
    {
        m_currentPage++;
        m_strPagesText = QString(tr("%1 of %2")).arg(m_currentPage).arg(m_totalPages);
        emit nextPageReq();
    }
    else if(m_loopable)
    {
        m_currentPage = 0;
        m_strPagesText = QString(tr("1 of %1")).arg(m_totalPages);
        emit firstPage();
    }
}

void LibraryPageHandler::previousPage()
{
    previousPageTimer.stop();

    //We emit the signal because the limit is get by the scroll bar maximum value.
    if(b_scrollBar)
        emit previousPageReq();

    if(m_currentPage > 0)
    {
        m_strPagesText = QString(tr("%1 of %2")).arg(--m_currentPage).arg(m_totalPages);
        emit previousPageReq();
    }
    else if(m_loopable)
    {
        m_currentPage = m_totalPages - 1;
        m_strPagesText = QString(tr("%1 of %2")).arg(m_currentPage).arg(m_totalPages);
        emit lastPage();
    }
}

void LibraryPageHandler::nextPagePressed()
{
    qDebug() << Q_FUNC_INFO;
    connect(&nextPageTimer, SIGNAL(timeout()),  this, SLOT(goToLastPage()), Qt::UniqueConnection);
    nextPageTimer.setSingleShot(true);
    nextPageTimer.start(TIME_FOR_LONG_PRESS);
}

void LibraryPageHandler::previousPagePressed()
{
    qDebug() << Q_FUNC_INFO;
    connect(&previousPageTimer, SIGNAL(timeout()), this, SLOT(goToFirstPage()), Qt::UniqueConnection);
    previousPageTimer.setSingleShot(true);
    previousPageTimer.start(TIME_FOR_LONG_PRESS);
}

void LibraryPageHandler::goToLastPage()
{
    qDebug() << Q_FUNC_INFO;
    nextPageTimer.stop();
    m_lastPageLaunched = true;
    m_currentPage = m_totalPages - 1;
    emit lastPage();
}
void LibraryPageHandler::goToFirstPage()
{
    qDebug() << Q_FUNC_INFO;
    previousPageTimer.stop();
    m_firstPageLaunched = true;
    m_currentPage = 0;
    emit firstPage();
}

void LibraryPageHandler::nextPageReleased()
{
    if(nextPageTimer.isActive() && !m_firstPageLaunched)
    {
        nextPageTimer.stop();
        nextPage();
    }
    m_lastPageLaunched = false;
}

void LibraryPageHandler::previousPageReleased()
{
    if(previousPageTimer.isActive() && !m_lastPageLaunched)
    {
        previousPageTimer.stop();
        previousPage();
    }
    m_firstPageLaunched = false;
}

void LibraryPageHandler::jumpToPage(int sliderPage)
{
    qDebug() << Q_FUNC_INFO << "currentPage: " << m_currentPage << " sliderPage: " << sliderPage;

    if(m_currentPage != sliderPage || b_sliderMoved)
    {
        m_currentPage = (sliderPage >= m_totalPages) ? m_totalPages - 1 : sliderPage;
        emit goToLibraryToPage(m_currentPage);
    }

    b_sliderMoved = false;
}

void LibraryPageHandler::sliderMoved(int page)
{
    qDebug() << Q_FUNC_INFO;

    if(page == m_currentPage || page >= m_totalPages) return;

    m_currentPage = page;
    b_sliderMoved = true;
    m_strPagesText = QString(tr("%1 of %2")).arg(page+1).arg(m_totalPages);
    pageLabel->setText(m_strPagesText);
}

void LibraryPageHandler::setPage( int page )
{
    qDebug() << Q_FUNC_INFO;
    m_strPagesText = QString(tr("%1 of %2")).arg(page).arg(m_totalPages);
    m_currentPage = page -1;
    pagerSlider->updateSliderPosition(page-1);
    pageLabel->setText(m_strPagesText);
}

void LibraryPageHandler::handleSwipe(int direction)
{
    if(!isVisible() || m_totalPages <= 1)
        return;

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

void LibraryPageHandler::setup( int totalPages, int currentPage, bool loopable, bool scrollBar )
{
    qDebug() << Q_FUNC_INFO;
    pageLabel->show();

    m_totalPages = (totalPages != 0) ? totalPages : 1;
    pagerSlider->initSlider(m_totalPages);
    m_loopable = loopable;
    b_scrollBar = scrollBar;
    setPage(currentPage);
    m_currentPage = currentPage - 1;

    if(totalPages > 1)
    {
        forwardButton->show();
        backButton->show();
    }
    else
    {
        forwardButton->hide();
        backButton->hide();
    }

    if(totalPages <= 2)
        pagerSlider->hide();
    else
        pagerSlider->show();
}

void LibraryPageHandler::hidePages()
{
    pageLabel->hide();
    forwardButton->hide();
    backButton->hide();
    pagerSlider->hide();
}
