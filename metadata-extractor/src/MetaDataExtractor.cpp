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

#include <QString>
#include <QDebug>
#include <QFileInfo>
#include <QStringList>

#include <MetaDataExtractor.h>
#include <PdfMetaDataExtractor.h>
#include <EpubMetaDataExtractor.h>
#include <Fb2MetaDataExtractor.h>
#include <MobiMetaDataExtractor.h>


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

bool MetaDataExtractor::getMetaData( const QString& filename, QString& title, QString& author, QString& publisher, QDateTime& date, QString& description, QString& format, bool& isDRMBook)
{
    QFileInfo info = QFileInfo(filename);

	if (!info.isReadable()) {
        qDebug() << Q_FUNC_INFO << "Cannot read " << filename;
		return false;
	}

    format = info.suffix().toLower();

    if (format == "pdf")
    {
        return PdfMetaDataExtractor::getMetaData(filename, title, author, publisher, date, description);
    }
    else if (info.suffix().toLower() == "epub")
    {
        return EpubMetaDataExtractor::getMetaData(filename, title, author, publisher, date, description, isDRMBook);
    }
    else if (info.suffix().toLower() == "fb2")
    {
        return Fb2MetaDataExtractor::getMetaData(filename, title, author, publisher, date, description, isDRMBook);
    }
    else if (  info.suffix().toLower() == "mobi")
        return MobiMetaDataExtractor::getMetaData(filename, title, author, publisher, date, description, isDRMBook);
    else if (  info.suffix().toLower() == "doc"
            || info.suffix().toLower() == "chm"
            || info.suffix().toLower() == "txt"
            || info.suffix().toLower() == "rtf"
            || info.suffix().toLower() == "html")
    {
        title = QFileInfo(filename).baseName();
        return true;
    }

	return false;
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
