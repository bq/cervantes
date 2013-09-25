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


#include "BookLocation.h"

BookLocation::BookLocation()
    : pos(0)
    , text("")
    , note("")
    , bookmark("")
    , page(0)
    , lastUpdated(0)
    , chapterName("")
    {}

BookLocation::BookLocation(Type _type, Operation _operation, const QString& _preview, const QString& _bookmark, double _pos, int _page, qint64 _lastUpdated, const QString& _chapterName)
    : type(_type)
    , operation(_operation)
    , pos(_pos)
    , text(_preview)
    , bookmark(_bookmark)
    , page(_page)
    , lastUpdated(_lastUpdated)
    , chapterName(_chapterName)
    {}

BookLocation::BookLocation(Type _type, Operation _operation, const QString& _preview, const QString& _bookmark, double _pos, int _page, const QString& _note, qint64 _lastUpdated, const QString& _chapterName)
    : type(_type)
    , operation(_operation)
    , pos(_pos)
    , text(_preview)
    , note(_note)
    , bookmark(_bookmark)
    , page(_page)
    , lastUpdated(_lastUpdated)
    , chapterName(_chapterName)
    {}

BookLocation::BookLocation(const BookLocation& other)
    : type(other.type)
    , operation(other.operation)
    , pos(other.pos)
    , text(other.text)
    , note(other.note)
    , bookmark(other.bookmark)
    , page(other.page)
    , lastUpdated(other.lastUpdated)
    , chapterName(other.chapterName)
    {}

QString BookLocation::getStringFromType(BookLocation::Type type)
{
    if(type == BOOKMARK) {
        return "BOOKMARK";
    } else if(type == NOTE){
        return "NOTE";
    } else if(type == HIGHLIGHT){
        return "HIGHLIGHT";
    } else
        return "UNKNOWN";
}

QString BookLocation::getStringFromOperation(BookLocation::Operation operation)
{
    if(operation == DELETE)
        return "DELETE";
    else if (operation == ADD)
        return "ADD";
    else if (operation == UPDATE)
        return "UPDATE";
    else
        return "";
}

BookLocation::Type BookLocation::getTypeFromString(const QString& type)
{
    if(type == "BOOKMARK") {
        return BOOKMARK;
    } else if(type == "NOTE"){
        return NOTE;
    } else
        return HIGHLIGHT;
}

BookLocation::Operation BookLocation::getOperationFromString(const QString& operation)
{
    if(operation == "DELETE")
        return DELETE;
    else if(operation == "ADD")
        return ADD;
    else if(operation == "UPDATE")
        return UPDATE;
    else
        return EMPTY;
}

