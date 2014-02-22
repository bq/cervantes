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

#ifndef METADATAEXTRACTOR_H
#define METADATAEXTRACTOR_H

#include <QString>
#include <QDateTime>

#define UNDEFINED_LANGUAGE "und"

struct MetaData
{
    QString   filename;
    QString   title;
    QString   author;
    QString   publisher;
    QDateTime date;
    QString   description;
    bool      isDRMBook;
};

class MetaDataExtractor {

public:
    static bool getMetaData       (const QString& filename, QString& title, QString& author, QString& publisher, QDateTime& date, QString& description, QString& format, bool& isDRMBook, QString& collection, QString& language);
    static bool extractCover      (const QString& filename, QString& coverPath);
    static QStringList extractCSS (const QString& epubFilename);
    static QString getLanguage    (const QString& filename);
};

#endif  //METADATAEXTRACTOR_H
