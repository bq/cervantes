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

#ifndef DELETELATERFILE_H
#define DELETELATERFILE_H

#include "Model.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QHash>

class BookInfo;

class DeleteLaterFile : public QObject
{

public:

    DeleteLaterFile();
    virtual ~DeleteLaterFile();

    void                        sync                    ( const QString& = "");
    static void                 exec                    ( const QString& privatePartition, Model* model, const QString& viewerBookPath, bool& viewerBookInList, bool &booksDeleted  );
    static BookInfo*            loadBookInfo            ( QXmlStreamReader&);
    static void                 deleteBook              ( const BookInfo*, Model* model);
    static void                 insert                  ( const QList<const BookInfo*>&, const QString& privatePartition );
    static void                 insertBookInfo          ( const BookInfo*, QXmlStreamWriter&);

};
#endif // DELETELATERFILE_H
