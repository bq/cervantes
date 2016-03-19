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

#include "bqUtils.h"

#include <QString>
#include <QTextDocument>
#include <QDebug>
#include <QDir>
#include <QFontMetrics>

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

QString bqUtils::truncateStringToWidth(const QString& toTruncate,const int nPixels,const QFont& font)
{
    QFontMetrics fontMetrics(font);
    return fontMetrics.elidedText(toTruncate, Qt::ElideRight, nPixels);
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

bool bqUtils::removeDir(const QString& dirName)
{
    qDebug() << Q_FUNC_INFO << " dirName: " << dirName;
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName))
    {
        qDebug() << Q_FUNC_INFO << "1";
        foreach(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
        {
            qDebug() << Q_FUNC_INFO << "2: " << info.absoluteFilePath();
            if (info.isDir())
            {
                qDebug() << Q_FUNC_INFO << "3: " << info.absoluteFilePath();
                result = removeDir(info.absoluteFilePath());
            }else
                result = QFile::remove(info.absoluteFilePath());

            if (!result)
            {
                qDebug() << Q_FUNC_INFO << "4";
                return result;
            }
        }
        qDebug() << Q_FUNC_INFO << "5: " << dirName;
        result = QDir().rmdir(dirName);
        qDebug() << Q_FUNC_INFO << "result: " << result;
    }

    return result;
}

bool bqUtils::copyDir(const QString &srcPath, const QString &dstPath)
{
    qDebug() << Q_FUNC_INFO;
    QDir parentDstDir(QFileInfo(dstPath).path());
    if (parentDstDir.exists(QFileInfo(dstPath).path()) && !parentDstDir.mkdir(QFileInfo(dstPath).fileName()))
        return false;

    QDir srcDir(srcPath);
    foreach(const QFileInfo &info, srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
    {
        QString srcItemPath = srcPath + "/" + info.fileName();
        QString dstItemPath = dstPath + "/" + info.fileName();
        if (info.isDir())
        {
            if (info.exists() && !copyDir(srcItemPath, dstItemPath))
                return false;
        }
        else if (info.isFile())
        {
            if (!QFile::exists(dstItemPath) && !QFile::copy(srcItemPath, dstItemPath))
                return false;
        }
        else
            qDebug() << "Unhandled item" << info.filePath() << "in cpDir";
    }

    return true;
}

bool bqUtils::filesToCopy( const QString& path, int& booksToLoad )
{
    qDebug() << Q_FUNC_INFO;
    QDir dir(path);
    if(!dir.exists())
        return false;

    dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files | QDir::Readable);
    dir.setSorting(QDir::DirsFirst | QDir::Name);

    QStringList sub_list;
    QStringList list = dir.entryList();

    QStringList::const_iterator i = list.begin();
    QStringList::const_iterator itEnd = list.end();

    QFileInfo fi;
    while(i != itEnd)
    {
        QString file_path = dir.filePath(*i);
        fi.setFile(file_path);

        if(!fi.exists())
            return false;

        const QString& name = fi.fileName();

        if (!name.startsWith(".") && fi.isDir()) {
            sub_list += file_path;
            ++i;
            continue;
        }

        if (name.startsWith(".")) {
            ++i;
            continue;
        }

        ++booksToLoad;
        ++i;
    }

    QStringList::const_iterator j = sub_list.begin();
    while(j != sub_list.end())
    {
        if(!filesToCopy(*j, booksToLoad))
            return false;
        ++j;
    }

    qDebug() << Q_FUNC_INFO << "booksToLoad: " << booksToLoad;
    return true;
}
