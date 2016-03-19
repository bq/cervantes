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

#include "Fb2MetaDataExtractor.h"

#include <QFile>
#include <QDebug>
#include <QXmlStreamReader>
#include <QImage>
#include <QImageReader>


#define BYTES_TO_READ       10240

bool isImage(const QString& path);

bool Fb2MetaDataExtractor::getMetaData(MetaData& /*data*/)
{
    // TODO: use struct instead of multiple variables.
}

QString Fb2MetaDataExtractor::formatAuthor(QString& firstName, QString& middleName, QString& lastName)
{
    QString author = "";

    if (!firstName.isEmpty())
    {
        author = firstName;
        if (!lastName.isEmpty() || !middleName.isEmpty()) author += " ";
    }

    if (!middleName.isEmpty())
    {
        author += middleName;
        if (!lastName.isEmpty()) author += " ";
    }

    if (!lastName.isEmpty())
    {
        author += lastName;
    }

    author = author.simplified();
    qDebug() << Q_FUNC_INFO << author;
    return author;
}

bool Fb2MetaDataExtractor::getAuthorMetaData(const QByteArray& data, QString& author)
{
    QXmlStreamReader streamData(data);
    QString firstName = "", middleName = "", lastName = "";
    while (!streamData.atEnd())
    {
        if (streamData.readNextStartElement() && streamData.name() == "title-info")
        {
            while (!streamData.atEnd())
            {
                if ((streamData.readNext() == QXmlStreamReader::StartElement  && streamData.name() == "author") &&
                     streamData.readNextStartElement())
                {
                    bool isStartElement = true;
                    while (!streamData.atEnd())
                    {
                        if (isStartElement && streamData.name() == "first-name" && streamData.readNext() == QXmlStreamReader::Characters)
                        {
                            firstName = streamData.text().toString();
                            streamData.skipCurrentElement();
                            isStartElement = streamData.readNextStartElement();
                        }
                        else if (isStartElement && streamData.name() == "middle-name" && streamData.readNext() == QXmlStreamReader::Characters)
                        {
                            middleName = streamData.text().toString();
                            streamData.skipCurrentElement();
                            isStartElement = streamData.readNextStartElement();
                        }
                        else if (isStartElement && streamData.name() == "last-name" && streamData.readNext() == QXmlStreamReader::Characters)
                        {
                            lastName = streamData.text().toString();
                        }
                        else break;
                    } // End WHILE
                    break;
                }
                else if (streamData.isEndElement() && streamData.name() == "document-info") break; // Ensures author node withim title-info node.
            } // End WHILE
            break;
        }
        else if (streamData.name() == "document-info" || // Ensures author node withim title-info node.
                 streamData.name() == "body") break; //  Ensures break before the end.
    } // End WHILE

    if (streamData.hasError())
    {
        qDebug() << Q_FUNC_INFO << streamData.error();
        // TODO: do error handling
    }

    author = formatAuthor(firstName, middleName, lastName);

    if(author.isEmpty()) return false;
    return true;
}

bool Fb2MetaDataExtractor::getTitleMetaData(const QByteArray& data, QString& title)
{
    QXmlStreamReader streamData(data);
    while (!streamData.atEnd())
    {
        if (streamData.readNextStartElement() && streamData.name() == "book-title")
        {
            if (streamData.readNext() == QXmlStreamReader::Characters)
            {
                title = streamData.text().toString().simplified();
                return true;
            }
            else return false;
        }
        else if (streamData.name() == "body") return false; //  Ensures break before the end.
    }
    return false;
}

bool Fb2MetaDataExtractor::getDescriptionMetaData(const QByteArray& data, QString& description)
{
    QXmlStreamReader streamData(data);
    while (!streamData.atEnd())
    {
        if (streamData.readNextStartElement() && streamData.name() == "annotation")
        {
            bool several = false;
            // The description can be made for several "p" tags.
            while (!streamData.atEnd() &&
                    streamData.readNextStartElement() &&
                    streamData.name() == "p")
            {
                if (streamData.readNext() == QXmlStreamReader::Characters)
                {
                    if (several) description += '\n';
                    description += streamData.text().toString();
                    streamData.skipCurrentElement();
                    several = true;
                }
            }
        }
        else if (streamData.name() == "body") break; //  Ensures break before the end.
    }

    if (streamData.hasError())
    {
        qDebug() << Q_FUNC_INFO << streamData.error();
        // TODO: do error handling
    }

    qDebug() << Q_FUNC_INFO << description;

    if(description.isEmpty()) return false;
    return true;
}

bool Fb2MetaDataExtractor::getDateMetaData(const QByteArray& /*data*/, QDateTime& /*date*/)
{
    /** SUMMARY EXTRACT FROM fb2.xsd DOCUMENT
    2<tittle-info>
    ...
      3<annotation>
      ...
       [4<poem>
          5<date>]
          ...
     [3<keywords>]
      ...
      3<date> (TARGET DATE)
      ...
    2<document-info>
    ...
      3<date>
      ...
      */

    /*QXmlStreamReader streamData(data);
    while (!streamData.atEnd())
    {
        if (streamData.readNextStartElement() && streamData.name() == "date")
        {
        }
        else if (streamData.name() == "body") return false; //  Ensures break before the end.
    }

    if (streamData.hasError())
    {
        qDebug() << Q_FUNC_INFO << streamData.error();
        // TODO: do error handling
    }

    */

    return false;
}

bool Fb2MetaDataExtractor::getPublisherMetaData(const QByteArray& data, QString& publisher)
{
    QXmlStreamReader streamData(data);
    while (!streamData.atEnd())
    {
        if (streamData.readNextStartElement() && streamData.name() == "publisher")
        {
            if (streamData.readNext() == QXmlStreamReader::Characters)
            {
               publisher = streamData.text().toString();
               return true;
            }
            else return false;
        }
        else if (streamData.name() == "body") break; //  Ensures break before the end.
    }

    if (streamData.hasError())
    {
        qDebug() << Q_FUNC_INFO << streamData.error();
        // TODO: do error handling
    }

    return false;
}

bool Fb2MetaDataExtractor::getMetaData(const QString& fb2Filename, QString& title, QString& author, QString& publisher, QDateTime& date, QString& description, bool& /*isDRMBook*/, QString& language)
{

    QFile file(fb2Filename);
    if (!file.open(QIODevice::ReadOnly) || file.size() > TXT_MAX_FILE_SIZE) return false;
    QByteArray data = file.read(BYTES_TO_READ);
    file.close();
    if (data.isEmpty()) return false;

    // In order of appearance in FB2 xsd:
    // author, title, description, date, publisher
    /** SUMMARY EXTRACT FROM fb2.xsd DOCUMENT
    ...
    0<FictionBook>
      ...
      1<description>
        2<title-info>
        ...
          3<author> (AUTHOR)
            4<first-name>
            4<middle-name>
            4<last-name>
            ...
          3<book-title> (TITTLE)
          3<annotation>
            4<p> (DESCRIPTION)
            [4<p>]
            ...
          3<date> (DATE)
          3<coverpage>
            4<image xlink:href | l:href = "#fileName"> (***COVER***)
          ...
        2<publish-info>
        ...
          3<publisher> (PUBLISHER)
          ...
      1<body>
    */

    // Searching AUTHOR
    getAuthorMetaData(data, author);
    // Searching TITLE
    getTitleMetaData(data, title);
    // Searching DESCRIPTION
    getDescriptionMetaData(data, description);
    // Searching DATE
    getDateMetaData(data, date);
    // Searching PUBLISHER
    getPublisherMetaData(data, publisher);
    // Searching LANGUAGE
    getLanguageMetaData(data, language);

    // Accept if some value is not null.
    return (!author.isEmpty()      ||
            !title.isEmpty()       ||
            !description.isEmpty() ||
            !date.isNull()         ||
            !publisher.isEmpty()   ||
            !language.isEmpty());
}

bool Fb2MetaDataExtractor::getCoverFileNameMetaData(const QByteArray& data, QString& coverFileName)
{
    QString nodeAttr1Name   = "xlink:href",
            nodeAttr2Name   = "l:href",
            nodeAttrStart   = "#";

    QXmlStreamReader streamData(data);

    // Searching cover name.
    while (!streamData.atEnd())
    {
        if (streamData.readNextStartElement() && streamData.name() == "coverpage") // Begin of a target node.
        {
            while (!streamData.atEnd())
            {
                if (streamData.readNext() == QXmlStreamReader::StartElement && streamData.name() == "image") // Begin of a target node .
                {
                    QXmlStreamAttributes nodeAttributes = streamData.attributes();

                    if (nodeAttributes.hasAttribute(nodeAttr1Name) &&
                        nodeAttributes.value(nodeAttr1Name).toString().startsWith(nodeAttrStart))
                    {
                        coverFileName = nodeAttributes.value(nodeAttr1Name).toString();
                        coverFileName = coverFileName.right(coverFileName.size() - 1); // Delete first character of the ref '#'
                        break;
                    }
                    else if (nodeAttributes.hasAttribute(nodeAttr2Name) &&
                             nodeAttributes.value(nodeAttr2Name).toString().startsWith(nodeAttrStart))
                         {
                             coverFileName = nodeAttributes.value(nodeAttr2Name).toString();
                             coverFileName = coverFileName.right(coverFileName.size() - 1); // Delete first character of the ref '#'
                             break;
                         }
                }
                else if (streamData.isEndElement() && streamData.name() == "coverpage") break; // Ensures image node withim coverpage node.
            }// end While

            if (!coverFileName.isEmpty()) break;
        }
        else if (streamData.name() == "body") break; //  Ensures break before the end.
    }// end While


    if (streamData.hasError())
    {
        qDebug() << Q_FUNC_INFO << streamData.error();
        // TODO: do error handling
    }

    if (coverFileName.isEmpty()) return false;
    return true;
}

bool Fb2MetaDataExtractor::extractCover(const QString& fb2Filename, const QString& coverPath)
{
    QFile file(fb2Filename);
    if (!file.open(QIODevice::ReadOnly) || file.size() > TXT_MAX_FILE_SIZE) return false;
    QByteArray data = file.readAll();
    file.close();
    if (data.isEmpty()) return false;
    QXmlStreamReader streamData(data);

    /** EXTRACT FROM fb2.xsd DOCUMENT
    ...
    <xs:element name="description">
      2<title-info>
      ...
        3<coverpage>
          4<image xlink:href | l:href = "#fileName"> (***COVER***)
    ...
    <xs:element name="binary" minOccurs="0" maxOccurs="unbounded">
      <xs:annotation>
        <xs:documentation>
          Any binary data that is required for the presentation of this book in base64 format. Currently only images are used.
        </xs:documentation>
      </xs:annotation>
      <xs:complexType>
        <xs:simpleContent>
          <xs:extension base="xs:base64Binary">
            <xs:attribute name="content-type" type="xs:string" use="required"/>
            <xs:attribute name="id" type="xs:ID" use="required"/>
          </xs:extension>
        </xs:simpleContent>
      </xs:complexType>
    </xs:element>
    ...
    */

    QString coverFileName = "";
    if (!getCoverFileNameMetaData(data, coverFileName)) return false;

    QString nodeName        = "binary",
            nodeAttr1Name   = "content-type",
            nodeAttr2Name   = "id",
            nodeAttrStart  = "image/";

    // Search and create cover image.
    while (!streamData.atEnd())
    {
        if (streamData.readNext() == QXmlStreamReader::StartElement && streamData.name() == nodeName) // Begin of a target node .
        {
            QXmlStreamAttributes nodeAttributes = streamData.attributes();

            if (nodeAttributes.hasAttribute(nodeAttr1Name) &&
                nodeAttributes.hasAttribute(nodeAttr2Name) &&
                nodeAttributes.value(nodeAttr1Name).toString().startsWith(nodeAttrStart, Qt::CaseInsensitive) &&
                nodeAttributes.value(nodeAttr2Name).toString() == coverFileName &&
                streamData.readNext() == QXmlStreamReader::Characters)
            {
                QByteArray coverArray = QByteArray::fromBase64(streamData.text().toUtf8());
                QImage cover = QImage::fromData(coverArray);
                if (cover.isNull()) continue;
                QString sFormat = nodeAttributes.value(nodeAttr1Name).toString().mid(nodeAttrStart.size());
                if (isImage(sFormat) && cover.save(coverPath,"jpg")) return true;
            }
        }
    } // end While

    if (streamData.hasError())
    {
        qDebug() << Q_FUNC_INFO << streamData.error();
        // TODO: do error handling
    }

    return false;
}

bool isImage(const QString& path)
{
    if(path.isEmpty()) return false;

    QList<QByteArray> supportedImageslist = QImageReader::supportedImageFormats();
    QString fileName = path.split("/").last();
    QString fileExtension = fileName.split(".").last();
    return supportedImageslist.contains(fileExtension.toLower().toAscii()) && fileExtension.toLower() != "mng";
}

QString Fb2MetaDataExtractor::getCollection(const QString& fb2Filename)
{
    QString nodeAttr1Name   = "name";

    QFile file(fb2Filename);
    if (!file.open(QIODevice::ReadOnly)) return false;
    QByteArray data = file.readAll();
    file.close();
    if (data.isEmpty()) return false;
    QXmlStreamReader streamData(data);
    QString collection;

    // Searching cover name.
    while (!streamData.atEnd())
    {
                if (streamData.readNext() == QXmlStreamReader::StartElement && streamData.name() == "sequence") // Begin of a target node .
                {
                    QXmlStreamAttributes nodeAttributes = streamData.attributes();

                    if (nodeAttributes.hasAttribute(nodeAttr1Name))
                    {
                        collection = QString::fromUtf8(nodeAttributes.value(nodeAttr1Name).toUtf8());
                        break;
                    }
                }
                else if (streamData.isEndElement() && streamData.name() == "sequence") break; // Ensures image node withim coverpage node.

            if (!collection.isEmpty()) break;
        else if (streamData.name() == "body") break; //  Ensures break before the end.
    }// end While


    if (streamData.hasError())
    {
        qDebug() << Q_FUNC_INFO << streamData.error();
        // TODO: do error handling
    }

    if (collection.isEmpty()) return "";
    return collection;
}

bool Fb2MetaDataExtractor::getLanguageMetaData(const QByteArray& data, QString& language)
{
    QXmlStreamReader streamData(data);
    while (!streamData.atEnd())
    {
        if (streamData.readNextStartElement() && streamData.name() == "lang")
        {
            if (streamData.readNext() == QXmlStreamReader::Characters)
            {
                language = streamData.text().toString().simplified();
                return true;
            }
            else return false;
        }
        else if (streamData.name() == "body") return false; //  Ensures break before the end.
    }
    return false;
}

double Fb2MetaDataExtractor::getCollectionIndex(const QString& fb2Filename)
{
    QString nodeAttr1Name   = "number";

    QFile file(fb2Filename);
    if (!file.open(QIODevice::ReadOnly)) return 0;
    QByteArray data = file.readAll();
    file.close();
    if (data.isEmpty()) return 0;
    QXmlStreamReader streamData(data);
    QString collectionIndex;

    // Searching cover name.
    while (!streamData.atEnd())
    {
                if (streamData.readNext() == QXmlStreamReader::StartElement && streamData.name() == "sequence") // Begin of a target node .
                {
                    QXmlStreamAttributes nodeAttributes = streamData.attributes();

                    if (nodeAttributes.hasAttribute(nodeAttr1Name))
                    {
                        collectionIndex = nodeAttributes.value(nodeAttr1Name).toString();
                        break;
                    }
                }
                else if (streamData.isEndElement() && streamData.name() == "sequence") break; // Ensures image node withim coverpage node.

            if (!collectionIndex.isEmpty()) break;
        else if (streamData.name() == "body") break; //  Ensures break before the end.
    }// end While


    if (streamData.hasError())
    {
        qDebug() << Q_FUNC_INFO << streamData.error();
        // TODO: do error handling
    }

    return collectionIndex.toDouble();
}
