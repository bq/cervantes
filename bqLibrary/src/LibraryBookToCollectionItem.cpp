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

#include "LibraryBookToCollectionItem.h"

#include "BookInfo.h"
#include "bqUtils.h"
#include "GestureWidget.h"

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QPainter>
#include <QDebug>

#define FOLDER_MAX_LENGTH 48
#define TITLE_MAX_LENGTH 55
#define AUTHOR_MAX_LENGTH 50

LibraryBookToCollectionItem::LibraryBookToCollectionItem(QWidget* parent) : GestureWidget(parent) , m_book(NULL)
{
    setupUi(this);
    m_checked = false;

    connect(addBookToCollectionBtn, SIGNAL(clicked()), this, SLOT(handleClicked()));
}

LibraryBookToCollectionItem::~LibraryBookToCollectionItem()
{}

void LibraryBookToCollectionItem::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void LibraryBookToCollectionItem::setBook(const BookInfo *book)
{
    m_book = book;
    bookTitleLabel->setText(book->title);
    if(book->author == "---")
        booksNumberLlb->setText(tr("Autor Desconocido"));
    else
        booksNumberLlb->setText(book->author);
    bookTitleLabel->show();
    booksNumberLlb->show();
    addBookToCollectionBtn->show();
}

void LibraryBookToCollectionItem::setChecked(bool checked)
{
    m_checked = checked;
    addBookToCollectionBtn->setChecked(checked);
}

void LibraryBookToCollectionItem::handleClicked()
{
    qDebug() << Q_FUNC_INFO;
    setChecked(!m_checked);
    emit changeStatus();
}

void LibraryBookToCollectionItem::hideItem()
{
    bookTitleLabel->hide();
    booksNumberLlb->hide();
    addBookToCollectionBtn->hide();
}
