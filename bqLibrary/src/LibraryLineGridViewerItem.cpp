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

#include "LibraryLineGridViewerItem.h"

#include "BookInfo.h"
#include "bqUtils.h"
#include "QBookApp.h"

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QPainter>
#include <QDebug>

#define FOLDER_MAX_LENGTH 48
#define TITLE_MAX_LENGTH 55
#define AUTHOR_MAX_LENGTH 50

LibraryLineGridViewerItem::LibraryLineGridViewerItem(QWidget* parent) :
    LibraryGridViewerItem(parent)
{
    setupUi(this);

    newTagLbl->raise();
    readingProgressLbl->raise();
    sampleTagLbl->raise();
    //subscriptionTagLbl->raise();
    subscriptionTagLbl->hide();
    selectBookLbl->raise();
    downloadBtn->hide();
    archiveTagLbl->hide();
}

LibraryLineGridViewerItem::~LibraryLineGridViewerItem()
{}

void LibraryLineGridViewerItem::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}


void LibraryLineGridViewerItem::setBook( const BookInfo* book, Library::ELibraryFilterMode filter )
{
    LibraryGridViewerItem::setBook(book, filter);
    m_path = book->path;

    // Hide elements
    selectBookLbl->hide();

    // Icon
    if(book->isCorrupted())
        iconTypeFileLbl->setStyleSheet("background-image:url(:/res/corrupted_list.png)");
    QString imageCover = QBookApp::instance()->getImageResource(book->path, true);
    iconTypeFileLbl->setStyleSheet("background-image:url(" + imageCover + ")");

    // Title
    titleLbl->setText(bqUtils::truncateStringToLength(book->title, TITLE_MAX_LENGTH));

    // Author
    if(book->author == "---")
        authorLbl->setText(tr("Autor Desconocido"));
    else
        authorLbl->setText(bqUtils::truncateStringToLength(book->author, AUTHOR_MAX_LENGTH));
    authorLbl->show();

    int percentage = book->readingProgress * 100;
    readingProgressLbl->setText(QString::number(percentage) + "%");
    readingProgressLbl->show();

    if(book->m_type == BookInfo::BOOKINFO_TYPE_DEMO)
        sampleTagLbl->show();
    else
        sampleTagLbl->hide();

    /*if(book->m_type == BookInfo::BOOKINFO_TYPE_SUBSCRIPTION)
        subscriptionTagLbl->show();
    else
        subscriptionTagLbl->hide();*/

    int state = book->readingStatus;
    switch(state)
    {
    case BookInfo::NO_READ_BOOK:
        if(book->lastReadLink.isEmpty())
            newTagLbl->show();
        else
            newTagLbl->hide();
        break;
    case BookInfo::READ_BOOK:
    case BookInfo::READING_BOOK:
        newTagLbl->hide();
        break;
    }
}

void LibraryLineGridViewerItem::setFile( const QFileInfo* file )
{
    LibraryGridViewerItem::setFile(file);
    m_path = file->filePath();

    // Hide elements
    selectBookLbl->hide();
    authorLbl->hide();
    newTagLbl->hide();
    sampleTagLbl->hide();

    if(file->isDir())
    {
        QDir dir(file->filePath());
        // Title
        // NOTE (epaz) dir->count()-2 (the 2 is because . and .. are included in the count)
        titleLbl->setText(bqUtils::truncateStringToLength(file->fileName(), FOLDER_MAX_LENGTH));

        readingProgressLbl->setText("(" + QString::number(dir.count()-2) + ")");
        readingProgressLbl->show();

        // Cover
        iconTypeFileLbl->setStyleSheet("background-image:url(:/res/folder_list_big.png)");
    }
    else
    {
        // Title
        titleLbl->setText(bqUtils::truncateStringToLength(file->fileName(), AUTHOR_MAX_LENGTH));

        readingProgressLbl->hide();

        const QString suffix = file->suffix();

        if(QBookApp::instance()->isImage(file->fileName()))
            iconTypeFileLbl->setStyleSheet("background-image:url(:/res/generic_image_list_big.png)");
        else
            iconTypeFileLbl->setStyleSheet("background-image:url(:/res/unknow_list_big.png)");

    }
}

void LibraryLineGridViewerItem::setArrow( const QString& path )
{
    LibraryGridViewerItem::setArrow(path);
    m_path = path;

    authorLbl->hide();
    newTagLbl->hide();
    sampleTagLbl->hide();
    readingProgressLbl->hide();
    unselect();
    //subscriptionTagLbl->hide();

    titleLbl->setText(tr("Volver"));
    iconTypeFileLbl->setStyleSheet("background-image:url(:/res/up_level_list.png)");
}

void LibraryLineGridViewerItem::select()
{
    selectBookLbl->show();
}

void LibraryLineGridViewerItem::unselect()
{
    selectBookLbl->hide();
}
