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

#include "LibraryIconGridViewerItem.h"

#include "BookInfo.h"
#include "bqUtils.h"
#include "QBook.h"
#include "QBookApp.h"
#include "Library.h"
#include "QBookApp.h"

#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QPainter>
#include <QDebug>

#define ICONITEM_STRING_MAX_LENGTH 13

LibraryIconGridViewerItem::LibraryIconGridViewerItem(QWidget* parent) :
    LibraryGridViewerItem(parent)
{
    setupUi(this);

    newTagLbl->show();
    readingTagLbl->show();
    sampleTagLbl->show();
    subscriptionTagLbl->hide();
    selectBookLbl->show();
    archiveTagLbl->hide();
    downloadBtn->hide();
}

LibraryIconGridViewerItem::~LibraryIconGridViewerItem()
{
}

void LibraryIconGridViewerItem::paintEvent(QPaintEvent*)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void LibraryIconGridViewerItem::setBook( const BookInfo* book, Library::ELibraryFilterMode filter )
{
    m_path = book->path;

    if(book->isCorrupted())
    {
        setThumbnailElements(false);
        fileNameLbl->show();
        //subscriptionTagLbl->hide();
        QFileInfo fileInfo(book->path);
        fileNameLbl->setText(bqUtils::truncateStringToLength(fileInfo.completeBaseName(), ICONITEM_STRING_MAX_LENGTH));
        coverLabel->setPixmap(NULL);
        coverLabel->setStyleSheet("image:url(:/res/corrupted_book_ico.png)");
        return;
    }

    setThumbnailElements(true);

    // Cover
    QString coverPath;
    if( !book->thumbnail.isEmpty()) {
        QFileInfo fi(book->thumbnail);
        if(fi.exists())
            coverPath = book->thumbnail;
    }

    if (coverPath == NO_COVER_RESOURCE || coverPath.isEmpty())
    {
        // Title
        titleLbl->setText(bqUtils::truncateStringToLength(book->title, ICONITEM_STRING_MAX_LENGTH));
        titleLbl->show();

        // Author
        if(book->author == "--")
            authorLbl->setText(tr("Autor Desconocido"));
        else
            authorLbl->setText(bqUtils::truncateStringToLength(book->author, ICONITEM_STRING_MAX_LENGTH));
        authorLbl->show();

        // Cover
        coverLabel->setPixmap(NULL);
        QString imageCover = QBookApp::instance()->getImageResource(book->path);
        coverLabel->setStyleSheet("background-image:url(" + imageCover + ")");
    }
    else
    {
        titleLbl->hide();
        authorLbl->hide();

        // Cover
        coverLabel->setPixmap(NULL);
        QPixmap pixmap(book->thumbnail);
        coverLabel->setPixmap(pixmap.scaled(coverLabel->size(),Qt::KeepAspectRatioByExpanding));
    }


    if(book->m_type == BookInfo::BOOKINFO_TYPE_DEMO)
        sampleTagLbl->show();
    else
        sampleTagLbl->hide();

    /*if(book->m_type == BookInfo::BOOKINFO_TYPE_SUBSCRIPTION)
        subscriptionTagLbl->show();
    else
        subscriptionTagLbl->hide();*/

    if(book->m_archived)
    {
        newTagLbl->hide();
        readingTagLbl->hide();
    }
    else
    {
        int state = book->readingStatus;
        switch(state)
        {
        case BookInfo::NO_READ_BOOK:
            readingTagLbl->hide();
            if(book->lastReadLink.isEmpty())
                newTagLbl->show();
            else
                newTagLbl->hide();
            break;
        case BookInfo::READING_BOOK:
            newTagLbl->hide();
            readingTagLbl->show();
            break;
        case BookInfo::READ_BOOK:
            newTagLbl->hide();
            if(book->lastReadLink.isEmpty())
                readingTagLbl->hide();
            else
                readingTagLbl->show();
            break;
        }
    }
}


void LibraryIconGridViewerItem::setThumbnailElements(bool visible)
{
    //qDebug() << Q_FUNC_INFO << visible;

    if(visible)
    {
        selectBookLbl->hide();
        bottomLeftLbl->show();
        bottomRightLbl->show();
        bottomCenterLbl->show();
        centerLeftLbl->show();
        centerRightLbl->show();
        topLeftLbl->show();
        topRightLbl->show();
        topCenterLbl->show();
        fileNameLbl->hide();

    }
    else
    {
        // Hide elements
        selectBookLbl->hide();
        authorLbl->hide();
        newTagLbl->hide();
        titleLbl->hide();
        readingTagLbl->hide();
        sampleTagLbl->hide();
        //subscriptionTagLbl->hide();

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

void LibraryIconGridViewerItem::setFile( const QFileInfo* file )
{
    qDebug() << Q_FUNC_INFO << file->baseName() << file->fileName();

    m_path = file->filePath();
    //subscriptionTagLbl->hide();
    QString fileName = file->fileName();

    setThumbnailElements(false);

    fileNameLbl->show();
    coverLabel->setPixmap(NULL);
    if(file->isDir())
    {
        QDir dir(m_path);
        // Title
        // NOTE (epaz) dir->count()-2 (the 2 is because . and .. are included in the count)
        fileNameLbl->setText(bqUtils::truncateStringToLength(fileName, ICONITEM_STRING_MAX_LENGTH) + "(" + QString::number(dir.count()-2) + ")");

        // Cover
        coverLabel->setStyleSheet("background-image:url(:/res/folder_ico.png)");
    }
    else
    {
        const QString fileName = file->fileName();

        // Title        
        QString text = fontMetrics().elidedText(fileName, Qt::ElideMiddle, width ());
        fileNameLbl->setText(text);



        if(QBookApp::instance()->isImage(fileName))
            coverLabel->setStyleSheet("background-image:url(:/res/generic_image_file_ico.png)");
        else
            coverLabel->setStyleSheet("background-image:url(:/res/unknow_file_ico.png)");

    }
}

void LibraryIconGridViewerItem::setArrow( const QString& path )
{
    m_path = path;

    // Hide elements
    selectBookLbl->hide();
    authorLbl->hide();
    newTagLbl->hide();
    titleLbl->hide();
    readingTagLbl->hide();
    sampleTagLbl->hide();
    //subscriptionTagLbl->hide();

    bottomLeftLbl->hide();
    bottomRightLbl->hide();
    bottomCenterLbl->hide();
    centerLeftLbl->hide();
    centerRightLbl->hide();
    topLeftLbl->hide();
    topRightLbl->hide();
    topCenterLbl->hide();


    fileNameLbl->setText(tr("Volver"));
    fileNameLbl->show();
    coverLabel->setPixmap(NULL);
    coverLabel->setStyleSheet("image:url(:/res/up_level.png)");
}

void LibraryIconGridViewerItem::select()
{
    selectBookLbl->show();
}

void LibraryIconGridViewerItem::unselect()
{
    selectBookLbl->hide();
}
