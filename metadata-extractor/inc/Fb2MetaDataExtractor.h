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

#ifndef FB2METADATAEXTRACTOR_H
#define FB2METADATAEXTRACTOR_H

#include "MetaDataExtractor.h"

#define TXT_MAX_FILE_SIZE       (16 * 1024 * 1024)

class Fb2MetaDataExtractor
{
public:
    static bool getMetaData  (MetaData &data);
    static bool getMetaData  (const QString& fb2Filename, QString& title, QString& author, QString& publisher, QDateTime& date, QString& description, bool& isDRMBook, QString& language);
    static bool extractCover (const QString& fb2Filename, const QString& coverPath);
    static QString getCollection (const QString& fb2Filename);

protected:
    static bool getAuthorMetaData       (const QByteArray& data, QString& author);
    static bool getTitleMetaData        (const QByteArray& data, QString& title);
    static bool getDescriptionMetaData  (const QByteArray& data, QString& description);
    static bool getDateMetaData         (const QByteArray& data, QDateTime& date);
    static bool getPublisherMetaData    (const QByteArray& data, QString& publisher);
    static bool getCoverFileNameMetaData(const QByteArray& data, QString& coverFileName);
    static bool getLanguageMetaData     (const QByteArray& data, QString& language);

    static QString formatAuthor(QString& firstName, QString& middleName, QString& lastName);
};

#endif // FB2METADATAEXTRACTOR_H
