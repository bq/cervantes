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

#include <QPainter>
#include <QList>
#include <QDebug>
#include "BookInfo.h"
#include "GestureWidget.h"
#include "HomeBooksListItem.h"
#include "QBookApp.h"
#include "Library.h"
#include "bqUtils.h"

#define ICONITEM_STRING_MAX_LENGTH 13

HomeBooksListItem::HomeBooksListItem(QWidget *parent) : GestureWidget(parent){

        setupUi(this);
        connect(this,SIGNAL(tap()),this,SLOT(processTap()));
        subscriptionTagLbl->hide();
}

HomeBooksListItem::~HomeBooksListItem(){
    // do nothing
}

void HomeBooksListItem::paintEvent (QPaintEvent *){
        QStyleOption opt;
        opt.init(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void HomeBooksListItem::paintBook (const BookInfo* _book){
        book = _book;

        if(book == 0){
            coverLbl->hide();
            return;
        }

        if(book->m_type == BookInfo::BOOKINFO_TYPE_DEMO)
            sampleTagLbl->show();
        else
            sampleTagLbl->hide();

        /*if(book->m_type == BookInfo::BOOKINFO_TYPE_SUBSCRIPTION)
            subscriptionTagLbl->show();
        else
            subscriptionTagLbl->hide();*/

        coverLbl->show();
        newTagLbl->hide();
        if (book->thumbnail == NO_COVER_RESOURCE || book->thumbnail.isEmpty())
        {
            // Title
            titleLabel->setText(bqUtils::truncateStringToLength(book->title, ICONITEM_STRING_MAX_LENGTH));
            titleLabel->show();

            // Author
            if(book->author == "--")
                authorLabel->setText(tr("Autor Desconocido"));
            else
                authorLabel->setText(bqUtils::truncateStringToLength(book->author, ICONITEM_STRING_MAX_LENGTH));
            authorLabel->show();

            // Cover
            coverLbl->setPixmap(NULL);
            QString imageCover = QBookApp::instance()->getImageResource(book->path);
            coverLbl->setStyleSheet("background-image:url(" + imageCover + ")");
        }
        else{
            titleLabel->hide();
            authorLabel->hide();
            QPixmap pixmap(book->thumbnail);
            coverLbl->setPixmap(pixmap.scaled(coverLbl->size(),Qt::KeepAspectRatioByExpanding));
        }
}

void HomeBooksListItem::processTap() {
    qDebug() << "--->" << Q_FUNC_INFO;
    emit openContent(book);
}
