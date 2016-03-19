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

#include "SearchItemShop.h"
#include "BookInfo.h"
#include "bqUtils.h"

#include <QDebug>
#include <QString>
#include <QPainter>

#define SEARCH_MAX_RESULT_TEXT_LENGTH 60

SearchItemShop::SearchItemShop (QWidget* parent) : GestureWidget(parent){
    qDebug() << "--->" << Q_FUNC_INFO;
    connect(this, SIGNAL(tap()), this, SLOT(handleTap()));
    setupUi(this);
}

SearchItemShop::~SearchItemShop (){
        qDebug() << "--->" << Q_FUNC_INFO;
}

void SearchItemShop::setBook (const BookInfo *_book){
        book = _book;
}

const BookInfo* SearchItemShop::getBook () const {
        return book;
}

void SearchItemShop::paint ()
{

        qDebug() << "--->" << Q_FUNC_INFO;
        QString title, author;

        title = bqUtils::truncateStringToLength(book->title,SEARCH_MAX_RESULT_TEXT_LENGTH);
        author = bqUtils::truncateStringToLength(book->author,SEARCH_MAX_RESULT_TEXT_LENGTH);

        searchTitle->setText(title);
        searchAuthor->setText(author);

}

void SearchItemShop::handleTap() {
        qDebug() << "--->" << Q_FUNC_INFO;
        emit openStore(book);
}

void SearchItemShop::paintEvent (QPaintEvent *)
{
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
