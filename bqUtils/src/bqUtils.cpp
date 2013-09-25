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

#include "bqUtils.h"

#include <QString>
#include <QTextDocument>
#include <QDebug>

QString bqUtils::truncateStringToLength(const QString& toTruncate, int length, bool wholeWords, const QString& appendStr)
{
    QString truncated = toTruncate.simplified();

    if(truncated.length() <= length)
        return truncated;

    truncated = truncated.left(length - appendStr.length() +1);

    int lastSpace = truncated.lastIndexOf(" ");

    if(lastSpace == truncated.length() || lastSpace <= length/2)
        wholeWords = false;

    if(wholeWords) // Don't let split words
        truncated = truncated.left(truncated.lastIndexOf(" "));

    truncated.append(appendStr);

    return truncated;
}

QString bqUtils::specialCharactersFromUtf8(const QString& text)
{
    QString utf8String = text;
    utf8String.replace(QString::fromUtf8("\u2026"), "...");
    utf8String.replace(QString::fromUtf8("\u2010"), "-");
    utf8String.replace(QString::fromUtf8("\u2011"), "-");
    utf8String.replace(QString::fromUtf8("\u2012"), "-");
    utf8String.replace(QString::fromUtf8("\u2013"), "-");
    utf8String.replace(QString::fromUtf8("\u2014"), "-");
    utf8String.replace(QString::fromUtf8("\u2015"), "-");
    utf8String.replace(QString::fromUtf8("\u2016"), "-");
    utf8String.replace(QString::fromUtf8("\u2017"), "-");
    utf8String.replace(QString::fromUtf8("\u23BA"), "-");
    utf8String.replace(QString::fromUtf8("\u23BD"), "-");
    utf8String.replace(QString::fromUtf8("\u23BB"), "-");
    utf8String.replace(QString::fromUtf8("\u23BC"), "-");

    return utf8String;
}

QString bqUtils::simplify( const QString& str )
{
    QString result = str.toLower();

    // A
    result.replace(QChar(192), "A");
    result.replace(QChar(192), "A");
    result.replace(QChar(194), "A");
    result.replace(QChar(195), "A");
    result.replace(QChar(196), "A");
    result.replace(QChar(197), "A");
    result.replace(QChar(198), "A");
    result.replace(QChar(224), "a");
    result.replace(QChar(225), "a");
    result.replace(QChar(226), "a");
    result.replace(QChar(227), "a");
    result.replace(QChar(228), "a");
    result.replace(QChar(229), "a");
    result.replace(QChar(230), "a");

    // E
    result.replace(QChar(200), "E");
    result.replace(QChar(201), "E");
    result.replace(QChar(202), "E");
    result.replace(QChar(203), "E");
    result.replace(QChar(232), "e");
    result.replace(QChar(233), "e");
    result.replace(QChar(234), "e");
    result.replace(QChar(235), "e");

    // I
    result.replace(QChar(204), "I");
    result.replace(QChar(205), "I");
    result.replace(QChar(206), "I");
    result.replace(QChar(207), "I");
    result.replace(QChar(236), "i");
    result.replace(QChar(237), "i");
    result.replace(QChar(238), "i");
    result.replace(QChar(239), "i");

    // O
    result.replace(QChar(210), "O");
    result.replace(QChar(211), "O");
    result.replace(QChar(212), "O");
    result.replace(QChar(213), "O");
    result.replace(QChar(214), "O");
    result.replace(QChar(215), "O");
    result.replace(QChar(242), "o");
    result.replace(QChar(243), "o");
    result.replace(QChar(244), "o");
    result.replace(QChar(245), "o");
    result.replace(QChar(246), "o");
    result.replace(QChar(248), "o");

    // U
    result.replace(QChar(217), "U");
    result.replace(QChar(218), "U");
    result.replace(QChar(219), "U");
    result.replace(QChar(220), "U");
    result.replace(QChar(249), "u");
    result.replace(QChar(250), "u");
    result.replace(QChar(251), "u");
    result.replace(QChar(252), "u");

    return result;
}
