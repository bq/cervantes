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

#ifndef MODEL_H_
#define MODEL_H_

#include <QObject>
#include <QString>
#include <QMap>
#include <QList>
#include <QFuture>
#include "BookInfo.h"

class IModelBackend;
class PowerManagerLock;
class QFileInfo;

bool pathLessThan( const BookInfo* b1, const BookInfo* b2 );
bool titleLessThan( const BookInfo* b1, const BookInfo* b2 );
bool authorLessThan( const BookInfo* b1, const BookInfo* b2 );
bool readingTimeAfterThan(const BookInfo* b1, const BookInfo* b2);
bool dateTimeAfterThan(const BookInfo* b1, const BookInfo* b2);
bool fileNameLessThan( const QFileInfo* f1, const QFileInfo* f2);
bool fileTimeLessThan( const QFileInfo* f1, const QFileInfo* f2);

class Model : public QObject{

    Q_OBJECT

public:

    enum SortType {
        LIBRARY_SORT_PATH,
        LIBRARY_SORT_TITLE,
        LIBRARY_SORT_AUTHOR,
        LIBRARY_SORT_READING_TIME,
        LIBRARY_NO_SORT
    };

    enum OpeningState {
        BOOK_OPEN,
        BOOK_CLOSE
    };

                                    Model                                   ( IModelBackend* m_backend );
    virtual                         ~Model                                  ();

    void                            addDir                                  ( const QString& );
    void                            removeDir                               ( const QString& );
    int                             getCount                                ( const QString& );
    const BookInfo*                 getBookInfo                             ( const QString& );
    void                            addBook                                 ( const BookInfo* );
    void                            updateBook                              ( const BookInfo* );
    void                            removeBook                              ( const BookInfo* );
    void                            archiveBook                             ( const BookInfo* );
    void                            unArchiveBook                           ( const BookInfo* );
    void                            closeBook                               ( const BookInfo* );
    void                            changeReadState                         ( const BookInfo* bookInfo, BookInfo::readStateEnum state);
    void                            syncModel                               ( const QString& = "", qint64 serverTimestamp = 0 );
    void                            getBooks                                ( QList<const BookInfo*>& books, SortType, bool recursive, const QString& path = "", bool bIncludeArchivedBooks = true);
    void                            getBooksByISBN                          ( QHash<QString, const BookInfo*>& books, bool recursive, const QString& path = "", bool bIncludeArchivedBooks = true );

    void                            getAllBooks                             ( QList<const BookInfo*>& books ) const;
    void                            getBooksInPath                          ( QList<const BookInfo*>& books, const QString& path ) const;
    void                            getBooksInCollection                    ( QList<const BookInfo *>& books, const QString& collection ) const;
    void                            getStoreArchivedBooks                   ( QList<const BookInfo*>& books ) const;
    void                            getPremiumArchivedBooks                 ( QList<const BookInfo*>& books ) const;
    void                            getNowReadingBooks                      ( QList<const BookInfo*>& books, const QString& path ) const;
    void                            getAlreadyReadBooks                     ( QList<const BookInfo*>& books, const QString& path ) const;
    void                            getNewBooks                             ( QList<const BookInfo*>& books, const QString& path ) const;
    void                            getSampleBooks                          ( QList<const BookInfo*>& books ) const;
    void                            getPurchasedBooks                       ( QList<const BookInfo*>& books, const QString& path ) const;
    void                            getSubscriptionBooks                    ( QList<const BookInfo*>& books ) const;
    void                            getSubscriptionExpiredBooks             ( QList<const BookInfo *>& books, const QString& path ) const;
    qint64                          getServerTimestamp                      () ;
    int                             getAllBooksCount                        () const;
    int                             getBooksInPathCount                     ( const QString& path ) const;
    int                             getBooksInCollectionCount               ( const QString& collection ) const;
    int                             getArchivedBooksCount                   () const;
    int                             getNowReadingBooksCount                 ( const QString& path ) const;
    int                             getSampleBooksCount                     () const;
    int                             getSubscriptionBooksCount               () const;
    void                            getBooksCounters                        ( const QString& privatePath, int& all, int& allActive, int& allAlreadyRead, int& alreadyRead, int& allNew, int& storeNew, int& storeAll, int& storeActive, int& storeSample, int& storePurchased, int& storeSubscription, int& storeStoreArchived, int& storePremiumArchived );

    void                            clearTransactions                       ();
    void                            addTransaction                          ( const QString& isbn, bool filed );
    void                            removeTransaction                       ( const QString& isbn );
    void                            loadDefaultInfo                         ( const QString& path );
    bool                            writeNotesToFile                        ( const BookInfo*, const QString&, const QString& );
    QStringList                     getCollections                          ();
    void                            addCollection                           (QString collection);
    const BookInfo*                 bookInPath                              ( const QString& , const QString& );
    bool                            bookListEquals                          ( QList<const BookInfo*> , QList<const BookInfo*> );
    void                            removeCollection                        (QString collection);

protected slots:
    void                            syncModelConcurrent                     ( const QString& = "", qint64 serverTimestamp = 0 );

private:
    typedef QMap<QString, BookInfo*> TBooks;

    TBooks                          m_books;// QMap < bookInfo->path, bookInfo >
    IModelBackend *                 m_backend;
    QFuture<void>                   m_futureThumbnail;

signals:
    void                            modelChanged                            ( QString, int );
    void                            syncStart                               ();
    void                            syncEnd                                 ();
    void                            transactionsSent                        ();
    void                            userLibraryReceived                     ();
    void                            syncBookFailed                          ();
    void                            syncBookSucceeded                       ();
    void                            syncCanceled                            ();
    void                            loadingBooks                            (int);
    void                            downloadProgress                        (int);
    void                            loadFinished                            ();

};

#endif
