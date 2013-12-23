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

#ifndef _BOOKINFO_H_
#define _BOOKINFO_H_

#include "BookLocation.h"

#include <QDateTime>
#include <QList>
#include <QString>
#include <QHash>
#include <QStringList>

class QByteArray;


enum BookInfoUpdateType {
    UPDATE_NONE =                    0,
    UPDATE_METADATA =         (1u << 1),
    UPDATE_READING_METADATA = (1u << 2),
    UPDATE_READING_POSITION = (1u << 3),
    UPDATE_TYPE =             (1u << 4),
    UPDATE_EXPIRATION =       (1u << 5),
    UPDATE_ARCHIVED =         (1u << 6),
    UPDATE_CLOSED =           (1u << 7),
    UPDATE_FULL = UPDATE_METADATA | UPDATE_READING_METADATA | UPDATE_READING_POSITION | UPDATE_TYPE | UPDATE_EXPIRATION | UPDATE_ARCHIVED | UPDATE_CLOSED
};

class BookInfo{

public:

    enum sortingStateEnum {
        BOOKINFO_SORT_TITLE = 0,
        BOOKINFO_SORT_AUTHOR = 1,
        BOOKINFO_SORT_ALL = 2
    };

    enum bookTypeEnum {
        BOOKINFO_TYPE_PURCHASE = 0,
        BOOKINFO_TYPE_DEMO,
        BOOKINFO_TYPE_SUBSCRIPTION,
        BOOKINFO_TYPE_LENDING,

        BOOKINFO_TYPE_COUNT
    };

    enum readStateEnum {
        NONE,
        NO_READ_BOOK,
        READING_BOOK,
        READ_BOOK
    };

    enum orientationEnum {
        ORI_PORTRAIT,
        ORI_LANDSCAPE
    };

    BookInfo(const QString& path = "");
    BookInfo(const BookInfo&);

    virtual ~BookInfo();

    QString         title;
    QString         isbn;
    QDateTime       lastTimeRead;
    QString         path;
    QString         author;
    QString         thumbnail;
    QString         coverUrl;
    QDateTime       publishTime;
    QDateTime       downloadTime;
    int             markCount;
    int             noteCount;
    int             hiliCount;

    int             pageCount;
    QString         lastReadLink;
    int             lastReadPage;
    double          readingProgress;
    double          readingPercentage;
    double          fontSize;
    int             pageMode;
    orientationEnum orientation;
    qint64          timestamp;

    QString         publisher;
    QString         synopsis;
    double          fileSize;
    QDateTime       syncDate;
    QString         storePrice;// TODO: Cambiar a float. Cuidado con punto o coma 9,95 o 9.95!!!
    QString         format;// TODO: Cambiar a enumerado

    bookTypeEnum    m_type;
    QDateTime       m_expirationDate;

    bool            corrupted;
    bool            m_archived;
    bool            isDRMFile;
    bool            dirty;
    int             totalReadingTime;
    mutable QString percentageList;
    readStateEnum   readingStatus;

    QHash<QString,double>                   locationsPosCache;
    QHash<QString,int>                      locationsPageCache;
    // IMPORTANT: Nobody can delete the BookLocations returned
    QList<const BookLocation*>*             getLocationList                 ( const QList<BookLocation::Type>& types = QList<BookLocation::Type>() ) const;
    int                                     getLocationListCount            ( const QList<BookLocation::Type>& types = QList<BookLocation::Type>() ) const;
    void                                    getLocationsAsString            ( QStringList& locations, const QList<BookLocation::Type>& types = QList<BookLocation::Type>()) const;

    // IMPORTANT: Nobody can delete the BookLocations returned
    const QHash<QString, BookLocation*>&    getLocations                    () const { return m_locations; }
    void                                    setLocations                    ( const QHash<QString, BookLocation*>& locations );
    void                                    setInfoFromCaches               ( );
    void                                    setCachesFromInfo               ( QHash<QString, BookLocation*> locations );
    // IMPORTANT: Nobody can delete the BookLocation returned
    BookLocation*                           getLocation                     ( const QString& _ref ) const;
    void                                    clearLocations                  ();
    void                                    clearAllMetadata                ();
    // IMPORTANT: BookInfo will now take care of the BookLocation's memory.
    void                                    addLocation                     ( const QString& key, BookLocation* location );
    void                                    removeLocation                  ( const QString& key );

    bool                                    hasMark                         () const { return markCount + noteCount + hiliCount > 0; }
    bool                                    operator==                      ( const BookInfo& other ) const { return path == other.path; }
    QString                                 toString                        () const;
    int                                     update                          ( const BookInfo*);
    bool                                    locationsHasChanged             ( const QHash<QString, BookLocation*>& locations );
    void                                    editNote                        ( const QString&, const QString&, qint64 lastUpdated);
    void                                    deleteLocation                  ( const QString& );
    static bool                             isBook                          ( const QString& );
    void                                    setMarktoRemove                 ( BookLocation* );
    void                                    setMarktoAdd                    ( BookLocation* );
    QByteArray                              parseJSON                       () const;
    void                                    resetReadingPeriod              () const;
    void                                    setReadingPeriod                ( long period ) { readingPeriod = period; }
    long                                    getReadingPeriod                () const { return readingPeriod; }

    bool                                    isSubscription                  () const { return m_type == BOOKINFO_TYPE_SUBSCRIPTION; }
    bool                                    isLending                       () const { return m_type == BOOKINFO_TYPE_LENDING; }

    QString                                 getBookTypeString               () const;
    bool                                    hasExpired                      () const;
    bool                                    isCorrupted                     () const;
    static BookInfo::bookTypeEnum           getTypeEnumFromString           ( const QString& type );
    void                                    setPathAndThumbnail             ( const QString& bookPath );

    void                                    setCSSFileList                  ( const QStringList& fileList ) const { m_cssFileList = fileList;}
    const QStringList&                      getCSSFileList                  () const { return m_cssFileList; }
    bool                                    inSD                            ( const QString& privatePartition, const QString& publicPartition) const;

    bool                                    existsFile                      () const;
    void                                    appendPercent                   ( const QString percent );
    void                                    clearPercentageList             ( ) const;
    QStringList&                            getCollectionsList              ( ) const;
    void                                    addCollection                   (QString);
    void                                    removeCollection                (QString);
    static readStateEnum                    getReadingStateFromString       ( QString );
    static QString                          getStringFromReadingState       ( readStateEnum );

private:
    QHash<QString, BookLocation*>           m_locations;
    mutable QStringList                     m_collections;
    mutable long                            readingPeriod;
    mutable QStringList                     m_cssFileList;
};

#endif
