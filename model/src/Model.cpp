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

#include "Model.h"

#include <QtAlgorithms>
#include "IModelBackend.h"
#include "ModelBackendOneFile.h"
#include "BookInfo.h"
#include "bqUtils.h"
#include <QDir>
#include <QDebug>
#include <QList>
#include <QDateTime>

Model::Model(IModelBackend *_backend)
{
    m_backend = _backend;
    m_backend->init(&m_books);
    connect(m_backend, SIGNAL(modelChanged(QString, int)), this, SIGNAL(modelChanged(QString, int)));
    connect(m_backend, SIGNAL(loadingBooks(int)), this, SIGNAL(loadingBooks(int)));
    connect(m_backend, SIGNAL(downloadProgress(int)), this, SIGNAL(downloadProgress(int)));
    connect(m_backend, SIGNAL(loadFinished()), this, SIGNAL(loadFinished()));
}

Model::~Model()
{
    m_futureThumbnail.waitForFinished();

    // NOTE: the backend takes care of the deletion of books;
    delete m_backend;
    m_backend = NULL;
}

void Model::addDir (const QString& path)
{
    qDebug() << Q_FUNC_INFO << path;
    m_backend->addDir (path);
}

void Model::removeDir (const QString& path)
{
    qDebug() << Q_FUNC_INFO << path;
    m_backend->removeDir (path);
}

int Model::getCount (const QString& path) {
    if (path == NULL || path.isEmpty() || path == "/")
        return m_books.count();

    int count = 0;
    TBooks::const_iterator i = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    while (i != itEnd) {
        if (i.key().contains(path))
            ++count;
        ++i;
    }
    return count;
}

const BookInfo* Model::getBookInfo (const QString& path) {
    return m_books.value(path, NULL);
}

void Model::addBook (const BookInfo *book)
{
    qDebug() << Q_FUNC_INFO << book->isbn;
    m_backend->add(book);
}

void Model::updateBook (const BookInfo *book)
{
    qDebug() << Q_FUNC_INFO << book->isbn;
    m_backend->update(book);
}

void Model::removeBook (const BookInfo* bookInfo)
{
    qDebug() << Q_FUNC_INFO << " Removing " << bookInfo->path;
    // Remove the book
    m_backend->remove(bookInfo);
}

void Model::archiveBook (const BookInfo* bookInfo)
{
    qDebug() << Q_FUNC_INFO;

    BookInfo* modelBook = new BookInfo(*bookInfo);
    modelBook->m_archived = true;
    QFile::remove(modelBook->path);
    m_backend->archive(modelBook);
    delete modelBook;
}

void Model::unArchiveBook (const BookInfo* bookInfo)
{
    qDebug() << Q_FUNC_INFO;
    if(!bookInfo) return;
    BookInfo* modelBook = new BookInfo(*bookInfo);
    modelBook->m_archived = false;
    m_backend->unArchive(modelBook);
    delete modelBook;
}

void Model::closeBook( const BookInfo* bookInfo )
{
    qDebug() << Q_FUNC_INFO;

    BookInfo* modelBook = new BookInfo(*bookInfo);
    modelBook->lastTimeRead = QDateTime();
    m_backend->close(modelBook);
    delete modelBook;
}

void Model::changeReadState( const BookInfo* bookInfo, BookInfo::readStateEnum state)
{
    qDebug() << Q_FUNC_INFO << "state: " << state;  
    BookInfo* modelBook = new BookInfo(*bookInfo);
    modelBook->readingStatus = state;
    modelBook->timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    modelBook->lastTimeRead = QDateTime();
    m_backend->close(modelBook);
    delete modelBook;
}

void Model::syncModel (const QString& path, qint64 serverTimestamp)
{
    qDebug() << Q_FUNC_INFO << "Waiting to launch sync.";

    if(serverTimestamp == 0)
        serverTimestamp = getServerTimestamp();

    m_futureThumbnail.waitForFinished();
    m_backend->sync(path, serverTimestamp);
    qDebug() << Q_FUNC_INFO << "End.";
}

void Model::syncModelConcurrent (const QString& path, qint64 serverTimestamp)
{
    qDebug() << Q_FUNC_INFO << "Start";
    if(serverTimestamp == 0)
        serverTimestamp = getServerTimestamp();

    m_backend->sync(path, serverTimestamp);
    qDebug() << Q_FUNC_INFO << "End";
}

bool pathLessThan(const BookInfo *b1, const BookInfo *b2)
{
//  return b1->path.toLower() < b2->path.toLower();
    return b1->path.trimmed().toLower().localeAwareCompare(b2->path.trimmed().toLower()) < 0;
}

bool titleLessThan(const BookInfo *b1, const BookInfo *b2)
{
//  return b1->title.toLower() < b2->title.toLower();
    if(b1->title.trimmed().isEmpty() && b2->title.trimmed().isEmpty())
        return pathLessThan(b1, b2);
    else
        return bqUtils::simplify(b1->title.trimmed()).toLower().localeAwareCompare(bqUtils::simplify(b2->title.trimmed().toLower())) < 0;
}

bool authorLessThan(const BookInfo *b1, const BookInfo *b2)
{
    if(b1->author.trimmed().toLower() == b2->author.trimmed().toLower())
        return titleLessThan(b1, b2);
    else
        return bqUtils::simplify(b1->author.trimmed()).toLower().localeAwareCompare(bqUtils::simplify(b2->author.trimmed().toLower())) < 0;
}

bool readingTimeAfterThan(const BookInfo *b1, const BookInfo *b2)
{
    if(b1->lastTimeRead.isNull() && b2->lastTimeRead.isNull())
        return dateTimeAfterThan(b1, b2);
    else if(!b1->lastTimeRead.isNull() && b2->lastTimeRead.isNull())
        return true;
    else if(b1->lastTimeRead.isNull() && !b2->lastTimeRead.isNull())
        return false;
    else
        return b1->lastTimeRead > b2->lastTimeRead;
}

bool dateTimeAfterThan(const BookInfo *b1, const BookInfo *b2)
{
    if(b1->downloadTime.isNull() && b2->downloadTime.isNull())
        return titleLessThan(b1, b2);
    else if(!b1->downloadTime.isNull() && b2->downloadTime.isNull())
        return true;
    else if(b1->downloadTime.isNull() && !b2->downloadTime.isNull())
        return false;
    else
        return b1->downloadTime > b2->downloadTime;
}

bool fileNameLessThan( const QFileInfo* f1, const QFileInfo* f2)
{
    if(f1->isDir() && f2->isDir())
        return f1->completeBaseName().toLower() < f2->completeBaseName().toLower();
    else if(!f1->isDir() && f2->isDir())
        return false;
    else if (f1->isDir() && !f2->isDir())
        return true;
    else
        return f1->completeBaseName().toLower() < f2->completeBaseName().toLower();
}

bool fileTimeLessThan( const QFileInfo* f1, const QFileInfo* f2)
{
    if(f1->isDir() && f2->isDir())
        return f1->lastModified() < f2->lastModified();
    else if(!f1->isDir() && f2->isDir())
        return false;
    else if (f1->isDir() && !f2->isDir())
        return true;
    else
        return f1->lastModified() < f2->lastModified();
}

bool stringLessThan( QString s1, QString s2)
{
     return s1.toLower() < s2.toLower();
}

void Model::getBooks (QList<const BookInfo*>& orderedList, SortType type, bool recursive, const QString& path, bool bIncludeArchivedBooks) {
        TBooks::const_iterator i = m_books.constBegin();
        TBooks::const_iterator itEnd = m_books.constEnd();
        while (i != itEnd) {
            const BookInfo* bookInfo = i.value();
            if(path.isEmpty() || (recursive && bookInfo->path.contains(path)) || (!recursive && QFileInfo(bookInfo->path).dir() == QDir(path)) || bookInfo->m_archived )
            {
                if((!bookInfo->m_archived || bIncludeArchivedBooks) && !bookInfo->hasExpired())
                    orderedList.prepend(bookInfo);
            }
            ++i;
        }
        switch (type) {
        case LIBRARY_SORT_PATH:
                qSort(orderedList.begin(), orderedList.end(), pathLessThan);
                break;
        case LIBRARY_SORT_TITLE:
                qSort(orderedList.begin(), orderedList.end(), titleLessThan);
                break;
        case LIBRARY_SORT_AUTHOR:
                qSort(orderedList.begin(), orderedList.end(), authorLessThan);
                break;
        case LIBRARY_SORT_READING_TIME:
                qSort(orderedList.begin(), orderedList.end(), readingTimeAfterThan);
                break;
        default:
            break;
        }
}

void Model::getBooksByISBN( QHash<QString, const BookInfo*>& booksHash, bool recursive, const QString& path, bool bIncludeArchivedBooks )
{
    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();
        if(path.isEmpty() || (recursive && bookInfo->path.contains(path)) || (!recursive && QFileInfo(bookInfo->path).dir() == QDir(path)) || bookInfo->m_archived )
        {
            if((!bookInfo->m_archived || bIncludeArchivedBooks))
            {
                booksHash.insert(bookInfo->isbn, bookInfo);
            }
        }
    }
}

void Model::getAllBooks( QList<const BookInfo *>& books ) const
{
    qDebug() << Q_FUNC_INFO << "begin";

    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        // Just for debug
        const BookInfo* bookInfo = it.value();
        if(!bookInfo->m_archived && !bookInfo->hasExpired())// NOTE: we are extracting the books not archived)
            books.append(bookInfo);
    }

    qDebug() << Q_FUNC_INFO << "end";
}

void Model::getBooksInPath( QList<const BookInfo *>& books, const QString& path ) const
{
    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();
        if(bookInfo->path.contains(path) && !bookInfo->m_archived && !bookInfo->hasExpired())// NOTE: we are extracting the books not archived
            books.append(bookInfo);
    }
}

void Model::getBooksInCollection( QList<const BookInfo *>& books, const QString& collection ) const
{
    qDebug() << Q_FUNC_INFO << collection;
    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();
        if(!bookInfo->m_archived && !bookInfo->hasExpired())
        {
            const QHash<QString, double>& bookCollections = bookInfo->getCollectionsList();
            if(bookCollections.find(collection) != bookCollections.end())
                {
                    qDebug() << Q_FUNC_INFO << "Add book to list" << bookInfo->title;
                    books.append(bookInfo);
            }
        }
    }
}

void Model::getStoreArchivedBooks( QList<const BookInfo *>& books ) const
{
    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();
        if(bookInfo->m_archived && !bookInfo->hasExpired() && bookInfo->m_type == BookInfo::BOOKINFO_TYPE_PURCHASE)
            books.append(bookInfo);
    }
}

void Model::getPremiumArchivedBooks( QList<const BookInfo *>& books ) const
{
    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();
        if(bookInfo->m_archived && !bookInfo->hasExpired() && bookInfo->m_type == BookInfo::BOOKINFO_TYPE_SUBSCRIPTION)
            books.append(bookInfo);
    }
}

void Model::getNewBooks( QList<const BookInfo *>& books, const QString& path ) const
{
    qDebug() << Q_FUNC_INFO;

    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();
        if(bookInfo->readingStatus == BookInfo::NO_READ_BOOK && !bookInfo->m_archived && !bookInfo->hasExpired())
        {
            if(path.isEmpty() || bookInfo->path.contains(path))
                books.append(bookInfo);
        }
    }
}


void Model::getNowReadingBooks( QList<const BookInfo *>& books, const QString& path ) const
{
    qDebug() << Q_FUNC_INFO << "begin";

    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();

        if(bookInfo->readingStatus == BookInfo::READING_BOOK && !bookInfo->m_archived && !bookInfo->hasExpired())
            if(path.isEmpty() || bookInfo->path.contains(path))
                books.append(bookInfo);
    }
}

void Model::getAlreadyReadBooks( QList<const BookInfo *>& books, const QString& path ) const
{
    qDebug() << Q_FUNC_INFO << "begin";

    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();

        if(bookInfo->readingStatus == BookInfo::READ_BOOK && !bookInfo->m_archived && !bookInfo->hasExpired())
            if(path.isEmpty() || bookInfo->path.contains(path))
                books.append(bookInfo);
    }
}

void Model::getSampleBooks( QList<const BookInfo *>& books ) const
{
    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();
        if(bookInfo->m_type == BookInfo::BOOKINFO_TYPE_DEMO && !bookInfo->hasExpired())
            books.append(bookInfo);
    }
}

void Model::getPurchasedBooks( QList<const BookInfo *>& books, const QString& path ) const
{
    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();
        if(bookInfo->path.contains(path) && bookInfo->m_type == BookInfo::BOOKINFO_TYPE_PURCHASE && !bookInfo->hasExpired() && !bookInfo->m_archived)
            books.append(bookInfo);
    }

    qDebug() << Q_FUNC_INFO << "end";

}

void Model::getSubscriptionBooks( QList<const BookInfo *>& books ) const
{
    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();
        if(bookInfo->m_type == BookInfo::BOOKINFO_TYPE_SUBSCRIPTION && !bookInfo->hasExpired() && !bookInfo->m_archived)// NOTE: we are extracting the books not archived)
            books.append(bookInfo);
    }

    qDebug() << Q_FUNC_INFO << "end";

}

void Model::getSubscriptionExpiredBooks( QList<const BookInfo *>& books, const QString& path ) const
{
    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();
        if(bookInfo->path.contains(path) && bookInfo->hasExpired() && bookInfo->m_type == BookInfo::BOOKINFO_TYPE_SUBSCRIPTION)
            books.append(bookInfo);
    }
}

int Model::getAllBooksCount() const
{
    int count = 0;
    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();
        if(!bookInfo->m_archived && !bookInfo->hasExpired())// NOTE: we are extracting the books not archived)
            ++count;
    }
    return count;
}

int Model::getBooksInPathCount( const QString& path ) const
{
    int count = 0;
    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();
        if(bookInfo->path.contains(path) && !bookInfo->m_archived && !bookInfo->hasExpired())// NOTE: we are extracting the books not archived)
            ++count;
    }
    return count;
}

int Model::getBooksInCollectionCount( const QString& collection ) const
{
    int count = 0;
    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();
        const QHash<QString, double>& collections = bookInfo->getCollectionsList();
        if(collections.find(collection) != collections.end() && !bookInfo->m_archived && !bookInfo->hasExpired())
                ++count;
        }
    return count;
}

int Model::getArchivedBooksCount() const
{
    int count = 0;
    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();
        if(bookInfo->m_archived && !bookInfo->hasExpired())
            ++count;
    }
    return count;
}

int Model::getNowReadingBooksCount( const QString& path ) const
{
    int count = 0;
    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();
        if(bookInfo->path.contains(path) && bookInfo->readingStatus == BookInfo::READING_BOOK && !bookInfo->m_archived && !bookInfo->hasExpired())// NOTE: we are extracting the books not archived)
            ++count;
    }
    return count;
}

int Model::getSampleBooksCount() const
{
    int count = 0;
    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();
        if(bookInfo->m_type == BookInfo::BOOKINFO_TYPE_DEMO)
            ++count;
    }
    return count;
}

int Model::getSubscriptionBooksCount() const
{
    int count = 0;
    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();
        if(bookInfo->m_type == BookInfo::BOOKINFO_TYPE_SUBSCRIPTION && !bookInfo->hasExpired())// NOTE: we are extracting the books not archived)
            ++count;
    }
    return count;
}

void Model::getBooksCounters( const QString& privatePath, int& all, int& allActive, int& allAlreadyRead, int& alreadyRead, int& allNew, int& storeNew, int& storeAll, int& storeActive, int& storeSample, int& storePurchased, int& storeSubscription, int& storeStoreArchived, int& storePremiumArchived )
{
    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();
    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();
        if(!bookInfo->hasExpired())
        {
            if(!bookInfo->m_archived )
            {
                ++all;
                switch(bookInfo->readingStatus)
                {
                case BookInfo::NO_READ_BOOK:
                    ++allNew;
                break;
                case BookInfo::READING_BOOK:
                    ++allActive;
                break;
                case BookInfo::READ_BOOK:
                    ++allAlreadyRead;
                break;
                }

                if(bookInfo->path.contains(privatePath))
                {
                    ++storeAll;
                    if(bookInfo->m_type == BookInfo::BOOKINFO_TYPE_SUBSCRIPTION)
                        ++storeSubscription;
                    if(bookInfo->m_type == BookInfo::BOOKINFO_TYPE_DEMO)
                        ++storeSample;
                    if(bookInfo->m_type == BookInfo::BOOKINFO_TYPE_PURCHASE)
                        ++storePurchased;
                    if(bookInfo->readingStatus == BookInfo::NO_READ_BOOK)
                        ++storeNew;
                    if(bookInfo->readingStatus == BookInfo::READING_BOOK)
                        ++storeActive;
                    if(bookInfo->readingStatus == BookInfo::READ_BOOK)
                        ++alreadyRead;
                }
            }
            else
            {
                if(bookInfo->path.contains(privatePath))
                {
                    if(bookInfo->m_type == BookInfo::BOOKINFO_TYPE_SUBSCRIPTION)
                        ++storePremiumArchived;
                    else if(bookInfo->m_type == BookInfo::BOOKINFO_TYPE_PURCHASE)
                        ++storeStoreArchived;
                }
            }
        }
    }
}

qint64 Model::getServerTimestamp()
{
    qDebug() <<Q_FUNC_INFO;
    return m_backend->getServerTimestamp();
}

void Model::clearTransactions()
{
    qDebug() <<Q_FUNC_INFO;
    m_backend->clearTransactions();
}

void Model::addTransaction(const QString& isbn, bool filed)
{
    qDebug() <<Q_FUNC_INFO;
    m_backend->addTransaction(isbn, filed);
}

void Model::removeTransaction(const QString& isbn)
{
    qDebug() <<Q_FUNC_INFO;
    m_backend->removeTransaction(isbn);
}

void Model::loadDefaultInfo(const QString& path)
{
    qDebug() << Q_FUNC_INFO << "Path == " << path;
    if(BookInfo::isBook(path)) {
        BookInfo* bookInfo = m_backend->loadDefaultInfo(path);
        addBook(bookInfo);
    }
}

bool Model::writeNotesToFile(const BookInfo* bookInfo, const QString& fileType, const QString& destination)
{
    qDebug() << Q_FUNC_INFO;
    return m_backend->writeNotesToFile(bookInfo, fileType, destination);
}

QStringList Model::getCollections()
{
    QStringList collections = m_backend->getCollections();
    qSort(collections.begin(), collections.end(), stringLessThan);
    return collections;
}

void Model::addCollection(QString collection)
{
    qDebug() << Q_FUNC_INFO;
    m_backend->addCollection(collection);
}

void Model::removeCollection(QString collection)
{
    qDebug() << Q_FUNC_INFO;
    m_backend->removeCollection(collection);
}

const BookInfo* Model::bookInPath( const QString& _isbn, const QString& path)
{
    TBooks::const_iterator it = m_books.constBegin();
    TBooks::const_iterator itEnd = m_books.constEnd();

    for(; it != itEnd; ++it)
    {
        const BookInfo* bookInfo = it.value();
        if(bookInfo->isbn == _isbn && bookInfo->path.contains(path) && !bookInfo->m_archived)
            return bookInfo;
    }

    return NULL;
}

bool Model::bookListEquals(QList<const BookInfo*> list1, QList<const BookInfo*> list2)
{
    if(list1.size() != list2.size())
        return false;

    QList<const BookInfo*>::const_iterator itList1 = list1.begin();
    QList<const BookInfo*>::const_iterator itEndList1 = list1.end();

    while(itList1 != itEndList1)
    {
        bool exists = false;
        QList<const BookInfo*>::const_iterator itList2 = list2.begin();
        QList<const BookInfo*>::const_iterator itEndList2 = list2.end();
        while(itList2 != itEndList2 && exists)
        {
            if((*itList1)->isbn == (*itList2)->isbn)
            {
                exists = true;
                break;
            }
            itList2++;
        }

        if(!exists)
            return false;

        itList1++;
    }

    return true;
}
