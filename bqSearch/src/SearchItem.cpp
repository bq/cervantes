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

#include "SearchItem.h"
#include "BookInfo.h"
#include "QBookApp.h"
#include "Model.h"

#include "bqUtils.h"

#include <QDebug>
#include <QString>
#include <QPainter>
#include <QFileInfo>

#define SEARCH_MAX_RESULT_TEXT_LENGTH 60

SearchItem::SearchItem (QWidget* parent) : GestureWidget(parent){
    qDebug() << "--->" << Q_FUNC_INFO;
    connect(this, SIGNAL(tap()), this, SLOT(handleTap()));
    setupUi(this);
}

SearchItem::~SearchItem (){
        qDebug() << "--->" << Q_FUNC_INFO;
}

void SearchItem::setBook (const BookInfo *_book){
        book = _book;
        file = NULL;
}

void SearchItem::setFile (const QFileInfo * _file){
        file = _file;
        book = NULL;
}

const BookInfo* SearchItem::getBook () const {
        return book;
}

void SearchItem::paint ()
{
        qDebug() << "--->" << Q_FUNC_INFO;

        if(book)
            paintBook();
        else if(file)
            paintFile();
}

void SearchItem::paintBook()
{
    qDebug() << "--->" << Q_FUNC_INFO;
    searchTitle->setText(book->title);
    if(book->author == "--")
        searchAuthor->setText(tr("Autor Desconocido"));
    else
        searchAuthor->setText(book->author);
    QString imageCover = QBookApp::instance()->getImageResource(book->path, true);
    fileTypeLbl->setStyleSheet("background-image:url(" + imageCover + ")");
}

void SearchItem::paintFile()
{
    qDebug() << "--->" << Q_FUNC_INFO;

    Model* model = QBookApp::instance()->getModel();
    const BookInfo* bookItem = model->getBookInfo(file->absoluteFilePath());
    if(bookItem)
    {
        searchTitle->setText(bookItem->title);
        if(bookItem->author == "--")
            searchAuthor->setText(tr("Autor Desconocido"));
        else
            searchAuthor->setText(bookItem->author);
        QString cover = QBookApp::instance()->getImageResource(file->absoluteFilePath(), true);
        fileTypeLbl->setStyleSheet(" background-image:url(:" + cover + ")");
    }
    else if(file->isDir()) {
        searchTitle->setText(file->fileName());
        searchAuthor->hide();
        fileTypeLbl->setStyleSheet("background-image:url(:/res/folder_list.png)");
    }
    else if(QBookApp::instance()->isImage(file->fileName())) {
        searchTitle->setText(file->completeBaseName());
        searchAuthor->hide();
        fileTypeLbl->setStyleSheet("background-image:url(:/res/generic_image_list_big.png)");
    }
    else {
        searchTitle->setText(file->completeBaseName());
        searchAuthor->hide();
        fileTypeLbl->setStyleSheet(" background-image:url(:/res/unknow_list.png)");
    }
}

void SearchItem::handleTap()
{
        qDebug() << "--->" << Q_FUNC_INFO;
        if(file && file->absoluteFilePath()!=NULL)
            emit openPath(file->absoluteFilePath());
        if(book && book->path != NULL)
            emit openPath(book->path);
}

void SearchItem::paintEvent (QPaintEvent *)
{
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
