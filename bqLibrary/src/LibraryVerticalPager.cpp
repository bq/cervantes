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

#include "LibraryVerticalPager.h"

#include "QBook.h"
#include "MouseFilter.h"
#include "QBookApp.h"

#include <QFile>
#include <QPainter>
#include <QDebug>

#define INT_ROUND_VALUE 0.5
#define TIME_FOR_LONG_PRESS 700

LibraryVerticalPager::LibraryVerticalPager(QWidget* parent) :
    QWidget(parent), m_strPagesText()
{
    setupUi(this);
    connect(downButton,             SIGNAL(pressed()),                 this,          SLOT(nextPagePressed()));
    connect(upButton,               SIGNAL(pressed()),                 this,          SLOT(previousPagePressed()));
    connect(downButton,             SIGNAL(released()),                this,          SLOT(nextPageReleased()));
    connect(upButton,               SIGNAL(released()),                this,          SLOT(previousPageReleased()));
    connect(QBookApp::instance(),   SIGNAL(swipe(int)),                this,          SLOT(handleSwipe(int)));
}

LibraryVerticalPager::~LibraryVerticalPager()
{
}

void LibraryVerticalPager::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void LibraryVerticalPager::nextPage()
{
    Screen::getInstance()->queueUpdates();
    nextPageTimer.stop();

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
    Screen::getInstance()->flushUpdates();
}

void LibraryVerticalPager::previousPage()
{
    Screen::getInstance()->queueUpdates();
    previousPageTimer.stop();

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
    Screen::getInstance()->flushUpdates();
}

void LibraryVerticalPager::nextPagePressed()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    connect(&nextPageTimer, SIGNAL(timeout()),  this, SLOT(goToLastPage()), Qt::UniqueConnection);
    nextPageTimer.setSingleShot(true);
    nextPageTimer.start(TIME_FOR_LONG_PRESS);
}

void LibraryVerticalPager::previousPagePressed()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    connect(&previousPageTimer, SIGNAL(timeout()), this, SLOT(goToFirstPage()), Qt::UniqueConnection);
    previousPageTimer.setSingleShot(true);
    previousPageTimer.start(TIME_FOR_LONG_PRESS);
}

void LibraryVerticalPager::goToLastPage()
{
    qDebug() << Q_FUNC_INFO;
    nextPageTimer.stop();
    emit lastPage();
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}
void LibraryVerticalPager::goToFirstPage()
{
    qDebug() << Q_FUNC_INFO;
    previousPageTimer.stop();
    emit firstPage();
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void LibraryVerticalPager::nextPageReleased()
{
    if(nextPageTimer.isActive())
    {
        nextPageTimer.stop();
        nextPage();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
    }
}

void LibraryVerticalPager::previousPageReleased()
{
    if(previousPageTimer.isActive())
    {
        previousPageTimer.stop();
        previousPage();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
    }
}

void LibraryVerticalPager::setPage( int page )
{
    m_currentPage = page;
    m_strPagesText = QString(tr("%1 of %2")).arg(m_currentPage).arg(m_totalPages);
    pageLabel->setText(m_strPagesText);
}

void LibraryVerticalPager::handleSwipe(int direction)
{
    qDebug()<< Q_FUNC_INFO << "Direction" << direction;
    if(m_totalPages <= 1) return;
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

void LibraryVerticalPager::setup( int totalPages, int currentPage, bool loopable )
{
    qDebug() << Q_FUNC_INFO;
    pageLabel->show();

    m_totalPages = (totalPages != 0) ? totalPages : 1;
    m_loopable = loopable;
    setPage(currentPage);

    if(totalPages > 1)
    {
        upButton->show();
        downButton->show();
    }
    else
    {
        upButton->hide();
        downButton->hide();
    }
}

void LibraryVerticalPager::hidePages()
{
    pageLabel->hide();
    upButton->hide();
    downButton->hide();
}

