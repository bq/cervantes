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


#include "bqLibepubUtils.h"
#include <epub.h>
#include <QDebug>
#include <QStringList>

//Return 0 if the nodes are the same. 1 if readLink is greather than currentReadLink and -1 if currentReadLink is greater than readLink
int bqLibepubUtils::compareReadLink(const QString currentReadLink, const QString readLink, const QString path) //static
{
    qDebug() << Q_FUNC_INFO;
    QStringList parts = readLink.split("#");
    if(parts.size() <= 1)
        return -1;
    QString chapter = parts[0];
    QString point = parts[1];

    QStringList currentParts = currentReadLink.split("#");
    if(currentParts.size() <= 1)
        return 1;
    QString currentChapter = currentParts[0];
    QString currentPoint = currentParts[1];

    //Open the spine to find the chapters
    const char * pathBook = path.toAscii().data();
    struct epub *book;
    book = epub_open(pathBook, 0);
    struct eiterator *eit = epub_get_iterator(book, EITERATOR_SPINE, 0);

    if(chapter.compare(currentChapter) == 0){
        epub_free_iterator(eit);
        epub_close(book);
        if(compareNodes(currentPoint, point) == 0)
            return 0;
        else if(compareNodes(currentPoint, point) > 0)
            return 1;
        else
            return -1;
    }

    char* res;
    int pos = 0;

    do {
        QString result = QString(epub_it_get_curr_url(eit));
        qDebug() << "result: " << result << "new chapter " << chapter << " currentChapter: " << currentChapter;
        if(chapter.contains(result)){
            epub_free_iterator(eit);
            epub_close(book);
            return -1;
        }else if (currentChapter.contains(result)){
            epub_free_iterator(eit);
            epub_close(book);
            return 1;
        }else{
            res = epub_it_get_next(eit);
            pos++;
            qDebug() << "chapter:" << result.data() << " Pos: "<< pos;
        }
    }while (res);

    return 0;
}

//Return 0 if the nodes are the same. 1 if node2 is greather than node1 and -1 if node1 is greater than node2
int bqLibepubUtils::compareNodes(const QString node1, const QString node2) //static
{
    qDebug() << Q_FUNC_INFO;
    QStringList parts = node2.split("/");
    QStringList currentParts = node1.split("/");
    int i=0;

    while(i < parts.size() -1 && i < currentParts.size() -1)
    {
        if(parts[i].toInt() < currentParts[i].toInt())
            return -1;
        else if(parts[i].toInt() > currentParts[i].toInt())
            return 1;
        else{
            i++;
            if(i == parts.size() -1 && i < currentParts.size() -1)
                return -1;
            else if(i == currentParts.size() -1 && i < parts.size() -1 )
                return 1;
        }
    }
    QStringList listLastNode = parts[i].split(":");
    QStringList listCurrentLastNode =currentParts[i++].split(":");

    if(listLastNode.size() <= 1)
        return -1;

    if(listCurrentLastNode.size() <= 1)
        return 1;

    qDebug() << "listLastNode" << listLastNode << " listCurrentLastNode " << listCurrentLastNode;
    QString lastNode = listLastNode[1];
    QString currentLastNode = listCurrentLastNode[1];
    //Delete the last character ")"
    lastNode.chop(1);
    currentLastNode.chop(1);

    if(lastNode.toInt() < currentLastNode.toInt())
        return -1;
    else if(lastNode.toInt() > currentLastNode.toInt())
        return 1;
    else
        return 0;
}

