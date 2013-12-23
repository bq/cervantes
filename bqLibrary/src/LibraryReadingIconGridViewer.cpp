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

#include "LibraryReadingIconGridViewer.h"

#include "Library.h"
#include "BookInfo.h"
#include "bqUtils.h"
#include "QBookApp.h"

#include <QFileInfo>
#include <QFile>
#include <QPainter>
#include <QDebug>

const int s_itemsPerPage = 3;
#define COVER_STRING_MAX_LENGTH 15
#define STRING_MAX_LENGTH 35

LibraryReadingIconGridViewer::LibraryReadingIconGridViewer(QWidget* parent) :
    LibraryGridViewer(parent)
{
    setupUi(this);
    connect(mainBookInfoCont, SIGNAL(tap()), this, SLOT(openMostRecent()));
    connect(mainBookInfoCont, SIGNAL(longPressStart()), this, SLOT(showSummary()));

    subscriptionTagLbl->hide();
}

LibraryReadingIconGridViewer::~LibraryReadingIconGridViewer()
{}

void LibraryReadingIconGridViewer::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

LibraryPageHandler* LibraryReadingIconGridViewer::getPageHandler()
{
    return VerticalPagerHandler;
}

int LibraryReadingIconGridViewer::getItemsPerPage()
{
    return s_itemsPerPage;
}

void LibraryReadingIconGridViewer::getBookItems( LibraryGridViewerItem* books[] )
{
    books[1] = book1;
    books[2] = book2;
    books[3] = book3;
}

void LibraryReadingIconGridViewer::openMostRecent(){
    qDebug() << Q_FUNC_INFO << "last book clicked";
    emit itemClicked(m_library->m_books.at(0)->path);
}

void LibraryReadingIconGridViewer::showSummary(){
    qDebug() << Q_FUNC_INFO << "last book clicked";
    emit itemLongPress(m_library->m_books.at(0)->path);
}

void LibraryReadingIconGridViewer::paintBooks()
{
    qDebug() << Q_FUNC_INFO;

    if(m_library->m_books.size())
    {
        const BookInfo* bookInfo = m_library->m_books.at(0);

        // Cover
        QString coverPath;
        if( !bookInfo->thumbnail.isEmpty())
        {
            QFileInfo fi(bookInfo->thumbnail);
            if(fi.exists())
                coverPath = bookInfo->thumbnail;
        }

        if(bookInfo->m_type == BookInfo::BOOKINFO_TYPE_DEMO)
        {
            sampleTagLbl->show();
        }
        else
        {
            sampleTagLbl->hide();

            /*if(bookInfo->m_type == BookInfo::BOOKINFO_TYPE_SUBSCRIPTION)
                subscriptionTagLbl->show();
            else
                subscriptionTagLbl->hide();*/
        }

        newTagLbl->hide();

        if (coverPath == NO_COVER_RESOURCE || coverPath.isEmpty())
        {
            //Cover
            currentBookCoverLbl->setPixmap(NULL);
            QString imageCover = QBookApp::instance()->getImageResource(bookInfo->path);
            currentBookCoverLbl->setStyleSheet("background-image:url(" + imageCover + ")");
            // Title
            coverTitleLbl->setText(bqUtils::truncateStringToLength(bookInfo->title, COVER_STRING_MAX_LENGTH));
            coverTitleLbl->show();
            // Author
            if(bookInfo->author == "---")
                coverAuthorLbl->setText(tr("Autor Desconocido"));
            else
                coverAuthorLbl->setText(bqUtils::truncateStringToLength(bookInfo->author, COVER_STRING_MAX_LENGTH));
            coverAuthorLbl->show();
        }
        else{
            QPixmap pixmap(bookInfo->thumbnail);
            currentBookCoverLbl->setPixmap(pixmap.scaled(currentBookCoverLbl->size(),Qt::KeepAspectRatioByExpanding));
            coverTitleLbl->hide();
            coverAuthorLbl->hide();
        }

        // Title
        bookTitleLbl->setText(bqUtils::truncateStringToLength(bookInfo->title, STRING_MAX_LENGTH));

        // Author
        if(bookInfo->author == "--")
            authorLbl->setText(tr("Autor Desconocido"));
        else
            authorLbl->setText(bqUtils::truncateStringToLength(bookInfo->author, STRING_MAX_LENGTH));

        int percentage = bookInfo->readingProgress * 100;
        readingPercentLbl->setText(QString::number(percentage, 'g', 3));

        dateAccessLbl->setText(getDateStyled(bookInfo->lastTimeRead, true));
        lastAccessLbl->show();
        dateAccessLbl->show();
    }
    LibraryGridViewer::paintBooks();
}
