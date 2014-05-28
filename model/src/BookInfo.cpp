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

#include "BookInfo.h"
#include "json.h"

#include <epub.h>
#include <QStringList>
#include <QFileInfo>
#include <QDebug>
#include <QtCore/qmath.h>
#include <QDir>

#define CHUNK_SIZE = 500;
#define EPS (0.000001)

const int si_bookTypeNum = BookInfo::BOOKINFO_TYPE_COUNT;
static const char* sz_bookType[si_bookTypeNum] = {"purchase\0", "sample\0", "subscription\0", "lending\0"};

BookInfo::BookInfo(const QString& _path)
    : title("")
    , isbn("")
    , path(_path)
    , author("")
    , thumbnail("")
    , coverUrl("")
    , publishTime()
    , downloadTime()
    , markCount(0)
    , noteCount(0)
    , hiliCount(0)
    , pageCount(0)
    , lastReadLink("")
    , lastReadPage(0)
    , readingProgress(0)
    , readingPercentage(0)
    , fontSize(0.0)
    , pageMode(0)
    , orientation(ORI_PORTRAIT)
    , timestamp(0)
    , publisher("")
    , synopsis("")
    , fileSize(0)
    , syncDate()
    , storePrice("")
    , format("")
    , m_type(BOOKINFO_TYPE_PURCHASE)
    , m_expirationDate()
    , corrupted(false)
    , m_archived(false)
    , isDRMFile(false)
    , readingPeriod(0)
    , totalReadingTime(0)
    , percentageList("")
    , readingStatus(NONE)
    , size(0)
    , language("")
{
    m_cssFileList.clear();
    m_collections.clear();
}

BookInfo::BookInfo(const BookInfo& other)
    : title(other.title)
    , isbn(other.isbn)
    , lastTimeRead(other.lastTimeRead)
    , path(other.path)
    , author(other.author)
    , thumbnail(other.thumbnail)
    , coverUrl(other.coverUrl)
    , publishTime(other.publishTime)
    , downloadTime(other.downloadTime)
    , markCount(other.markCount)
    , noteCount(other.noteCount)
    , hiliCount(other.hiliCount)
    , pageCount(other.pageCount)
    , lastReadLink(other.lastReadLink)
    , lastReadPage(other.lastReadPage)
    , readingProgress(other.readingProgress)
    , readingPercentage(other.readingPercentage)
    , fontSize(other.fontSize)
    , pageMode(other.pageMode)
    , orientation(other.orientation)
    , timestamp(other.timestamp)
    , publisher(other.publisher)
    , synopsis(other.synopsis)
    , fileSize(other.fileSize)
    , syncDate(other.syncDate)
    , storePrice(other.storePrice)
    , format(other.format)
    , m_type(other.m_type)
    , m_expirationDate(other.m_expirationDate)
    , corrupted(other.corrupted)
    , m_archived(other.m_archived)
    , isDRMFile(other.isDRMFile)
    , readingPeriod(other.readingPeriod)
    , m_collections(other.m_collections)
    , m_cssFileList(other.m_cssFileList)
    , totalReadingTime(other.totalReadingTime)
    , percentageList(other.percentageList)
    , readingStatus(other.readingStatus)
    , size(other.size)
    , language(other.language)
{
    QHash<QString, BookLocation*>::const_iterator it = other.m_locations.constBegin();
    QHash<QString, BookLocation*>::const_iterator itEnd = other.m_locations.constEnd();
    while( it != itEnd )
    {
        m_locations.insert(it.key(), new BookLocation(*it.value()));
        ++it;
    }

    QHash<QString, double>::const_iterator itPosCache = other.locationsPosCache.constBegin();
    QHash<QString, double>::const_iterator itEndPosCache = other.locationsPosCache.constEnd();
    while( itPosCache != itEndPosCache )
    {
        locationsPosCache.insert(itPosCache.key(), itPosCache.value());
        ++itPosCache;
    }

    QHash<QString, int>::const_iterator itPageCache = other.locationsPageCache.constBegin();
    QHash<QString, int>::const_iterator itEndPageCache = other.locationsPageCache.constEnd();
    while( itPageCache != itEndPageCache )
    {
        locationsPageCache.insert(itPageCache.key(), itPageCache.value());
        ++itPageCache;
    }
}

BookInfo::~BookInfo()
{
    clearLocations();
}

QString BookInfo::toString() const
{
    QString string;
    string += "Title:" + title;
    string +=  "ISBN:" + isbn;
    string +=  "Author" + author;
    string +=  "lastReadTime" + lastTimeRead.toString(Qt::ISODate);

    return string;
}

bool pageLessThan(const BookLocation* bl1, const BookLocation* bl2)
{
    return bl1->page <= bl2->page;
}

bool posLessThan(const BookLocation* bl1, const BookLocation* bl2)
{
    return bl1->pos <= bl2->pos;
}

QList<const BookLocation*>* BookInfo::getLocationList( const QList<BookLocation::Type>& types ) const
{
    QList<const BookLocation*>* locations = new QList<const BookLocation*>();

    QHash<QString, BookLocation*>::const_iterator it = m_locations.constBegin();
    QHash<QString, BookLocation*>::const_iterator itEnd = m_locations.constEnd();
    if(types.isEmpty())
    {
        for(; it != itEnd; ++it)
        {
            if(it.value()->operation != BookLocation::DELETE)
                locations->append(it.value());
        }
    }
    else
    {
        for(; it != itEnd; ++it)
        {
            if( types.contains(it.value()->type) )
            {
                if(it.value()->operation != BookLocation::DELETE)
                    locations->append(it.value());
            }
        }
    }

    qSort(locations->begin(), locations->end(), posLessThan);

    return locations;
}

int BookInfo::getLocationListCount( const QList<BookLocation::Type>& types ) const
{
    int count = 0;

    QHash<QString, BookLocation*>::const_iterator it = m_locations.constBegin();
    QHash<QString, BookLocation*>::const_iterator itEnd = m_locations.constEnd();
    if(types.isEmpty())
    {
        for(; it != itEnd; ++it)
        {
            if(it.value()->operation != BookLocation::DELETE)
                count++;
        }
    }
    else
    {
        for(; it != itEnd; ++it)
        {
            if( types.contains(it.value()->type) ){
                if(it.value()->operation != BookLocation::DELETE)
                    count++;
            }
        }
    }

    return count;
}

void BookInfo::getLocationsAsString ( QStringList& locations, const QList<BookLocation::Type>& types ) const
{
    QHash<QString, BookLocation*>::const_iterator it = m_locations.constBegin();
    QHash<QString, BookLocation*>::const_iterator itEnd = m_locations.constEnd();

    if(types.isEmpty())
    {
        for(; it != itEnd; ++it)
        {
            if(it.value()->operation != BookLocation::DELETE)
                locations.append(it.value()->bookmark);
        }
    }
    else
    {
        for(; it != itEnd; ++it)
        {
            if( types.contains(it.value()->type) ){
                if(it.value()->operation != BookLocation::DELETE)
                    locations.append(it.value()->bookmark);
            }
        }
    }
}

void BookInfo::setLocations(const QHash<QString, BookLocation*>& locations)
{
    qDebug() << Q_FUNC_INFO;
    setCachesFromInfo(locations);
    clearLocations();
    QHash<QString, BookLocation*>::const_iterator it = locations.constBegin();
    QHash<QString, BookLocation*>::const_iterator itEnd = locations.constEnd();
    while( it != itEnd )
    {
        BookLocation* location = new BookLocation(*(it.value()));
        m_locations.insert(it.key(), location);
        ++it;
    }
    setInfoFromCaches();
}

BookLocation* BookInfo::getLocation( const QString& ref ) const
{
    return m_locations.value(ref, NULL);
}

void BookInfo::clearLocations()
{
    QHash<QString, BookLocation*>::iterator it = m_locations.begin();
    QHash<QString, BookLocation*>::iterator itEnd = m_locations.end();
    for(; it != itEnd; ++it)
    {
        delete *it;
    }
    m_locations.clear();
}

void BookInfo::addLocation( const QString& key, BookLocation* location )
{
    m_locations.insert(key, location);
}

void BookInfo::removeLocation( const QString& key )
{
    QHash<QString, BookLocation*>::iterator it = m_locations.find(key);
    if(it != m_locations.end())
    {
        delete *it;
        m_locations.erase(it);
    }
}

int BookInfo::update(const BookInfo *other) {
    qDebug() << Q_FUNC_INFO << "isbn"  << other->isbn;

    int updateType = UPDATE_NONE;

    if(this == other) return updateType;

    dirty = true;

    if (title != other->title) {
        title = other->title;
        updateType |= UPDATE_METADATA;
    }

    if (isbn != other->isbn) {
        isbn = other->isbn;
        updateType |= UPDATE_METADATA;
    }

    qDebug() << Q_FUNC_INFO << "lastTimeRead1" << lastTimeRead;
    qDebug() << Q_FUNC_INFO << "other->lastTimeRead" << other->lastTimeRead;
    if (lastTimeRead != other->lastTimeRead)
    {
        lastTimeRead = other->lastTimeRead;
        updateType |= UPDATE_METADATA;
    }

    qDebug() << Q_FUNC_INFO << "readingStatus" << readingStatus;
    qDebug() << Q_FUNC_INFO << "other->readingStatus: " << other->readingStatus;
    if(readingStatus != other->readingStatus)
    {
        readingStatus = other->readingStatus;
        if(other->readingStatus == BookInfo::READ_BOOK || other->readingStatus == BookInfo::NO_READ_BOOK)
            updateType |= UPDATE_CLOSED;
    }

    if (path != other->path) {
        path = other->path;
        updateType |= UPDATE_METADATA;
    }
    if (author != other->author) {
        author = other->author;
        updateType |= UPDATE_METADATA;
    }

    if (thumbnail != other->thumbnail) {
        thumbnail = other->thumbnail;
        updateType |= UPDATE_METADATA;
    }

    if (coverUrl != other->coverUrl) {
        coverUrl = other->coverUrl;
        updateType |= UPDATE_METADATA;
    }

    if (publishTime != other->publishTime) {
        publishTime = other->publishTime;
        updateType |= UPDATE_METADATA;
    }

    if (size != other->size) {
        size = other->size;
        updateType |= UPDATE_METADATA;
    }

    downloadTime = other->downloadTime;

    if (markCount != other->markCount) {
        markCount = other->markCount;
        updateType |= UPDATE_READING_METADATA;
    }

    if (noteCount != other->noteCount) {
        noteCount = other->noteCount;
        updateType |= UPDATE_READING_METADATA;
    }

    if (hiliCount != other->hiliCount) {
        hiliCount = other->hiliCount;
        updateType |= UPDATE_READING_METADATA;
    }

    if (pageCount != other->pageCount) {
        pageCount = other->pageCount;
        updateType |= UPDATE_READING_METADATA;
    }

    if (lastReadLink != other->lastReadLink) {
        lastReadLink = other->lastReadLink;
        updateType |= UPDATE_READING_POSITION;
        }

    if (lastReadPage != other->lastReadPage) {
        lastReadPage = other->lastReadPage;
        updateType |= UPDATE_READING_POSITION;
        }

    if (fabs(readingProgress - other->readingProgress) > EPS) {
        readingProgress = other->readingProgress;
        updateType |= UPDATE_READING_POSITION;
        }

    if (fabs(readingPercentage != other->readingPercentage) > EPS) {
        readingPercentage = other->readingPercentage;
        updateType |= UPDATE_READING_POSITION;
    }

    if (readingPeriod != other->readingPeriod)
        readingPeriod = other->readingPeriod;

    if (percentageList != other->percentageList)
        percentageList = other->percentageList;

    if (fontSize != other->fontSize) {
        fontSize = other->fontSize;
        updateType |= UPDATE_READING_METADATA;
    }

    if (pageMode != other->pageMode) {
        pageMode = other->pageMode;
        updateType |= UPDATE_READING_METADATA;
    }

    if (orientation != other->orientation) {
        orientation = other->orientation;
        updateType |= UPDATE_READING_METADATA;
    }

    if (timestamp != other->timestamp) {
        timestamp = other->timestamp;
        updateType |= UPDATE_METADATA;
    }

    publisher = other->publisher;
    synopsis = other->synopsis;
    fileSize = other->fileSize;
    syncDate = other->syncDate;
    storePrice = other->storePrice;
    format = other->format;
    language = other->language;

    if (m_type != other->m_type) {
        m_type = other->m_type;
        updateType |= UPDATE_TYPE;
    }

    if (m_expirationDate != other->m_expirationDate)
        m_expirationDate = other->m_expirationDate;

    if(hasExpired())
        updateType |= UPDATE_EXPIRATION;

    if(m_type == BOOKINFO_TYPE_SUBSCRIPTION)
        corrupted = true;
    else
        corrupted = other->corrupted;

    if (m_archived != other->m_archived) {
        m_archived = other->m_archived;
        updateType |= UPDATE_ARCHIVED;
    }

    if(isDRMFile != other->isDRMFile)
        isDRMFile = other->isDRMFile;

    m_collections = other->m_collections;
    m_cssFileList = other->m_cssFileList;
    totalReadingTime = other->totalReadingTime;

    if(locationsHasChanged(other->m_locations))
    {
        qDebug() << Q_FUNC_INFO << "locations has changed";
        updateType |= UPDATE_READING_METADATA;
        setLocations(other->m_locations);
    }

    return updateType;
}

int BookInfo::updateNewData( const BookInfo *other) {

    qDebug() << Q_FUNC_INFO;
    int updateType = UPDATE_NONE;

    if(this == other) return updateType;

    dirty = true;

    if (title != other->title) {
        title = other->title;
        updateType |= UPDATE_METADATA;
    }

    if (isbn != other->isbn) {
        isbn = other->isbn;
        updateType |= UPDATE_METADATA;
    }

    if (author != other->author) {
        author = other->author;
        updateType |= UPDATE_METADATA;
    }

    if (thumbnail != other->thumbnail) {
        thumbnail = other->thumbnail;
        updateType |= UPDATE_METADATA;
    }

    if (coverUrl != other->coverUrl) {
        coverUrl = other->coverUrl;
        updateType |= UPDATE_METADATA;
    }

    if (publishTime != other->publishTime) {
        publishTime = other->publishTime;
        updateType |= UPDATE_METADATA;
    }

    if (pageCount != other->pageCount) {
        pageCount = other->pageCount;
        updateType |= UPDATE_READING_METADATA;
    }

    publisher = other->publisher;
    synopsis = other->synopsis;
    fileSize = other->fileSize;
    syncDate = other->syncDate;
    storePrice = other->storePrice;
    format = other->format;
    language = other->language;

    if (m_type != other->m_type) {
        m_type = other->m_type;
        updateType |= UPDATE_TYPE;
    }

    if (size != other->size) {
        size = other->size;
        updateType |= UPDATE_METADATA;
    }

    if(isDRMFile != other->isDRMFile)
        isDRMFile = other->isDRMFile;

    qDebug() << Q_FUNC_INFO << "Colección antigua:" << m_collections << "Nuevas colecciones:" << other->m_collections;
    m_collections = other->m_collections;
    m_cssFileList = other->m_cssFileList;

    if(other->getLocationListCount() > 0)
    {
        QList<const BookLocation*>::iterator it = other->getLocationList()->begin();
        QList<const BookLocation*>::iterator itEnd = other->getLocationList()->end();
        for(; it != itEnd; ++it)
        {
            BookLocation* location = new BookLocation(*(*it));
            addLocation(location->bookmark, location);
            location = NULL;
            delete location;
        }
    }
    return updateType;
}

bool BookInfo::locationsHasChanged(const QHash<QString, BookLocation*>& locations)
{
    qDebug() << Q_FUNC_INFO << " " << m_locations.size() << " other: " << locations.size();
    if(m_locations.size() != locations.size())
        return true;

    QHash<QString, BookLocation*>::const_iterator it = m_locations.constBegin();
    QHash<QString, BookLocation*>::const_iterator itEnd = m_locations.constEnd();

    while(it != itEnd)
    {
        if(locations.find(it.key()) == locations.end())
            return true;
        else{
            if(it.value()->note != locations.find(it.key()).value()->note)
                return true;
            if(it.value()->operation != locations.find(it.key()).value()->operation)
                return true;
            if(it.value()->pos != locations.find(it.key()).value()->pos)
                return true;
            if(it.value()->page!= locations.find(it.key()).value()->page)
                return true;
        }
        ++it;
    }

    return false;
}

void BookInfo::setCachesFromInfo(QHash<QString, BookLocation*> locations)
{
    qDebug() << Q_FUNC_INFO;
    QHash<QString, BookLocation*>::iterator it = m_locations.begin();
    QHash<QString, BookLocation*>::iterator itEnd = m_locations.end();
    while( it != itEnd )
    {
        if(locationsPosCache.contains(it.value()->bookmark) && it.value()->pos > 0)
            locationsPosCache.insert(it.value()->bookmark, it.value()->pos);

        if(locationsPageCache.contains(it.value()->bookmark) && it.value()->pos > 0)
            locationsPageCache.insert(it.value()->bookmark, it.value()->page);

        ++it;
    }

    QHash<QString, BookLocation*>::iterator itLoc = locations.begin();
    QHash<QString, BookLocation*>::iterator itLocEnd = locations.end();
    while( itLoc != itLocEnd )
    {
        if(locationsPosCache.contains(itLoc.value()->bookmark) && itLoc.value()->pos > 0)
            locationsPosCache.insert(itLoc.value()->bookmark, itLoc.value()->pos);

        if(locationsPageCache.contains(itLoc.value()->bookmark) && itLoc.value()->pos > 0)
            locationsPageCache.insert(itLoc.value()->bookmark, itLoc.value()->page);

        ++itLoc;
    }

}

void BookInfo::setInfoFromCaches( )
{
    qDebug() << Q_FUNC_INFO;
    QHash<QString, BookLocation*>::iterator it = m_locations.begin();
    QHash<QString, BookLocation*>::iterator itEnd = m_locations.end();
    while( it != itEnd )
    {
        if(locationsPosCache.contains(it.value()->bookmark))
            it.value()->pos = locationsPosCache.value(it.value()->bookmark);

        if(locationsPageCache.contains(it.value()->bookmark))
            it.value()->page = locationsPageCache.value(it.value()->bookmark);

        ++it;
    }
}

void BookInfo::editNote( const QString& _bookmark, const QString& _note, qint64 lastUpdated )
{
    BookLocation* location = m_locations.take(_bookmark);
    if(!location) return;
    location->operation = BookLocation::UPDATE;
    if (_note.trimmed().isEmpty())
    {
        location->type = BookLocation::HIGHLIGHT;
        location->note = "";
    }
    else
    {
        location->type = BookLocation::NOTE;
        location->note = _note;
    }

    location->lastUpdated = lastUpdated;
    m_locations.insert(location->bookmark, location);
}

void BookInfo::deleteLocation( const QString& _bookmark )
{
    qDebug() << "--->" << Q_FUNC_INFO << ". Before deletion, Size: " << m_locations.size();
    m_locations.remove(_bookmark);
    qDebug() << "--->" << Q_FUNC_INFO << ". After deletion, Size: " << m_locations.size();
}

bool BookInfo::isBook( const QString& path )
{
     QFileInfo info(path);
     QString suffix = info.suffix().toLower();
     return (  suffix == "epub"
            || suffix == "fb2"
#ifndef DISABLE_ADOBE_SDK
            || suffix == "pdf"
#endif
            || suffix == "doc"
            || suffix == "mobi"
            || suffix == "txt"
            || suffix == "rtf");
}

void BookInfo::setMarktoRemove(BookLocation* location){

    qDebug() << Q_FUNC_INFO;
    if(!location) return;
    qDebug() << Q_FUNC_INFO << "location: " << location->bookmark;

    if(location->operation == BookLocation::ADD){
        qDebug() << Q_FUNC_INFO << "removing mark completely";
        m_locations.remove(location->bookmark);
    }
    else{
        qDebug() << Q_FUNC_INFO << "removing mark delete";
        location->operation = BookLocation::DELETE;
        location->lastUpdated = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }
}

void BookInfo::setMarktoAdd(BookLocation* location){
    qDebug() << Q_FUNC_INFO;

    location->operation = BookLocation::ADD;
    if(!m_locations.contains(location->bookmark))
    {
        location->lastUpdated = QDateTime::currentDateTime().toMSecsSinceEpoch();
        m_locations.insert(location->bookmark, location);
    }
}


QByteArray BookInfo::parseJSON() const
{
    qDebug() << "--->" << Q_FUNC_INFO;
    QByteArray finalMetadata;
    QString metadata =" { ";

    QHash<QString, BookLocation*>::const_iterator i = m_locations.constBegin();

    QString bookMarksJSON = "\"bookmarks\" : [";
    QString notesJSON = "\"notes\" : [";
    int bookmarksNumber = 0, notesNumber = 0;

    while(i != m_locations.constEnd())
    {
        switch (i.value()->type){
        case BookLocation::BOOKMARK:
            if(i.value()->operation == BookLocation::ADD || i.value()->operation == BookLocation::DELETE ){
                if(bookmarksNumber > 0) bookMarksJSON.append(",");
                bookMarksJSON.append("{ \"path\" : \"");
                bookMarksJSON.append(i.value()->bookmark);
                bookMarksJSON.append("\", \"lastUpdated\" : ");
                bookMarksJSON.append(QString::number(i.value()->lastUpdated));
                bookMarksJSON.append(", \"operation\" : \"");
                bookMarksJSON.append(BookLocation::getStringFromOperation(i.value()->operation));
                bookMarksJSON.append("\" } ");
                bookmarksNumber++;
                break;
            }

        case BookLocation::NOTE:
        case BookLocation::HIGHLIGHT:
            if(i.value()->operation == BookLocation::ADD || i.value()->operation == BookLocation::DELETE || i.value()->operation == BookLocation::UPDATE ){
                if(notesNumber > 0) notesJSON.append(",");
                if(i.value()->operation == BookLocation::UPDATE){
                    notesJSON.append("{ \"path\" : \"");
                    notesJSON.append(i.value()->bookmark);
                    notesJSON.append("\", \"operation\" : \"");
                    notesJSON.append(BookLocation::getStringFromOperation(BookLocation::DELETE));
                    notesJSON.append("\", \"lastUpdated\" : ");
                    notesJSON.append(QString::number(i.value()->lastUpdated));
                    notesJSON.append("},");
                    notesJSON.append("{ \"path\" : \"");
                    notesJSON.append(i.value()->bookmark);
                    notesJSON.append("\", \"note\" : \"");
                    notesJSON.append(i.value()->note);
                    notesJSON.append("\", \"text\" : \"");
                    notesJSON.append(Json::sanitizeString(i.value()->text));
                    notesJSON.append("\", \"lastUpdated\" : ");
                    notesJSON.append(QString::number(i.value()->lastUpdated));
                    notesJSON.append(", \"operation\" : \"");
                    notesJSON.append(BookLocation::getStringFromOperation(BookLocation::ADD));
                    notesJSON.append("\" }");
                    notesNumber+=2;
                    break;
                }else{
                    notesJSON.append("{ \"path\" : \"");
                    notesJSON.append(i.value()->bookmark);
                    notesJSON.append("\", \"note\" : \"");
                    notesJSON.append(i.value()->note);
                    notesJSON.append("\", \"text\" : \"");
                    notesJSON.append(Json::sanitizeString(i.value()->text));
                    notesJSON.append("\", \"lastUpdated\" : ");
                    notesJSON.append(QString::number(i.value()->lastUpdated));
                    notesJSON.append(", \"operation\" : \"");
                    notesJSON.append(BookLocation::getStringFromOperation(i.value()->operation));
                    notesJSON.append("\" }");
                    notesNumber++;
                    break;
                }
            }
        }
        ++i;
    }

    metadata.append(bookMarksJSON + "] ,");
    metadata.append(notesJSON + "] ,");
    metadata.append("\"lastLinkRead\" : \"");
    metadata.append(this->lastReadLink.trimmed());
    metadata.append("\" , ");

    metadata.append("\"readingProgress\" : ");
    metadata.append(QString::number(this->readingProgress));
    metadata.append(", ");

    metadata.append("\"readingPercentage\" : ");
    metadata.append(QString::number(this->readingPercentage));
    metadata.append(", ");

    metadata.append("\"readingPeriod\" : ");
    metadata.append(QString::number(this->readingPeriod));
    metadata.append(" , ");

    metadata.append("\"readingRange\" : ");
    if(this->percentageList.size())
    {
        metadata.append("\"");
        metadata.append(this->percentageList.trimmed());
        metadata.append("\"");
    }
    else
        metadata.append("0");
    metadata.append(" , ");

    metadata.append("\"readingStatus\" : ");
    metadata.append(getStringFromReadingState(this->readingStatus));
    metadata.append(" , ");

    metadata.append("\"timestamp\" : ");
    metadata.append(QString::number(this->timestamp));
    metadata.append(" }");

    finalMetadata.append(metadata);
    return finalMetadata;
}

QString BookInfo::getBookTypeString() const
{
    return sz_bookType[m_type];
}

bool BookInfo::hasExpired() const
{
    return (m_type == BOOKINFO_TYPE_SUBSCRIPTION || m_type == BOOKINFO_TYPE_LENDING) && m_expirationDate <= QDateTime::currentDateTimeUtc();
}

bool BookInfo::isCorrupted() const
{
    //qDebug() << Q_FUNC_INFO << "Type" << m_type << "Corrupted" << corrupted;

    /**
        In order to prevent that books got by susbscription can be read in
        previous versions (< 4.2.1) they are marked as corrupted
    */

    return (corrupted && m_type != BOOKINFO_TYPE_SUBSCRIPTION);
}

BookInfo::bookTypeEnum BookInfo::getTypeEnumFromString(const QString& type)
{
    for(int i = 0; i < (int)BOOKINFO_TYPE_COUNT; ++i)
    {
        if(sz_bookType[i] == type)
            return (BookInfo::bookTypeEnum)i;
    }
    return BOOKINFO_TYPE_PURCHASE;
}

void BookInfo::setPathAndThumbnail(const QString& _path)
{
    qDebug() << Q_FUNC_INFO;
    path = _path + isbn + "." + format;
    thumbnail = _path + "/.thumbnail/" + isbn + "." + format + ".jpg";
}

void BookInfo::resetReadingPeriod() const
{
    readingPeriod = 0;
}

bool BookInfo::inSD(const QString& privatePartition, const QString& publicPartition) const
{
    return !path.contains(privatePartition) && !path.contains(publicPartition);
}

bool BookInfo::existsFile() const
{
    return QFile::exists(path);
}

void BookInfo::appendPercent( const QString percent)
{
    qDebug() << Q_FUNC_INFO << "percent: " << percent;
    if(percent.isEmpty())
        return;

    if(!percentageList.size())
        percentageList = percent;
    else
        percentageList += QString("," + percent);

    qDebug() << Q_FUNC_INFO << "finished";
}

void BookInfo::clearPercentageList( ) const
{
    qDebug() << Q_FUNC_INFO;
    percentageList.clear();
}

void BookInfo::clearAllMetadata()
{
    clearLocations();
    lastTimeRead = QDateTime();
    lastReadLink = "";
    timestamp = 0;
    readingProgress = 0;
    readingPercentage = 0;
    clearPercentageList();
    resetReadingPeriod();
}

void BookInfo::addCollection(QString collection, double index)
{
    qDebug() << Q_FUNC_INFO << collection << index;

    QHash<QString, double>::iterator it = m_collections.find(collection);
    if(it == m_collections.end())
        m_collections.insert(collection, index);
    else
        it.value() = index;
}

void BookInfo::removeCollection(QString collection)
{
    qDebug() << Q_FUNC_INFO;
    QHash<QString, double>::iterator it = m_collections.find(collection);
    if(it != m_collections.end() && it.key() == collection)
    {
        it = m_collections.erase(it);
    }
}

const QHash<QString, double>&  BookInfo::getCollectionsList() const
{
    return m_collections;
}

BookInfo::readStateEnum BookInfo::getReadingStateFromString( QString state )
{
    qDebug() << Q_FUNC_INFO;
    if(state == "reading")
        return READING_BOOK;
    else if(state == "read")
        return READ_BOOK;
    else if(state == "unread")
        return NO_READ_BOOK;
    else
        return NONE;
}

QString BookInfo::getStringFromReadingState( readStateEnum state )
{
    qDebug() << Q_FUNC_INFO;
    switch(state)
    {
    case READING_BOOK:
        return "reading";
    case READ_BOOK:
        return "read";
    case NO_READ_BOOK: default:
        return "unread";
    }
}
