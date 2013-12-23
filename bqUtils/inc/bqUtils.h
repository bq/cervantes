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

#ifndef BQUTILS_H
#define BQUTILS_H

#include <QString>
#include <QFont>

class bqQTextBrowser;
class bqUtils {

public:
    static QString      truncateStringToLength              ( const QString& toTruncate, int nChars, bool wholeWords = true, const QString& appendStr = QString("..."));
    static QString      truncateStringToWidth               ( const QString& toTruncate,const int nPixels,const QFont& font );
    static QString      specialCharactersFromUtf8           ( const QString& text );
    static QString      simplify                            ( const QString& str );
    static bool         removeDir                           ( const QString& dirName );
    static bool         copyDir                             ( const QString &src, const QString &dest );
    static bool         filesToCopy                         ( const QString& path, int& booksToLoad );
};

#endif // BQUTILS_H
