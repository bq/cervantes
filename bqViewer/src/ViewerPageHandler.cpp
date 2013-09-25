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

#include "ViewerPageHandler.h"
#include "Viewer.h"
#include "Screen.h"

#include <QWidget>
#include <QDebug>
#include <QLabel>
#include <QString>
#include <QPoint>

#include "QBook.h"

const QString SINGLE_PAGE_FORMAT("%1/%2");
const QString DOUBLE_PAGE_FORMAT("%1-%2/%3");

ViewerPageHandler::ViewerPageHandler(QWidget *parent) :
    QWidget(parent)
  , m_currentPageIni(0)
  , m_currentPageEnd(0)
  , m_visibleMask(EVPHM_NONE)
  , m_pdfToolbarShowed(false)
{
    qDebug() << Q_FUNC_INFO;

    setupUi(this);
    connect(pageBackBtn,            SIGNAL(clicked()), this, SIGNAL(goBack()));
    connect(pdfMenuBtn,             SIGNAL(clicked()), this, SIGNAL(pdfMenuBtnClicked()));

    readingProgress->setMinimum(0);
    readingProgress->setMaximum(100);
}

ViewerPageHandler::~ViewerPageHandler()
{
    qDebug() << Q_FUNC_INFO;
}

void ViewerPageHandler::mousePressEvent(QMouseEvent* received)
{
    TouchEvent *event = new TouchEvent(received->pos(), MouseFilter::TAP);
    emit pressEvent(event);
    received->accept();
}

void ViewerPageHandler::updatePageHandler()
{

    if (!shouldBeShown()) { hide(); return; }


    if ( (m_visibleMask & EVPHM_PROGRESS) && m_currentPageIni != 0)
    {
        readingPercentLbl->show();
        readedLbl->show();
        spacerNoPercentNoBakcLbl->hide();
    }
    else
    {
        readingPercentLbl->hide();
        readedLbl->hide();
        spacerNoPercentNoBakcLbl->show();
    }

    if ( (m_visibleMask & EVPHM_PAGE) && m_currentPageIni != 0)
    {
        pageText->show();
    }
    else
    {
        pageText->hide();
    }

    if ( m_visibleMask & EVPHM_PDF )
    {
        pdfMenuBtn->show();
    }
    else
    {
        pdfMenuBtn->hide();
    }

    if((m_visibleMask & EVPHM_CHAPTER) && m_currentPageIni != 0)
    {
        pagToFinishLbl->show();
        toFinishChapLbl->show();
    }
    else
    {
        pagToFinishLbl->hide();
        toFinishChapLbl->hide();
    }

    if((m_visibleMask & EVPHM_PROGRESSBAR) && m_currentPageIni != 0)
    {
        readingProgress->show();
    }
    else
    {
        readingProgress->hide();
    }

    show();
}

void ViewerPageHandler::handlePageChange(int start, int end, int total)
{
    qDebug() << Q_FUNC_INFO << start << end << total;

    if(start == 0) resetPager();
    // TODO: review tweak to avoid refresh on longpress

    m_totalPages = total; // To include final page for opinion

    if(m_currentPageIni != start || m_currentPageEnd != end)
    {
        m_currentPageIni = start;
        m_currentPageEnd = end;

        updateDisplay();
    }
    qDebug() << Q_FUNC_INFO << "finished";
}

void ViewerPageHandler::setCurrentPage(int value)
{
    readingPercentLbl->setText(QString("%1%").arg(value));
}

void ViewerPageHandler::setChapterPage(int page)
{
    pagToFinishLbl->setText(QString("%1").arg(page));
}

void ViewerPageHandler::updateDisplay()
{
    qDebug() << Q_FUNC_INFO;

    if(m_currentPageIni == 0) // Hide page on cover
    {
        pageText->hide();
        readingPercentLbl->hide();
        return;
    }

    if(m_currentPageIni != m_currentPageEnd)
        pageText->setText(DOUBLE_PAGE_FORMAT.arg(m_currentPageIni).arg(m_currentPageEnd).arg(m_totalPages));
    else
        pageText->setText(SINGLE_PAGE_FORMAT.arg(m_currentPageIni).arg(m_totalPages));

    int percent = 0;
    if(m_totalPages != 0)
        percent = int((m_currentPageEnd)*100/m_totalPages);
    readingPercentLbl->setText(QString("%1%").arg(percent));// FIXME: ¿por qué hay un método setCurrentPage?
    Screen::getInstance()->queueUpdates();
    readingProgress->setValue(percent);

    update();
    updatePageHandler();
    Screen::getInstance()->flushUpdates();
}

void ViewerPageHandler::setCurrentPageMode( bool isPdf )
{
    if(isPdf)
        m_visibleMask |= EVPHM_PDF;
    else
        m_visibleMask &= (~EVPHM_PDF);

    if(QBook::settings().value("setting/showProgressBar", false).toBool())
        m_visibleMask |= EVPHM_PROGRESSBAR;
    else
        m_visibleMask &= (~EVPHM_PROGRESSBAR);

    if(QBook::settings().value("setting/showPage", true).toBool())
        m_visibleMask |= EVPHM_PAGE;
    else
        m_visibleMask &= (~EVPHM_PAGE);

    if(QBook::settings().value("setting/showProgress", true).toBool())
        m_visibleMask |= EVPHM_PROGRESS;
    else
        m_visibleMask &= (~EVPHM_PROGRESS);

    if(QBook::settings().value("setting/showChapterInfo", true).toBool())
        m_visibleMask |= EVPHM_CHAPTER;
    else
        m_visibleMask &= (~EVPHM_CHAPTER);

    updatePageHandler();
}


bool ViewerPageHandler::shouldBeShown()
{
    if(m_pdfToolbarShowed) return false;

    if(m_visibleMask != EVPHM_NONE)
        return true;

    return false;
}

void ViewerPageHandler::hideBackBtn()
{
    m_visibleMask &= (~EVPHM_BACK_BTN);
    if(!shouldBeShown()) hide();
    pageBackBtn->hide();
    spacerNoPercentLbl->show();
}

void ViewerPageHandler::showBackBtn()
{
    m_visibleMask |= EVPHM_BACK_BTN;
    if(shouldBeShown()) show();
    pageBackBtn->show();
    spacerNoPercentLbl->hide();
}

void ViewerPageHandler::resetPager()
{
    qDebug() << Q_FUNC_INFO;
    m_currentPageIni = 0;
    m_currentPageEnd = 0;
    pageText->hide();
    readingPercentLbl->hide();
    readedLbl->hide();
    pagToFinishLbl->hide();
    toFinishChapLbl->hide();
    readingProgress->hide();
    if (!(m_visibleMask & EVPHM_PDF)) pdfMenuBtn->hide();
}

void ViewerPageHandler::hideChapterInfo()
{
    m_visibleMask &= (~EVPHM_CHAPTER);
    if(!shouldBeShown()) hide();
    pagToFinishLbl->hide();
    toFinishChapLbl->hide();
}
