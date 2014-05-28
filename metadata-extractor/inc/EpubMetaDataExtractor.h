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

#ifndef EPUBMETADATAEXTRACTOR_H
#define EPUBMETADATAEXTRACTOR_H

#include <QString>
#include <epub.h>
#include <QDateTime>

class EpubMetaDataExtractor {

public:
    static bool getMetaData(const QString& filename, QString& title, QString& author, QString& publisher, QDateTime& date, QString& description, bool& isDRMBook, QString& language);
    static QString *getMetaString(struct epub *book, epub_metadata field, const char *stringOffset);
    static bool extractCover(const QString& epubFilename, const QString& coverPath);
    static QStringList extractCSS(const QString& epubFilename);
    static bool isDRM(const QString& filename);
    static QString getCollection(const QString& epubFilename);
    static double getCollectionIndex(const QString& epubFilename);



private:
    static struct epub *openEpub(const QString& filename);
};

#endif // EPUBMETADATAEXTRACTOR_H
