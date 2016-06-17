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

#include "ViewerPageHandlerLandscape.h"
#include "Viewer.h"
#include "Screen.h"
#include "bqUtils.h"

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

ViewerPageHandlerLandscape::ViewerPageHandlerLandscape(QWidget *parent) :
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

    if(QBook::getInstance()->getResolution() == QBook::RES1072x1448)
        m_chapterLineWidth = WIDTH_LINE_HD;
    else if(QBook::getInstance()->getResolution() == QBook::RES600x800)
        m_chapterLineWidth = WIDTH_LINE_SD;
    else
        m_chapterLineWidth = WIDTH_LINE_HD;

    titleLandscapeLbl->setAlignment(Qt::AlignVCenter |  Qt::AlignLeft);
    readingPercentLbl->setAlignment(Qt::AlignVCenter |  Qt::AlignLeft);
    pagToFinishLbl->setAlignment(Qt::AlignVCenter |  Qt::AlignRight);
}

ViewerPageHandlerLandscape::~ViewerPageHandlerLandscape()
{
    qDebug() << Q_FUNC_INFO;
}

void ViewerPageHandlerLandscape::updatePageHandler()
{
    hide();
    if (!shouldBeShown()) return;

    if ( (m_visibleMask & EVPHM_PROGRESS) && m_currentPageIni != 0)
    {
        readingPercentLbl->show();
        noReadingPercentLbl->hide();
    }
    else
    {
        readingPercentLbl->hide();
        noReadingPercentLbl->show();
    }

    if ( (m_visibleMask & EVPHM_PAGE) && m_currentPageIni != 0)
    {
        pageText->show();
        noPageText->hide();
    }
    else
    {
        pageText->hide();
        noPageText->show();
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
        noPagToFinishLbl->hide();
    }
    else
    {
        pagToFinishLbl->hide();
        noPagToFinishLbl->show();
    }

    if((m_visibleMask & EVPHM_BOOK_TITLE) && m_currentPageIni != 0)
    {
        titleLandscapeLbl->show();
        titleLeftMargin->show();
        noTitleLandscapeLbl->hide();
    }
    else
    {
        titleLandscapeLbl->hide();
        titleLeftMargin->hide();
        noTitleLandscapeLbl->show();
    }

    if((m_visibleMask & EVPHM_DATE_TIME) && m_currentPageIni != 0)
    {
        dateTimeLandscapeLbl->show();
    }
    else
    {
        dateTimeLandscapeLbl->hide();
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

void ViewerPageHandlerLandscape::handlePageChange(int start, int end, int total)
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

void ViewerPageHandlerLandscape::setCurrentReadingPercent(int value)
{
    readingPercentLbl->setText(tr("%1% reading").arg(value));
}

void ViewerPageHandlerLandscape::setChapterPage(int page)
{
    pagToFinishLbl->setText(tr("%1 pag. to chap. end").arg(page));
}

void ViewerPageHandlerLandscape::updateBookTitle(const QString& title)
{

    titleLandscapeLbl->setText(bqUtils::truncateStringToWidth(title, titleLandscapeLbl->height(), titleLandscapeLbl->font()));
}

void ViewerPageHandlerLandscape::updateDateTime(const QString& dateTime)
{
    dateTimeLandscapeLbl->setText(dateTime);
}

void ViewerPageHandlerLandscape::updateDisplay()
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

void ViewerPageHandlerLandscape::setCurrentPageMode( bool isPdf )
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

    if(QBook::settings().value("setting/showBookTitle", QVariant(false)).toBool())
        m_visibleMask |= EVPHM_BOOK_TITLE;
    else
        m_visibleMask &= (~EVPHM_BOOK_TITLE);

    if(QBook::settings().value("setting/showDateTime", QVariant(false)).toBool())
        m_visibleMask |= EVPHM_DATE_TIME;
    else
        m_visibleMask &= (~EVPHM_DATE_TIME);
    if(QBook::settings().value("setting/showChapterLines", QVariant(true)).toBool())
        m_visibleMask |= EVPHM_CHAPTERLINE;
    else
        m_visibleMask &= (~EVPHM_CHAPTERLINE);

    updatePageHandler();
}


bool ViewerPageHandlerLandscape::shouldBeShown()
{
    if (m_pdfToolbarShowed) return false;
    if (m_visibleMask)      return true;
    return false;
}

void ViewerPageHandlerLandscape::hideBackBtn()
{
    m_visibleMask &= (~EVPHM_BACK_BTN);
    hide();
    pageBackBtn->hide();
    if(shouldBeShown())
        show();
}

void ViewerPageHandlerLandscape::showBackBtn()
{
    m_visibleMask |= EVPHM_BACK_BTN;
    pageBackBtn->show();
    if(shouldBeShown())
    {
        hide();
        show();
    }
}

void ViewerPageHandlerLandscape::hideTitle()
{
    m_visibleMask &= (~EVPHM_BOOK_TITLE);
    if(!shouldBeShown()) hide();
    titleLandscapeLbl->hide();
    titleLeftMargin->hide();
    noTitleLandscapeLbl->show();
}

void ViewerPageHandlerLandscape::showTitle()
{
    m_visibleMask |= EVPHM_BOOK_TITLE;
    titleLandscapeLbl->show();
    titleLeftMargin->show();
    noTitleLandscapeLbl->hide();
}

void ViewerPageHandlerLandscape::hideDateTime()
{
    m_visibleMask &= (~EVPHM_DATE_TIME);
    if(!shouldBeShown()) hide();
    dateTimeLandscapeLbl->hide();
}

void ViewerPageHandlerLandscape::showDateTime()
{
    m_visibleMask |= EVPHM_DATE_TIME;
    dateTimeLandscapeLbl->show();
}

bool ViewerPageHandlerLandscape::dateTimeisVisible()
{
    return dateTimeLandscapeLbl->isVisible();
}

void ViewerPageHandlerLandscape::resetPager()
{
    qDebug() << Q_FUNC_INFO;
    m_currentPageIni = 0;
    m_currentPageEnd = 0;
    titleLandscapeLbl->hide();
    titleLeftMargin->hide();
    noTitleLandscapeLbl->hide();
    readingPercentLbl->hide();
    noReadingPercentLbl->hide();
    pageText->hide();
    noPageText->hide();
    pagToFinishLbl->hide();
    noPagToFinishLbl->hide();
    dateTimeLandscapeLbl->hide();
    readingProgressCont->hide();
    readingProgress->hide();
    hideChapterLines();
    setNeedToPaint(true);
    if (!(m_visibleMask & EVPHM_PDF)) pdfMenuBtn->hide();
}

void ViewerPageHandlerLandscape::hideChapterInfo()
{
    m_visibleMask &= (~EVPHM_CHAPTER);
    if(!shouldBeShown()) hide();
    pagToFinishLbl->hide();
    noPagToFinishLbl->show();
}

void ViewerPageHandlerLandscape::showEvent( QShowEvent* )
{
    if((m_visibleMask & EVPHM_PROGRESSBAR) && (m_visibleMask & EVPHM_CHAPTERLINE) && b_needToRepaint && m_currentPageIni != 0)
    {
        hideChapterLines();
        double realWidth = readingProgress->rect().height();
        //Create labels for each chapter and pos them into progressBar.
        for(int i = 0; i < linesPos.size(); i++)
        {
            QLabel* chapterLineLbl = new QLabel(readingProgressCont);
            chapterLineLbl->setFixedSize(readingProgress->width(),2);
            QImage myImage;
            myImage.load(":/res/chapter-progressbar.png");
            QImage image = myImage.scaled(chapterLineLbl->width(), chapterLineLbl->height(), Qt::IgnoreAspectRatio );
            chapterLineLbl->setPixmap(QPixmap::fromImage(image));
            int y = int((linesPos[i])*100/m_totalPages);
            double pos = double(y) * realWidth/100;
            chapterLineLbl->move(readingProgress->x(), pos + readingProgress->y());
            chapterLineLbl->show();
        }
        setNeedToPaint(false);
    }
}

void ViewerPageHandlerLandscape::setChapterPos(const QList<int> chaptersPage)
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

void ViewerPageHandlerLandscape::hideChapterLines()
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
