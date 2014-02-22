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

#define WIDTH_LINE_HD 2
#define WIDTH_LINE_SD 1

const QString SINGLE_PAGE_FORMAT("%1/%2");
const QString DOUBLE_PAGE_FORMAT("%1-%2/%3");

ViewerPageHandler::ViewerPageHandler(QWidget *parent) :
    QWidget(parent)
  , m_currentPageIni(0)
  , m_currentPageEnd(0)
  , m_pdfToolbarShowed(false)
  , b_needToRepaint(true)
  , m_visibleMask(EVPHM_NONE)  
{
    qDebug() << Q_FUNC_INFO;

    setupUi(this);
    connect(pageBackBtn,            SIGNAL(clicked()), this, SIGNAL(goBack()));
    connect(pdfMenuBtn,             SIGNAL(clicked()), this, SIGNAL(pdfMenuBtnClicked()));

    readingProgress->setMinimum(0);
    readingProgress->setMaximum(100);
    readingProgress->setStyleSheet("background:transparent;");

    if(QBook::getResolution() == QBook::RES600x800)
        m_chapterLineWidth = WIDTH_LINE_SD;
    else
        m_chapterLineWidth = WIDTH_LINE_HD;

    // Install Mouse filter
    m_pMouseFilter = new MouseFilter(this);
    this->installEventFilter(m_pMouseFilter);
}

ViewerPageHandler::~ViewerPageHandler()
{
    qDebug() << Q_FUNC_INFO;

    delete m_pMouseFilter;
    m_pMouseFilter = NULL;
}

void ViewerPageHandler::customEvent(QEvent* received)
{
    if (received->type() != (int)MouseFilter::TOUCH_EVENT)
    {
        qDebug() << "UNEXPECTED TYPE";
        return;
    }

    TouchEvent *event = static_cast<TouchEvent*>(received);
    if (event->touchType() == MouseFilter::TAP) emit tapEvent(event, true);
    else                                        emit touchEvent(event);
}

void ViewerPageHandler::updatePageHandler()
{

    hide();
    if (!shouldBeShown()) return;
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
        readingProgressCont->show();
        readingProgress->show();
    }
    else
    {
        readingProgressCont->hide();
        readingProgress->hide();
    }
    if(!(m_visibleMask & EVPHM_CHAPTERLINE) || m_currentPageIni == 0)
        hideChapterLines();

    show();
}

void ViewerPageHandler::handlePageChange(int start, int end, int total)
{
    qDebug() << Q_FUNC_INFO << start << end << total;

    if(start == 0) resetPager();
    // TODO: review tweak to avoid refresh on longpress


    if(m_currentPageIni != start || m_currentPageEnd != end || m_totalPages != total)
    {
        m_currentPageIni = start;
        m_currentPageEnd = end;
        m_totalPages = total;

        updateDisplay();
    }
    qDebug() << Q_FUNC_INFO << "finished";
}

void ViewerPageHandler::setCurrentReadingPercent(int value)
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

    Screen::getInstance()->queueUpdates();
    Screen::getInstance()->setMode(Screen::MODE_QUICK, true, FLAG_WAITFORCOMPLETION, Q_FUNC_INFO);

    if(m_currentPageIni != 0) // Hide page on cover
    {
        if(m_currentPageIni != m_currentPageEnd)
            pageText->setText(DOUBLE_PAGE_FORMAT.arg(m_currentPageIni).arg(m_currentPageEnd).arg(m_totalPages));
        else
            pageText->setText(SINGLE_PAGE_FORMAT.arg(m_currentPageIni).arg(m_totalPages));

        int percent = 0;
        if(m_totalPages != 0) percent = int((m_currentPageEnd)*100/m_totalPages);
        setCurrentReadingPercent(percent);
        readingProgress->setValue(percent);
    }

    updatePageHandler();

    Screen::getInstance()->flushUpdates();
}

void ViewerPageHandler::setCurrentPageMode( bool isPdf )
{
    setNeedToPaint(true);
    if(isPdf)
        m_visibleMask |= EVPHM_PDF;
    else
        m_visibleMask &= (~EVPHM_PDF);

    if(QBook::settings().value("setting/showProgressBar", QVariant(false)).toBool())
        m_visibleMask |= EVPHM_PROGRESSBAR;
    else
        m_visibleMask &= (~EVPHM_PROGRESSBAR);

    if(QBook::settings().value("setting/showPage", QVariant(true)).toBool())
        m_visibleMask |= EVPHM_PAGE;
    else
        m_visibleMask &= (~EVPHM_PAGE);

    if(QBook::settings().value("setting/showProgress", QVariant(true)).toBool())
        m_visibleMask |= EVPHM_PROGRESS;
    else
        m_visibleMask &= (~EVPHM_PROGRESS);

    if(QBook::settings().value("setting/showChapterInfo", QVariant(true)).toBool())
        m_visibleMask |= EVPHM_CHAPTER;
    else
        m_visibleMask &= (~EVPHM_CHAPTER);

    if(QBook::settings().value("setting/showChapterLines", QVariant(true)).toBool())
        m_visibleMask |= EVPHM_CHAPTERLINE;
    else
        m_visibleMask &= (~EVPHM_CHAPTERLINE);

    updatePageHandler();
}


bool ViewerPageHandler::shouldBeShown()
{
    if (m_pdfToolbarShowed) return false;
    if (m_visibleMask)      return true;
    return false;
}

void ViewerPageHandler::hideBackBtn()
{
    m_visibleMask &= (~EVPHM_BACK_BTN);
    hide();
    pageBackBtn->hide();
    spacerNoPercentLbl->show();
    if(shouldBeShown())
        show();
}

void ViewerPageHandler::showBackBtn()
{
    m_visibleMask |= EVPHM_BACK_BTN;
    pageBackBtn->show();
    if(shouldBeShown())
    {
        hide();
        show();
    }
    spacerNoPercentLbl->hide();
}

void ViewerPageHandler::resetPager()
{
    qDebug() << Q_FUNC_INFO;
    //m_currentPageIni = 0;
    //m_currentPageEnd = 0;
    pageText->hide();
    readingPercentLbl->hide();
    readedLbl->hide();
    pagToFinishLbl->hide();
    toFinishChapLbl->hide();
    readingProgressCont->hide();
    readingProgress->hide();
    hideChapterLines();
    setNeedToPaint(true);
    if (!(m_visibleMask & EVPHM_PDF)) pdfMenuBtn->hide();
}

void ViewerPageHandler::hideChapterInfo()
{
    m_visibleMask &= (~EVPHM_CHAPTER);
    if(!shouldBeShown()) hide();
    pagToFinishLbl->hide();
    toFinishChapLbl->hide();
}

void ViewerPageHandler::showEvent( QShowEvent* )
{
    qDebug() << Q_FUNC_INFO;
    if((m_visibleMask & EVPHM_PROGRESSBAR) && (m_visibleMask & EVPHM_CHAPTERLINE) && b_needToRepaint && m_currentPageIni != 0)
    {
        hideChapterLines();
        double realWidth = readingProgress->rect().width();
        //Create labels for each chapter and pos them into progressBar.
        for(int i = 0; i < linesPos.size(); i++)
        {
            QLabel* chapterLineLbl = new QLabel(readingProgressCont);
            chapterLineLbl->setFixedSize(m_chapterLineWidth,readingProgress->height());
            QImage myImage;
            myImage.load(":/res/chapter-progressbar.png");
            QImage image = myImage.scaled(chapterLineLbl->width(), chapterLineLbl->height(), Qt::IgnoreAspectRatio );
            chapterLineLbl->setPixmap(QPixmap::fromImage(image));
            int x = int((linesPos[i])*100/m_totalPages);
            double pos = double(x) * realWidth/100 - chapterLineLbl->width()/2;
            qDebug() << "Posicion" << pos;
            chapterLineLbl->move(pos,readingProgress->y());
            chapterLineLbl->show();
        }
        setNeedToPaint(false);
    }
}

void ViewerPageHandler::setChapterPos(const QList<int> chaptersPage)
{
    qDebug() << Q_FUNC_INFO;
    linesPos.clear();
    int chaptersCount = chaptersPage.size();
    for (int i = 0; i < chaptersCount; i++)
    {
        int page = chaptersPage[i];
        linesPos.append(page);
    }
}

void ViewerPageHandler::hideChapterLines()
{
    //Hide all labels that are child of readingProgressCont.
    QList<QLabel*> list = readingProgressCont->findChildren<QLabel *>();
    for(int j = 0; j < list.size();j++)
    {
        QLabel* previousLine = list[j];
        previousLine->hide();
        delete previousLine;
    }
}
