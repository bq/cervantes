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

#include "LibraryBookSummary.h"

#include "Library.h"
#include "BookInfo.h"
#include "LibraryBookListActions.h"
#include "bqUtils.h"
#include "QBookApp.h"
#include "Storage.h"
#include "Screen.h"

#include <QFileInfo>
#include <QFile>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QScrollBar>


#define STRING_MAX_LENGTH 28
#define ICON_LIBRARY_STRING_MAX_LENGTH 10
#define PERCENT_STEP_VALUE 0.75

LibraryBookSummary::LibraryBookSummary( QWidget* parent ) :
    GestureWidget(parent)
  , m_bookInfo(NULL)
{
    setupUi(this);
    connect(closePopupBtn,  SIGNAL(clicked()),  this, SIGNAL(hideMe()));
    connect(buyBookBtn,     SIGNAL(clicked()),  this, SLOT(buyBookClicked()));
    connect(copyToSDBtn,    SIGNAL(clicked()),  this, SLOT(copyBook()));
    connect(exportNotesBtn, SIGNAL(clicked()),  this, SLOT(exportNotes()));
    connect(archiveBtn,     SIGNAL(clicked()),  this, SLOT(archiveBook()));
    connect(removeBtn,      SIGNAL(clicked()),  this, SLOT(removeBook()));
    connect(moreActionsBtn, SIGNAL(clicked()),  this, SLOT(moreActionsClicked()));

    m_bookListActions = new LibraryBookListActions(this);
    m_bookListActions->hide();
    connect(m_bookListActions,  SIGNAL(readStateChanged(int)),   this, SLOT(changeReadState(int)));

    connect(VerticalPagination,   SIGNAL(previousPageReq()),   this, SLOT(synopsisUp()));
    connect(VerticalPagination,   SIGNAL(nextPageReq()),       this, SLOT(synopsisDown()));
    summaryTextBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vbar = summaryTextBrowser->verticalScrollBar();

}

LibraryBookSummary::~LibraryBookSummary()
{
    delete m_bookListActions;
    m_bookListActions = NULL;
}

void LibraryBookSummary::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void LibraryBookSummary::setThumbnailElements(bool visible)
{
    qDebug() << Q_FUNC_INFO << visible;

    if(visible)
    {
        bottomLeftLbl->show();
        bottomRightLbl->show();
        bottomCenterLbl->show();
        centerLeftLbl->show();
        centerRightLbl->show();
        topLeftLbl->show();
        topRightLbl->show();
        topCenterLbl->show();
    }
    else
    {
        bottomLeftLbl->hide();
        bottomRightLbl->hide();
        bottomCenterLbl->hide();
        centerLeftLbl->hide();
        centerRightLbl->hide();
        topLeftLbl->hide();
        topRightLbl->hide();
        topCenterLbl->hide();
    }
}

void LibraryBookSummary::setBook( const BookInfo* book )
{
    qDebug() << Q_FUNC_INFO;

    buyBookBtn->hide();
    priceCont->hide();
    bookPriceLbl->hide();
    m_bookInfo = book;

    QList<BookLocation::Type> types;
    types.append(BookLocation::NOTE);
    types.append(BookLocation::HIGHLIGHT);
    int countMarks = book->getLocationList(types)->size();

    // Cover
    QString coverPath;
    if( !book->thumbnail.isEmpty())
    {
        QFileInfo fi(book->thumbnail);
        if(fi.exists())
            coverPath = book->thumbnail;
    }

    if (coverPath == NO_COVER_RESOURCE || coverPath.isEmpty())
    {
        // Title
        titleLbl->setText(bqUtils::truncateStringToLength(book->title, ICON_LIBRARY_STRING_MAX_LENGTH));
        titleLbl->show();

        // Author
        if(book->author == "--")
            authorLbl->setText(bqUtils::truncateStringToLength(tr("Autor Desconocido"), ICON_LIBRARY_STRING_MAX_LENGTH));
        else
            authorLbl->setText(bqUtils::truncateStringToLength(book->author, ICON_LIBRARY_STRING_MAX_LENGTH));
        authorLbl->show();

        setThumbnailElements(true);

        // Cover
        coverLbl->setPixmap(NULL);
        coverLbl->setStyleSheet("image:url(:/res/no_cover.png)");
    }
    else if(book->isCorrupted())
    {
        titleLbl->hide();
        authorLbl->hide();
        setThumbnailElements(false);

        // Cover
        coverLbl->setPixmap(NULL);
        coverLbl->setStyleSheet("image:url(" + coverPath + ")");
    }
    else
    {
        titleLbl->hide();
        authorLbl->hide();
        setThumbnailElements(true);

        // Cover
        coverLbl->setPixmap(NULL);
        QPixmap pixmap(coverPath);
        coverLbl->setPixmap(pixmap.scaled(coverLbl->size(),Qt::KeepAspectRatioByExpanding));
    }

    // Title
    bookTitleLbl->setText(bqUtils::truncateStringToLength(book->title, STRING_MAX_LENGTH));

    // Author
    if(book->author == "--")
        bookAuthorLbl->setText(tr("Autor Desconocido"));
    else
        bookAuthorLbl->setText(bqUtils::truncateStringToLength(book->author, STRING_MAX_LENGTH));

    // Synopsis
    summaryTextBrowser->setText(book->synopsis);
    //Define a single step as the 75% of the normal height in the widget.
    vbar->setSingleStep(summaryTextBrowser->height()*PERCENT_STEP_VALUE);
    if(!m_bookInfo->lastReadLink.isEmpty() && book->pageCount > 0)
    {
        int pages = book->pageCount;
        numberTotalPagesLbl->setText(QString::number(pages));
        numberPagesCont->show();
    }
    else
        numberPagesCont->hide();

    m_bookListActions->setButtonsState(m_bookInfo->readingStatus);
    setActionsBtnText(m_bookInfo->readingStatus);

    if(m_bookInfo->m_type != BookInfo::BOOKINFO_TYPE_DEMO && !m_bookInfo->m_archived && countMarks > 0)
        exportNotesBtn->show();
    else
        exportNotesBtn->hide();

    if(!m_bookInfo->path.contains(Storage::getInstance()->getPrivatePartition()->getMountPoint()) )
    {
        if(m_bookInfo->path.contains(Storage::getInstance()->getPublicPartition()->getMountPoint()) )
            copyToSDBtn->setText(tr("Copy"));

        copyToSDBtn->show();
        removeBtn->show();
        archiveBtn->hide();
    }
    else
    {
        copyToSDBtn->hide();

        if(m_bookInfo->m_type == BookInfo::BOOKINFO_TYPE_DEMO)
        {
            buyBookBtn->show();
            QString euro = QString::fromUtf8("\u20AC");
            QString storePrice = book->storePrice + euro;
            if(!storePrice.isEmpty())
            {
                bookPriceLbl->setText(storePrice);
                buyBookBtn->show();
                priceCont->show();
                removeBtn->show();
                archiveBtn->hide();
            }
        }
        else
        {
            removeBtn->hide();
            archiveBtn->show();

            if(m_bookInfo->m_archived)
                archiveBtn->setText(QApplication::translate("LibraryBookSummary", "Unarchive", 0, QApplication::UnicodeUTF8));
            else
                archiveBtn->setText(QApplication::translate("LibraryBookSummary", "Archive", 0, QApplication::UnicodeUTF8));
        }
    }

    summaryTextBrowser->verticalScrollBar()->hide();
    setupPagination(1);
}

void LibraryBookSummary::close()
{
    m_bookListActions->hide();
    hide();
}

void LibraryBookSummary::buyBookClicked()
{
    qDebug() << Q_FUNC_INFO;
    if(m_bookListActions)
        m_bookListActions->hide();

    emit buyBook(m_bookInfo);
}

void LibraryBookSummary::moreActionsClicked()
{
    qDebug() << Q_FUNC_INFO;

    if(!m_bookListActions->isVisible())
    {
        Screen::getInstance()->queueUpdates();
        QPoint pos(moreActionsBtn->mapToGlobal(QPoint(0,0)));
        pos.setY(pos.y() + moreActionsBtn->height());
        m_bookListActions->move(mapFromGlobal(pos));
        m_bookListActions->resize(moreActionsBtn->width(), m_bookListActions->height());
        m_bookListActions->show();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
    }
    else
    {
        Screen::getInstance()->queueUpdates();
        m_bookListActions->hide();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
    }
}

void LibraryBookSummary::copyBook()
{
    qDebug() << Q_FUNC_INFO;

    emit copyBook(m_bookInfo->path);
}

void LibraryBookSummary::exportNotes()
{
    qDebug() << Q_FUNC_INFO;
    emit exportNotes(m_bookInfo->path);
}

void LibraryBookSummary::archiveBook()
{
    if(m_bookInfo->m_archived)
        emit unarchiveBook(m_bookInfo);
    else
        emit archiveBook(m_bookInfo);

    setBook(m_bookInfo);
}

void LibraryBookSummary::removeBook()
{
    qDebug() << Q_FUNC_INFO;
    emit removeBook(m_bookInfo);
}

void LibraryBookSummary::changeReadState(int state)
{
    emit changeReadState(m_bookInfo, (BookInfo::readStateEnum)state);
}

void LibraryBookSummary::mouseReleaseEvent( QMouseEvent* event )
{
    qDebug() << Q_FUNC_INFO;

    if(m_bookListActions->isVisible())
    {
        event->accept();
        m_bookListActions->hide();
    }
}

void LibraryBookSummary::synopsisDown()
{
    qDebug() << Q_FUNC_INFO;

    if (vbar)
    {
         vbar->triggerAction(QAbstractSlider::SliderSingleStepAdd);
         int page;
         if(vbar->value() % vbar->singleStep() == 0)
             page = vbar->value() / vbar->singleStep() + 1;
         else
             page = vbar->value() / vbar->singleStep() + 2;
         VerticalPagination->setPage(page);
    }
}

void LibraryBookSummary::synopsisUp()
{
    qDebug() << Q_FUNC_INFO;

    if (vbar)
    {
        vbar->triggerAction(QAbstractSlider::SliderSingleStepSub);
        int page;
        if(vbar->value() % vbar->singleStep() == 0)
            page = vbar->value() / vbar->singleStep() + 1;
        else
            page = vbar->value() / vbar->singleStep() + 2;
        VerticalPagination->setPage(page);
    }
}

void LibraryBookSummary::setupPagination (int initialPage){

    qDebug() << Q_FUNC_INFO;
    if(vbar){
        int totalPages;
        if(vbar->maximum() % vbar->singleStep() == 0)
            totalPages = vbar->maximum() / vbar->singleStep() + 1;
        else
            totalPages = vbar->maximum() / vbar->singleStep() + 2;
        VerticalPagination->setup(totalPages, initialPage, false, true);
    }
    VerticalPagination->hidePages();
}

//Needed to handle screen refresh
void LibraryBookSummary::hideListActionsMenu()
{
    qDebug() << Q_FUNC_INFO;
    if(m_bookListActions->isVisible())
        m_bookListActions->hide();
}

void LibraryBookSummary::setActionsBtnText(BookInfo::readStateEnum state)
{
    switch(state)
    {
    case BookInfo::READING_BOOK:
        moreActionsBtn->setText(tr("Leyendo"));
        break;
    case BookInfo::READ_BOOK:
        moreActionsBtn->setText(tr("Leido"));
        break;
    case BookInfo::NO_READ_BOOK: default:
        moreActionsBtn->setText(tr("No leido"));
        break;
    }
}
