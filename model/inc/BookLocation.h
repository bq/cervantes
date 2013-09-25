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

#ifndef BOOKLOCATION_H
#define BOOKLOCATION_H

#include <QString>

class BookLocation
{

public:

    enum Type
    {
        ALL_IN_BOOK = 0,
        BOOKMARK,
        NOTE,
        HIGHLIGHT,

        TYPE_COUNT
    };


    enum Operation
    {
        ADD,
        DELETE,
        UPDATE,
        EMPTY
    };


    Type type;
    Operation operation;
    double pos;
    QString text;
    QString note;
    QString bookmark;
    int page;
    qint64 lastUpdated;
    QString chapterName;

    BookLocation();
    BookLocation(Type _type, Operation _operation, const QString& _preview, const QString& _bookmark, double _pos, int _page, qint64 _lastUpdated, const QString& chapterName);
    BookLocation(Type _type, Operation _operation, const QString& _preview, const QString& _bookmark, double _pos, int _page, const QString& _note, qint64 _lastUpdated, const QString& chapterName);

    BookLocation(const BookLocation& other);

    static QString getStringFromType(Type);
    static Type getTypeFromString(const QString&);
    static QString getStringFromOperation(Operation);
    static Operation getOperationFromString(const QString&);

};
#endif // BOOKLOCATION_H
