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

#include "ModelBackendOneFile.h"

#define BOOK_DATA_FILE      ".data.xml"
const char* TRANSACTIONS_DATA_FILE = ".transactions.xml";
#define BOOKS_LOADED_TO_SHOW_DIALOG 20

#include "Model.h"
#include "MetaDataExtractor.h"
#include "BookInfo.h"
#include "BookLocation.h"
#include "bqUtils.h"

#include <unistd.h>
#include <QDir>
#include <QStringList>
#include <QDebug>
#include <QCoreApplication>


ModelBackendOneFile::ModelBackendOneFile( const QString& privatePartition ) : IModelBackend(), m_books(NULL)
{
    qDebug() << Q_FUNC_INFO << "Partition: " << privatePartition;
    _privatePartition = privatePartition;
    m_transactions = new QHash<QString, bool>();
    m_collections = new QStringList();
}

ModelBackendOneFile::~ModelBackendOneFile()
{
    if(m_books)
    {
        TBooks::iterator it = m_books->begin();
        TBooks::iterator itEnd = m_books->end();
        for(; it != itEnd; ++it)
        {
            delete *it;
            *it = NULL;
        }
        m_books->clear();
    }

    TBooks::iterator it = m_infos.begin();
    TBooks::iterator itEnd = m_infos.end();
    for(; it != itEnd; ++it)
    {
        delete *it;
        *it = NULL;
    }
    m_infos.clear();
}

void ModelBackendOneFile::init (TBooks* books)
{
    m_books = books;
}

void ModelBackendOneFile::addDir ( const QString& dirPath )
{
    // IMPORTANT: Keep in mind that the dirPath is not guaranteed to exist during the whole function (i.e a SD card may be extracted)
    QString path(QDir::cleanPath(dirPath));// Copy of the reference dirPath

    m_pathsMount.insert(path, false);

    // Load path files MetaInfo
    bool bRet = loadMetaInfo(path);
    if(!bRet)
    {
        qWarning() << Q_FUNC_INFO << "Error loading metaInfo, exiting";
        removeDir(path);// TODO: Do partial remove to make it faster
        return;
    }

    // Load path dir looking for new files
    QStringList booksPaths;
    bRet = booksToLoad(path, booksPaths);
    if(!bRet)
    {
        qDebug() << Q_FUNC_INFO << "Error loading books, Exiting";
        removeDir(path);// TODO: Do partial remove to make it faster
        return;
    }

    qDebug() << Q_FUNC_INFO << "Books to load: " << booksPaths;

    int totalBooks = booksPaths.size();
    if(path != _privatePartition)
    {
        if(totalBooks > BOOKS_LOADED_TO_SHOW_DIALOG) // For 20 books or more it shows progress dialog
            emit loadingBooks(totalBooks);
    }

    bRet = load(booksPaths);
    if(!bRet)
    {
        qDebug() << Q_FUNC_INFO << "Error. Exiting";
        removeDir(path);// TODO: Do partial remove to make it faster
        emit loadFinished();
        return;
    }

    qDebug() << Q_FUNC_INFO << "Before load finished";

    emit loadFinished();

    qDebug() << Q_FUNC_INFO << "After load finished";

    qDebug() << Q_FUNC_INFO << "dir path: " << path << "_privatePartition: " << _privatePartition;
    if(QDir::cleanPath(path) == QDir::cleanPath(_privatePartition))
        loadArchivedBooks();

    messDir(path, UPDATE_FULL);
}

void ModelBackendOneFile::removeDir ( const QString& dirPath )
{
    // IMPORTANT: Keep in mind that the dirPath is not guaranteed to exist during the whole function (i.e a SD card may be extracted)
    QString path(QDir::cleanPath(dirPath));// Copy of the reference dirPath

    m_pathsMount.remove(path);

    TBooks::iterator i = m_books->begin();
    while (i != m_books->end())
    {
        if (i.key().contains(path))
        {
            delete *i;
            i = m_books->erase(i);
        }
        else
            ++i;
    }

    TBooks::iterator j = m_infos.begin();
    while (j != m_infos.end())
    {
        if (j.key().contains(path))
        {
            delete *j;
            j = m_infos.erase(j);

        }
        else
            ++j;
    }

    emit modelChanged(path, UPDATE_FULL);
}

void ModelBackendOneFile::sync( const QString& path, qint64 serverTimestamp )
{
    qDebug() << Q_FUNC_INFO << "Path: " << path;

    QHash<QString, bool>::iterator i = m_pathsMount.begin();
    while(i != m_pathsMount.end())
    {
        if(i.value() && (path.isEmpty() || i.key() == path))
        {
            save(i.key(), serverTimestamp);
            m_pathsMount.insert(i.key(), false);
        }else
            ++i;
    }
	::sync();
}

bool ModelBackendOneFile::load( const QStringList& booksPaths )
{
    qDebug() << Q_FUNC_INFO;

    QStringList::const_iterator it = booksPaths.begin();
    QStringList::const_iterator itEnd = booksPaths.end();

    int progressBar = 0;
    float booksLoaded = 0;
    const int totalBooks = booksPaths.size();
    const float totalBooksPercentage = (totalBooks) ? 100.0 / totalBooks : 0;
    QFileInfo fi;
    while(it != itEnd)
    {
        fi.setFile(*it);
        if(!fi.exists())
        {
            return false;
        }

        BookInfo* book;
        if(!m_infos.contains(*it) || m_fileModelVersion != MODEL_VERSION)
        {
            if((*it).contains("private"))
            {
                QFile file(*it);
                file.remove();
                ++it;
                continue;
            }
            else
            {
                book = loadDefaultInfo(*it);
                if(totalBooks > 0)
                {
                    booksLoaded++;
                    qDebug() << Q_FUNC_INFO << booksLoaded * totalBooksPercentage;
                    progressBar = int( booksLoaded * totalBooksPercentage );
                    emit downloadProgress(progressBar);
                }
            }
        }
        else
            book = new BookInfo(*m_infos.value(*it));

        m_books->insert(book->path, book);
        ++it;
    }

    return true;
}

void ModelBackendOneFile::loadArchivedBooks()
{
    qDebug() << Q_FUNC_INFO;
    TBooks::const_iterator it = m_infos.constBegin();
    TBooks::const_iterator itEnd = m_infos.constEnd();
    for(; it != itEnd; ++it)
    {
        BookInfo* bookInfo = it.value();
        if(m_books->find(bookInfo->path) == m_books->end())
        {
            if(!bookInfo->m_archived  && bookInfo->m_type != BookInfo::BOOKINFO_TYPE_SUBSCRIPTION)
            {
                bookInfo->m_archived = true;
                qDebug()<< "Non archived book not found in the SD: " << bookInfo->path;
            }
            m_books->insert(bookInfo->path, new BookInfo(*bookInfo));
            qDebug()<< "Archived book " << bookInfo->path << " load in memory";
        }
    }
}

bool ModelBackendOneFile::loadMetaInfo( const QString& path )
{
    qDebug() << Q_FUNC_INFO;

    QFile file(path + QDir::separator() + BOOK_DATA_FILE);
    if (!file.open(QFile::ReadOnly))
    {
        if (!file.open(QFile::ReadOnly))
        {
            qDebug() << Q_FUNC_INFO << "Cannot open newly created model file";
            return true;
		}
    }

    QXmlStreamReader xml(&file);

    xml.readNext(); //Skip first tag
    xml.readNext();

    if(xml.hasError()){
        qWarning() << Q_FUNC_INFO << "XML error before loading";
        file.copy(file.fileName() + ".backup");
        file.remove();
        return true;
    }

    if(xml.name() == "books")
    {
        m_fileModelVersion = xml.attributes().value("model_version").toString().toUInt();
        qDebug() << Q_FUNC_INFO << "model version" << m_fileModelVersion;

        while (!xml.atEnd() && !xml.hasError())
        {
            xml.readNext();
            if(xml.name() == "collections")
            {
                while (!xml.atEnd() && !xml.hasError())
                {
                    xml.readNext();
                    if(xml.isEndElement())
                    {
                        if(xml.name() == "collections")
                            break;
                    }
                    if (xml.name() == "collection" && xml.isStartElement())
                    {
                        addCollection(xml.readElementText());
                    }
                }
            }

            if (xml.name() == "book" && xml.isStartElement())
            {
                BookInfo* info = loadBook(xml);
                m_infos.insert(info->path, info);
            }
        }
    }

    file.close();

    if(xml.hasError())
    {
        qWarning() << Q_FUNC_INFO << "XML with errors. Books loaded until errors:" << m_infos.size();
        file.copy(file.fileName() + ".backup");
        qWarning() << Q_FUNC_INFO << "file to remove:" << file.remove();
    }

    return true;
}

BookInfo* ModelBackendOneFile::loadBook (QXmlStreamReader& xml)
{
    BookInfo * info = new BookInfo();

    while (!xml.atEnd() && !xml.hasError())
    {
        xml.readNext();
        if(xml.isEndElement())
        {
            if(xml.name() == "book")
                break;
        } else {
            QStringRef name = xml.name();
            if (name == "isbn") {
                info->isbn = xml.readElementText();
            } else if (name == "path") {
                info->path = xml.readElementText();
            } else if (name == "format") {
                info->format = xml.readElementText();
            } else if (name == "publisher") {
                info->publisher = xml.readElementText();
            } else if (name == "synopsis") {
                info->synopsis = xml.readElementText();
            } else if (name == "syncDate") {
                info->syncDate = QDateTime::fromString(xml.readElementText(), Qt::ISODate);
            } else if (name == "title") {
                info->title = xml.readElementText();
            } else if (name == "author") {
                info->author = xml.readElementText();
            }else if (name == "thumbnail") {
                info->thumbnail = xml.readElementText();
            }else if (name == "publish-time") {
                info->publishTime = QDateTime::fromString(xml.readElementText(), Qt::ISODate);
            } else if (name == "download-time") {
                info->downloadTime = QDateTime::fromString(xml.readElementText(), Qt::ISODate);
            } else if (name == "lastTimeRead") {
                QString lastTime = xml.readElementText();
                if(!lastTime.isEmpty())
                    info->lastTimeRead = QDateTime::fromString(lastTime, Qt::ISODate);
            } else if (name == "readingPercentage") {
                info->readingPercentage = xml.readElementText().toDouble();
            } else if (name == "readingProgress") {
                info->readingProgress = xml.readElementText().toDouble();
            } else if (name == "readingPeriod") {
                info->setReadingPeriod(xml.readElementText().toLong());
            } else if (name == "mark-count") {
                info->markCount = xml.readElementText().toInt();
            } else if (name == "note-count") {
                info->noteCount = xml.readElementText().toInt();
            } else if (name == "hili-count") {
                info->hiliCount = xml.readElementText().toInt();
            } else if (name == "page-count") {
                info->pageCount = xml.readElementText().toInt();
            } else if (name == "last-read-link") {
                info->lastReadLink = xml.readElementText();
            } else if (name == "last-read-page") {
                info->lastReadPage = xml.readElementText().toInt();
            } else if (name == "font-size") {
                info->fontSize = xml.readElementText().toDouble();
            } else if (name == "page-mode") {
                info->pageMode = xml.readElementText().toInt();
            } else if (name == "corrupted") {
                info->corrupted = xml.readElementText().toInt();
            } else if (name == "type") {
                info->m_type = (BookInfo::bookTypeEnum)xml.readElementText().toInt();
            } else if (name == "archived") {
                info->m_archived = xml.readElementText().toInt();
            } else if (name == "isDRMFile") {
                info->isDRMFile = xml.readElementText().toInt();
            } else if (name == "storePrice") {
                info->storePrice = xml.readElementText();
            } else if (name == "expirationDate") {
                info->m_expirationDate = QDateTime::fromString(xml.readElementText(), Qt::ISODate);
            } else if (name == "totalReadingTime") {
                info->totalReadingTime = xml.readElementText().toInt();
            } else if (name == "timestamp") {
                info->timestamp = xml.readElementText().toLongLong();
            } else if (name == "cssFiles") {
                info->setCSSFileList(xml.readElementText().split(";"));
            } else if (name == "percentageList") {
                info->percentageList = xml.readElementText();
            } else if (name == "readingStatus") {
                info->readingStatus = (BookInfo::readStateEnum)xml.readElementText().toInt();
            } else if (name == "marks") {
                while (!xml.atEnd() && !xml.hasError())
                {
                    xml.readNext();
                    if (xml.isEndElement()) {
                        if (xml.name() == "marks") break;
                    } else if (xml.isStartElement()) {
                        if (xml.name() == "mark") {
                            BookLocation* loc = loadMark(xml);
                            info->addLocation(loc->bookmark, loc);
                        }

                    }
                }
            }
            else if (name == "collections") {
                while (!xml.atEnd() && !xml.hasError())
                {
                    xml.readNext();
                    if (xml.isEndElement()) {
                        if (xml.name() == "collections") break;
                    } else
                    {
                        QStringRef name = xml.name();
                        if (name == "collection") {
                            QString collection = xml.readElementText();
                            info->addCollection(collection);
                        }

                    }
                }
            }
        }
    }
    if(info->readingStatus == BookInfo::NONE)
    {
        if (info->lastTimeRead.isValid()) // reading ongoing
        {
            info->readingStatus = BookInfo::READING_BOOK;
        }
        else
        {
            if(info->lastReadLink.isEmpty()) // New ebooks ever opened
                info->readingStatus = BookInfo::NO_READ_BOOK;
            else // Closed ebooks
                info->readingStatus = BookInfo::READ_BOOK;
        }
    }
    return info;
}
BookLocation* ModelBackendOneFile::loadMark (QXmlStreamReader& xml)
{
    BookLocation* loc = new BookLocation();

    loc->type = BookLocation::getTypeFromString(xml.attributes().value("type").toString());
    loc->operation = BookLocation::getOperationFromString(xml.attributes().value("operation").toString());

    while (!xml.atEnd() && !xml.hasError()) {

        xml.readNext();

        if (xml.isEndElement()) {
            if (xml.name() == "mark") break;
        } else{
            QStringRef name = xml.name();
            if (name == "text") {
                loc->text = xml.readElementText();
            }else if (name == "note") {
                loc->note = xml.readElementText();
            }else if (name == "pos") {
                loc->pos = xml.readElementText().toDouble();
            } else if (name == "bookmark") {
                loc->bookmark = xml.readElementText();
            } else if (name == "page") {
                loc->page = xml.readElementText().toInt();
            } else if (name == "lastUpdated") {
                loc->lastUpdated = xml.readElementText().toLongLong();
            } else if (name == "chapterName") {
                loc->chapterName = xml.readElementText();
            }

        }
    }
    return loc;
}
qint64 ModelBackendOneFile::getServerTimestamp () const
{
    QString path = _privatePartition + QDir::separator() + BOOK_DATA_FILE;

    QFile file(QDir::cleanPath(path));
    if (!file.open(QFile::ReadOnly))
        qDebug() << "File can not be opened";

    QXmlStreamReader xml(&file);

    xml.readNext(); //Skip first tag
    xml.readNext();
    qint64 timestamp = 0;

    if(xml.name() == "books") {
        timestamp = xml.attributes().value("serverTimestamp").toString().toLongLong();
        qDebug() << Q_FUNC_INFO << "ServerTimestamp: " << timestamp;
    }

    return timestamp;
}

BookInfo* ModelBackendOneFile::loadDefaultInfo( const QString& path )
{
    qDebug() << Q_FUNC_INFO << "Path: " << path;

    BookInfo* bookInfo = new BookInfo();

    if (MetaDataExtractor::getMetaData(path, bookInfo->title, bookInfo->author, bookInfo->publisher, bookInfo->publishTime, bookInfo->synopsis, bookInfo->format, bookInfo->isDRMFile) == true) {
        if(bookInfo->title.isEmpty())
            bookInfo->title = QFileInfo(path).baseName();

        if(bookInfo->author.isEmpty())
            bookInfo->author = QString("--");

        bookInfo->setCSSFileList(MetaDataExtractor::extractCSS(path));
    } else {
        qWarning() << Q_FUNC_INFO << "Corrupted book: " << path;
        bookInfo->title = QFileInfo(path).baseName();
        bookInfo->corrupted = true;
        // FIXME: Get proper icon for this
        bookInfo->thumbnail = QString(":/res/corrupted_book_ico.png");
        // TODO: No están todos los atributos.
    }

    bookInfo->path = path;
    if (path.toLower().endsWith(".epub") || path.toLower().endsWith(".fb2"))
         bookInfo->fontSize = 2;

    return bookInfo;
}

void ModelBackendOneFile::add( const BookInfo* book )
{
    qDebug() << Q_FUNC_INFO;

    bool mount = false;
    QHash<QString, bool>::iterator j = m_pathsMount.begin();
    QHash<QString, bool>::iterator jEnd = m_pathsMount.end();
    while(j != jEnd)
    {
       if(book->path.contains(j.key()))
       {
           mount = true;
           break;
       }
       ++j;
    }

    if(!mount)
        return;

    TBooks::iterator i = m_books->find(book->path);
    if (i == m_books->end())
    {
        BookInfo *newBookInfo = new BookInfo(*book);
        newBookInfo->setCSSFileList(MetaDataExtractor::extractCSS(book->path));
        m_books->insert(book->path, newBookInfo);
    } else {
        if (!book->m_archived && book->getCSSFileList().isEmpty())
            book->setCSSFileList(MetaDataExtractor::extractCSS(book->path));
        i.value()->update(book);
    }
    messDir(book->path, UPDATE_FULL);
}

void ModelBackendOneFile::update( const BookInfo* book )
{
    qDebug() << Q_FUNC_INFO;
    int updateType = UPDATE_FULL;

    bool mount = false;
    QHash<QString, bool>::iterator j = m_pathsMount.begin();
    QHash<QString, bool>::iterator jEnd = m_pathsMount.end();
    while(j != jEnd)
    {
       if(book->path.contains(j.key())){
           mount = true;
           break;
       }
       ++j;
    }

    if(!mount)
        return;

    TBooks::iterator i = m_books->find(book->path);
    if (i != m_books->end())
    {
        if (!book->m_archived && book->getCSSFileList().isEmpty())
            book->setCSSFileList(MetaDataExtractor::extractCSS(book->path));

        updateType = i.value()->update(book);
    }
    messDir(book->path, updateType);
}

void ModelBackendOneFile::archive (const BookInfo * book)
{
    qDebug() << "--->" << Q_FUNC_INFO;
    addTransaction(book->isbn, true);
    update(book);
}

void ModelBackendOneFile::unArchive (const BookInfo * book)
{
    qDebug() << "--->" << Q_FUNC_INFO;
    update(book);
}

void ModelBackendOneFile::remove( const BookInfo* book )
{
    qDebug() << "--->" << Q_FUNC_INFO << " Removing " << book->path;

    TBooks::iterator it = m_books->find(book->path);
    if(it != m_books->end())
    {
        QFile::remove(book->path);
        QFile::remove(book->thumbnail);
        qDebug() << "--->" << Q_FUNC_INFO << " book " << book->path << " removed";
        QString path = book->path;
        if(book->m_type == BookInfo::BOOKINFO_TYPE_DEMO && book->path.contains("private"))
            addTransaction(book->isbn, true);

        m_books->take(book->path);
        delete *it;
        *it = NULL;

        messDir(path, UPDATE_FULL);// TODO: Path is temporary, and the parameter is const QString&!!
    }
}

void ModelBackendOneFile::close( const BookInfo* book )
{
    qDebug() << Q_FUNC_INFO << "Closing: " << book->path;
    update(book);
}

void ModelBackendOneFile::messDir (const QString& path, const int& updateType)
{
    QHash<QString, bool>::iterator i = m_pathsMount.begin();
    QHash<QString, bool>::iterator itEnd = m_pathsMount.end();
    while(i != itEnd)
    {
        if(path.contains(i.key()))
        {
            m_pathsMount.insert(i.key(), true);
            emit modelChanged(path, updateType);
            break;
        }
        ++i;
    }
}

void ModelBackendOneFile::save (const QString& path, long long serverTimestamp) const
{
    QFile file(path + QDir::separator() +  BOOK_DATA_FILE);
    file.open(QFile::ReadWrite);

    file.seek(0);
    file.resize(0);


    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(-1);

    xml.writeStartDocument();
    xml.writeStartElement("books");
    xml.writeAttribute("model_version", QString::number(MODEL_VERSION));
    xml.writeAttribute("serverTimestamp", QString::number(serverTimestamp));

    if(path == _privatePartition)
    {
        xml.writeStartElement("collections");
        QStringList::const_iterator it1 = m_collections->constBegin();
        QStringList::const_iterator itEnd1 = m_collections->constEnd();
        while(it1 != itEnd1)
        {
            xml.writeTextElement("collection", (*it1));
            ++it1;
        }
        xml.writeEndElement();
    }

    TBooks::const_iterator i = m_books->constBegin();
    TBooks::const_iterator itEnd = m_books->constEnd();
    while(i != itEnd)
    {
        if(i.key().contains(path))
        {
            xml.writeStartElement("book");
            saveBookInfo(i.value(), xml);
            xml.writeEndElement();
        }
        ++i;
    }

    xml.writeEndElement();
    xml.writeEndDocument();
    file.close();
}

void ModelBackendOneFile::saveBookInfo (const BookInfo* info, QXmlStreamWriter &xml) const
{
    xml.writeTextElement("path", info->path);
    xml.writeTextElement("isbn", info->isbn);
    xml.writeTextElement("format", info->format);
    xml.writeTextElement("title", info->title);
    xml.writeTextElement("publisher", info->publisher);
    xml.writeTextElement("synopsis", info->synopsis);
    xml.writeTextElement("syncDate", info->syncDate.toString(Qt::ISODate));
    xml.writeTextElement("author", info->author);
    xml.writeTextElement("thumbnail", info->thumbnail);
    xml.writeTextElement("storePrice", info->storePrice);
    xml.writeTextElement("lastTimeRead", info->lastTimeRead.toString(Qt::ISODate));
    xml.writeTextElement("readingPercentage", QString::number(info->readingPercentage));
    xml.writeTextElement("readingProgress", QString::number(info->readingProgress));
    xml.writeTextElement("readingPeriod", QString::number(info->getReadingPeriod()));
    xml.writeTextElement("publish-time", info->publishTime.toString(Qt::ISODate));
    xml.writeTextElement("download-time", info->downloadTime.toString(Qt::ISODate));
    xml.writeTextElement("mark-count", QString::number(info->markCount));
    xml.writeTextElement("note-count", QString::number(info->noteCount));
    xml.writeTextElement("hili-count", QString::number(info->hiliCount));
    xml.writeTextElement("page-count", QString::number(info->pageCount));
    xml.writeTextElement("last-read-link", info->lastReadLink);
    xml.writeTextElement("last-read-page", QString::number(info->lastReadPage));
    xml.writeTextElement("font-size", QString::number(info->fontSize));
    xml.writeTextElement("page-mode", QString::number(info->pageMode));
    xml.writeTextElement("corrupted", QString::number(info->corrupted));
    xml.writeTextElement("type", QString::number(info->m_type));
    xml.writeTextElement("archived", QString::number(info->m_archived));
    xml.writeTextElement("isDRMFile", QString::number(info->isDRMFile));
    xml.writeTextElement("expirationDate", info->m_expirationDate.toString(Qt::ISODate));
    xml.writeTextElement("timestamp", QString::number(info->timestamp));
    xml.writeTextElement("totalReadingTime", QString::number(info->totalReadingTime));
    xml.writeTextElement("cssFiles", info->getCSSFileList().join(";"));
    xml.writeTextElement("percentageList", info->percentageList);
    xml.writeTextElement("readingStatus", QString::number(info->readingStatus));
    xml.writeStartElement("marks");

    const QHash<QString, BookLocation*>& locations = info->getLocations();
    QHash<QString, BookLocation*>::const_iterator it = locations.constBegin();
    QHash<QString, BookLocation*>::const_iterator itEnd = locations.constEnd();
    while( it!=itEnd )
    {
        xml.writeStartElement("mark");
        saveLocation(it.value(), xml);
        m_books->value(info->path)->locationsPosCache.insert(it.value()->bookmark, it.value()->pos);
        m_books->value(info->path)->locationsPageCache.insert(it.value()->bookmark, it.value()->page);
        xml.writeEndElement();
        ++it;
    }
    xml.writeEndElement();
    xml.writeStartElement("collections");
    const QStringList& collections = info->getCollectionsList();
    QStringList::const_iterator it1 = collections.begin();
    QStringList::const_iterator itEnd1 = collections.constEnd();
    while( it1!=itEnd1 )
    {
        xml.writeTextElement("collection", (*it1));
        ++it1;
    }

    xml.writeEndElement();
}

void ModelBackendOneFile::saveLocation (const BookLocation* loc, QXmlStreamWriter &xml) const
{
    xml.writeAttribute("type", BookLocation::getStringFromType(loc->type));
    xml.writeAttribute("operation", BookLocation::getStringFromOperation(loc->operation));
    QString text = bqUtils::specialCharactersFromUtf8(loc->text);
    xml.writeTextElement("text", text);
    xml.writeTextElement("pos", QString::number(loc->pos));
    xml.writeTextElement("note", loc->note);
    xml.writeTextElement("bookmark", loc->bookmark);
    xml.writeTextElement("page", QString::number(loc->page));
    xml.writeTextElement("lastUpdated", QString::number(loc->lastUpdated));
    xml.writeTextElement("chapterName", loc->chapterName);
}



// Load transactions and insert new transaction
void ModelBackendOneFile::addTransaction(const QString& isbn, bool _filed)
{
    qDebug() << Q_FUNC_INFO << " size before: " << m_transactions->size();
    QString path = _privatePartition + QDir::separator() + TRANSACTIONS_DATA_FILE;
    QFile file(QDir::cleanPath(path));
    if(file.open(QFile::ReadOnly))
    {
        QXmlStreamReader xml(&file);
        xml.readNext(); //Skip first tag
        xml.readNext();

        if(xml.name() == "transactions")
        {
            while (!xml.atEnd() && !xml.hasError())
            {
                xml.readNext();
                if (xml.name() == "transaction" && xml.isStartElement())
                {
                    QString isbn = xml.attributes().value("isbn").toString();
                    bool filed = (bool)xml.attributes().value("filed").toString().toInt();
                    m_transactions->insert(isbn, filed);
                }
            }
        }
        file.close();
    }

    m_transactions->insert(isbn, _filed);
    qDebug() << Q_FUNC_INFO << " size after: " << m_transactions->size();
    writeTransactions();
    m_transactions->clear();
}

void ModelBackendOneFile::removeTransaction(const QString& _isbn)
{
    qDebug() << Q_FUNC_INFO;
    QString path = _privatePartition + QDir::separator() + TRANSACTIONS_DATA_FILE;
    QFile file(QDir::cleanPath(path));
    if(file.open(QFile::ReadOnly))
    {
        QXmlStreamReader xml(&file);
        xml.readNext(); //Skip first tag
        xml.readNext();

        if(xml.name() == "transactions")
        {
            while (!xml.atEnd() && !xml.hasError())
            {
                xml.readNext();
                if (xml.name() == "transaction" && xml.isStartElement())
                {
                    QString isbn = xml.attributes().value("isbn").toString();
                    bool filed = (bool)xml.attributes().value("filed").toString().toInt();
                    if(isbn != _isbn)
                        m_transactions->insert(_isbn, filed);
                }
            }
        }
        file.close();
    }
    writeTransactions();
    m_transactions->clear();
}

void ModelBackendOneFile::writeTransactions()
{
    // Write to file
    QString path = _privatePartition + QDir::separator() + TRANSACTIONS_DATA_FILE;
    QFile file(QDir::cleanPath(path));
    file.open(QFile::ReadWrite);

    file.seek(0);
    file.resize(0);

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(-1);

    xml.writeStartDocument();
    xml.writeStartElement("transactions");

    QHash<QString, bool>::const_iterator it = m_transactions->constBegin();
    QHash<QString, bool>::const_iterator itEnd = m_transactions->constEnd();

    for(; it != itEnd; ++it)
    {
        xml.writeStartElement("transaction");
        xml.writeAttribute("isbn", it.key());
        xml.writeAttribute("filed", it.value() ? "1" : "0");
        xml.writeEndElement();
    }

    xml.writeEndElement();
    xml.writeEndDocument();
    file.close();
}

void ModelBackendOneFile::clearTransactions()
{
    qDebug() << Q_FUNC_INFO;
    m_transactions->clear();
    QString path = _privatePartition + QDir::separator() + TRANSACTIONS_DATA_FILE;
    QFile file(QDir::cleanPath(path));
    file.open(QFile::ReadWrite);
    file.seek(0);
    file.resize(0);
    file.close();
}

bool ModelBackendOneFile::writeNotesToFile( const BookInfo* bookInfo, const QString& fileType, const QString& destination)
{
    qDebug() << Q_FUNC_INFO;
    if(bookInfo->getLocations().size() > 0)
    {
        if(fileType == "xml")
            writeXmlFile(bookInfo, destination);
        else
            writeHTMLFile(bookInfo, destination);

        ::sync();
    }
    return true;
}

void ModelBackendOneFile::writeXmlFile( const BookInfo* bookInfo, const QString& destination)
{
    qDebug() << Q_FUNC_INFO;
    QString path = destination + ".xml";
    QFile file(QDir::cleanPath(path));
    if(file.exists())
        file.remove();

    file.open(QFile::ReadWrite);

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(-1);
    xml.writeStartDocument();
    xml.writeStartElement("marks");

    const QHash<QString, BookLocation*>& locations = bookInfo->getLocations();
    QHash<QString, BookLocation*>::const_iterator it = locations.constBegin();
    QHash<QString, BookLocation*>::const_iterator itEnd = locations.constEnd();
    while( it != itEnd )
    {
        xml.writeStartElement("mark");
        exportXmlLocation(it.value(), xml);
        xml.writeEndElement();
        ++it;
    }
    xml.writeEndElement();
    xml.writeEndDocument();
    file.close();
}

void ModelBackendOneFile::writeHTMLFile( const BookInfo* bookInfo, const QString& destination)
{
    qDebug() << Q_FUNC_INFO;
    QString html;
    QString path = destination + ".html";
    QFile file(QDir::cleanPath(path));
    if(file.exists())
        file.remove();

    file.open(QFile::ReadWrite);

    QList<const BookLocation*>* locations = bookInfo->getLocationList();
    QList<const BookLocation*>::const_iterator it = locations->constBegin();
    QList<const BookLocation*>::const_iterator itEnd = locations->constEnd();

    html.append("<html>");
    html.append("<head>");
    html.append("</head>");
    html.append("<body>");

    while( it != itEnd )
    {
        if((*it)->operation != BookLocation::DELETE){
            exportHtmlLocation((*it), html);
            html.append("</div>");
        }

        ++it;
    }
    html.append("</body>");
    html.append("</html>");
    file.write(html.toAscii().data());
    file.close();
}


void ModelBackendOneFile::exportHtmlLocation (const BookLocation* loc, QString& html)
{
    html.append("<div style=\"margin:10px 0px;padding-bottom:5px;border-bottom:1px solid #333;\">");
    html.append("<p style=\"font-weight:bold;margin:0px 0px 8px;\">Type: ");
    html.append(BookLocation::getStringFromType(loc->type));
    html.append("</p>");

    html.append("<p style=\"margin:0px 0px 8px;\">Page: ");
    html.append(QString::number(loc->page));
    html.append("</p>");

    html.append("<p style=\"margin:0px 0px 8px;\">Chapter: ");
    if(loc->chapterName.size())
        html.append(loc->chapterName);
    else
        html.append("--");
    html.append("</p>");

    if (loc->type == BookLocation::BOOKMARK){
        QString date = QDateTime::fromTime_t(loc->lastUpdated / 1000).toString();
        html.append("<p style=\"font-style:italic;margin:0px 0px 8px;text-align:right;\">Date: ");
        html.append(date);
        html.append("</p>");
        return;
    }

    QString text = bqUtils::specialCharactersFromUtf8(loc->text);
    html.append("<p style=\"margin:0px 0px 8px;\">Text: ");
    html.append(text);
    html.append("</p>");

    if(loc->type == BookLocation::NOTE){
        html.append("<p style=\"margin:0px 0px 8px;\">Note: ");
        html.append(loc->note);
        html.append("</p>");
    }

    QString date = QDateTime::fromTime_t(loc->lastUpdated / 1000).toString();
    html.append("<p style=\"font-style:italic;margin:0px 0px 8px;text-align:right;\">Date: ");
    html.append(date);
    html.append("</p>");

}

void ModelBackendOneFile::exportXmlLocation (const BookLocation* loc, QXmlStreamWriter &xml) const
{
    xml.writeAttribute("type", BookLocation::getStringFromType(loc->type));
    QString text = bqUtils::specialCharactersFromUtf8(loc->text);
    xml.writeTextElement("text", text);
    xml.writeTextElement("position", QString::number(loc->pos));

    if(loc->type == BookLocation::NOTE)
        xml.writeTextElement("note", loc->note);

    xml.writeTextElement("bookmark", loc->bookmark);
    xml.writeTextElement("page", QString::number(loc->page));
    QString date = QDateTime::fromTime_t(loc->lastUpdated / 1000).toString();
    xml.writeTextElement("date", date);
}

bool ModelBackendOneFile::booksToLoad( const QString& path, QStringList& booksPaths )
{
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

        if (name.startsWith(".") || !BookInfo::isBook(file_path)) {
            ++i;
            continue;
        }

        // Handle problematic characters
        if(name.contains("+"))
        {
            // Rename the file
            QFile renaming(file_path);
            if(renaming.exists())
            {
                QString newFilePath(file_path);
                newFilePath.replace("+", "");
                qDebug() << Q_FUNC_INFO << "Renaming: " << file_path << " to " << newFilePath;
                if(renaming.rename(newFilePath))
                {
                    fi.setFile(renaming);
                    file_path = fi.absoluteFilePath();
                }
                else
                    return false;
            }
            else
                return false;
        }

        booksPaths.append(file_path);

        ++i;
    }

    QStringList::const_iterator j = sub_list.begin();
    while(j != sub_list.end())
    {
        if(!booksToLoad(*j, booksPaths))
            return false;
        ++j;
    }

    return true;
}

void ModelBackendOneFile::addCollection(QString collection)
{
    bool isAddedYet = false;
    for(int j = 0; j < m_collections->size(); j++)
    {
        if(m_collections->at(j) == collection)
        {
            isAddedYet = true;
            break;
        }
    }
    if(!isAddedYet)
        m_collections->append(collection);
}

void ModelBackendOneFile::removeCollection(QString collection)
{
    for(int j = 0; j < m_collections->size(); j++)
    {
        if(m_collections->at(j) == collection)
        {
            m_collections->removeAt(j);
            break;
        }
    }
}

QStringList ModelBackendOneFile::getCollections()
{
    return (*m_collections);
}
