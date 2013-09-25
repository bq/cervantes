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

#define DELETE_DATA_FILE      ".delete.xml"

#include "DeleteLaterFile.h"
#include "BookInfo.h"
#include "Model.h"

#include <QDir>
#include <QDebug>

DeleteLaterFile::DeleteLaterFile(){
}

DeleteLaterFile::~DeleteLaterFile() {
}


void DeleteLaterFile::sync (const QString& /*path*/){

}

void DeleteLaterFile::insert (const QList<const BookInfo*>& bookList, const QString& privatePartition){

    qDebug() << Q_FUNC_INFO ;
    QFile file(privatePartition + QDir::separator() +  DELETE_DATA_FILE);
    file.open(QFile::ReadWrite);

    file.seek(0);
    file.resize(0);

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(-1);

    xml.writeStartDocument();
    xml.writeStartElement("books");

    QList<const BookInfo*>::const_iterator i = bookList.constBegin();
    QList<const BookInfo*>::const_iterator itEnd = bookList.constEnd();
    while(i != itEnd){
            xml.writeStartElement("book");
            qDebug() << Q_FUNC_INFO << "isbn: " << (*i)->isbn << " title: " <<(*i)->title ;
            insertBookInfo((*i), xml);
            xml.writeEndElement();
            ++i;
    }

    xml.writeEndElement();
    xml.writeEndDocument();
    file.close();
}

void DeleteLaterFile::insertBookInfo (const BookInfo* info, QXmlStreamWriter &xml){

    qDebug() << Q_FUNC_INFO << "isbn: " << info->isbn;
    xml.writeTextElement("path", info->path);
    xml.writeTextElement("isbn", info->isbn);
    xml.writeTextElement("title", info->title);
}

void DeleteLaterFile::exec ( const QString& privatePartition, Model* model, const QString& viewerBookPath, bool& viewerBookInList, bool &booksDeleted ){

    qDebug() << Q_FUNC_INFO;

    QFile file(privatePartition + QDir::separator() +  DELETE_DATA_FILE);
    if(!file.exists()) return;
    file.open(QFile::ReadWrite);

    QXmlStreamReader xml(&file);
    xml.readNext(); //Skip first tag
    xml.readNext();
    if(xml.name() == "books"){
        while (!xml.atEnd() && !xml.hasError()) {
            xml.readNext();
            if (xml.name() == "book" && xml.isStartElement()){
                BookInfo* info = loadBookInfo(xml);
                qDebug() << Q_FUNC_INFO << "viewer Book:" << viewerBookPath;
                if(viewerBookPath.size() && QDir::cleanPath(info->path) == QDir::cleanPath(viewerBookPath)){
                    qDebug() << Q_FUNC_INFO << "viewer book to delete";
                    viewerBookInList = true;
                }
                qDebug() << Q_FUNC_INFO << "before delete book";
                deleteBook(info, model);
                booksDeleted = true;
                delete info;
            }
        }
    }
    file.close();
    file.remove();
}

BookInfo* DeleteLaterFile::loadBookInfo(QXmlStreamReader& xml){
    qDebug() << Q_FUNC_INFO << "xml";
    BookInfo* book = new BookInfo();
    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if(xml.isEndElement()){
            if(xml.name() == "book")
                break;
        }else {
            QStringRef name = xml.name();

            if (name == "isbn")
                book->isbn = xml.readElementText();
            else if (name == "path")
                book->path = xml.readElementText();
            else if (name == "title")
                book->title = xml.readElementText();
        }
    }
    qDebug() << Q_FUNC_INFO << "returning book";
    return book;
}

void DeleteLaterFile::deleteBook(const BookInfo * bookInfo, Model* model)
{
    qDebug() << Q_FUNC_INFO << "path: " << bookInfo->path << " isbn: " << bookInfo->isbn  << " title: " << bookInfo->title;

    const BookInfo * bookInfoModel = model->getBookInfo(bookInfo->path);

    if(bookInfoModel)
        model->removeBook(bookInfoModel);
}
