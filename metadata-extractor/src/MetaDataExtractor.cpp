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

#include <QString>
#include <QDebug>
#include <QFileInfo>
#include <QStringList>

#include <MetaDataExtractor.h>
#include <PdfMetaDataExtractor.h>
#include <EpubMetaDataExtractor.h>
#include <Fb2MetaDataExtractor.h>
#include <MobiMetaDataExtractor.h>

#define FILE_MAX_SIZE 31457280 // 30 MB

bool MetaDataExtractor::extractCover(const QString& filename, QString& coverPath)
{
    QFileInfo info = QFileInfo(filename);

    if (!info.isReadable())
    {
        qDebug() << Q_FUNC_INFO << "Cannot read " << filename;
        return false;
    }

         if (info.suffix().toLower() == "epub") return EpubMetaDataExtractor::extractCover(filename, coverPath);
    else if (info.suffix().toLower() == "fb2")  return Fb2MetaDataExtractor::extractCover(filename, coverPath);
    else if (info.suffix().toLower() == "mobi")  return MobiMetaDataExtractor::extractCover(filename, coverPath);
    else return false;
}

bool MetaDataExtractor::getMetaData( const QString& filename, QString& title, QString& author, QString& publisher, QDateTime& date, QString& description, QString& format, bool& isDRMBook, QString& collection, QString& language)
{
    QFileInfo info = QFileInfo(filename);
    bool result = false;

	if (!info.isReadable()) {
        qDebug() << Q_FUNC_INFO << "Cannot read " << filename;
		return false;
	}

    format = info.suffix().toLower();

    if (format == "pdf")
    {
        result = PdfMetaDataExtractor::getMetaData(filename, title, author, publisher, date, description, language);
    }
    else if (info.suffix().toLower() == "epub")
    {
        collection = EpubMetaDataExtractor::getCollection(filename);
        result = EpubMetaDataExtractor::getMetaData(filename, title, author, publisher, date, description, isDRMBook, language);
    }
    else if (info.suffix().toLower() == "fb2" && info.size() < FILE_MAX_SIZE)
    {
        collection = Fb2MetaDataExtractor::getCollection(filename);
        result = Fb2MetaDataExtractor::getMetaData(filename, title, author, publisher, date, description, isDRMBook, language);
    }
    else if (  info.suffix().toLower() == "mobi" && info.size() < FILE_MAX_SIZE)
    {
        result = MobiMetaDataExtractor::getMetaData(filename, title, author, publisher, date, description, isDRMBook, language);
    }
    else if (  info.suffix().toLower() == "doc"
            || info.suffix().toLower() == "chm"
            || info.suffix().toLower() == "txt"
            || info.suffix().toLower() == "rtf"
            || info.suffix().toLower() == "html")
    {
        title = QFileInfo(filename).baseName();
        result = true;
    }

    if(language.isEmpty())
        language = UNDEFINED_LANGUAGE;

    return result;
}

QStringList MetaDataExtractor::extractCSS(const QString& filename)
{
    QFileInfo info = QFileInfo(filename);
    if (!info.isReadable())
    {
        qDebug() << Q_FUNC_INFO << "Cannot read" << filename;
    }

    if (info.suffix () == "epub")
    {
        return EpubMetaDataExtractor::extractCSS(filename);
    }
    return QStringList();
}

QString MetaDataExtractor::getLanguage(const QString &filename)
{
    qDebug() << Q_FUNC_INFO << filename;

    QFileInfo info = QFileInfo(filename);
    QString format = info.suffix().toLower();
    QString language;

    // eBook language only applicable to some formats
    if (format == "epub" ||
        format == "fb2"  ||
        format == "mobi" )
    {
        QString dummy;
        QDateTime dummyDate;
        bool dummyBool;
        getMetaData(filename,dummy,dummy,dummy,dummyDate,dummy,dummy,dummyBool,dummy,language);

        if(language.isEmpty())
            language = UNDEFINED_LANGUAGE;
    }
    // format not supporting language metadata
    else
    {
        language = UNDEFINED_LANGUAGE;
    }

    qDebug() << Q_FUNC_INFO << language;

    return language;
}

double MetaDataExtractor::getCollectionIndex(const QString& filename)
{
    qDebug() << Q_FUNC_INFO;

    QFileInfo info = QFileInfo(filename);

    if (!info.isReadable()) {
        qDebug() << Q_FUNC_INFO << "Cannot read " << filename;
        return false;
    }

    QString format = info.suffix().toLower();

    if (format == "epub")
        return EpubMetaDataExtractor::getCollectionIndex(filename);
    else if (format == "fb2")
        return Fb2MetaDataExtractor::getCollectionIndex(filename);
}
