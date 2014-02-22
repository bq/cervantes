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

#ifndef MOBIMETADATAEXTRACTOR_H
#define MOBIMETADATAEXTRACTOR_H

#include <QString>
#include "pdbfmt.h"
#include "lvstream.h"
#include <QDateTime>

class MobiMetaDataExtractor {

public:
    static bool     getMetaData  (const QString& filename, QString& title, QString& author, QString& publisher, QDateTime& date, QString& description, bool& isDRMBook, QString& language);
    static bool     extractCover ( const QString& filename, const QString &coverPath);
    static int      getRecordOffset(LVStreamRef stream, int recordNumber);
    static QString  getRecordInfo(LVStreamRef stream, int offset);
    static int      getIntFromArray(LVStreamRef stream, int length);
    static bool     uncompressStream(LVStreamRef stream, int &offset, int &dataOffset, int &recordsNum);
};

#endif // MOBIMETADATAEXTRACTOR_H
