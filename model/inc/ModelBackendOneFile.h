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

#ifndef LIBRARYBACKENDONEFILE_H
#define LIBRARYBACKENDONEFILE_H

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "IModelBackend.h"
#include <QHash>

class BookInfo;
class BookLocation;

class ModelBackendOneFile : public IModelBackend
{
    Q_OBJECT
public:

    ModelBackendOneFile(const QString&);
    virtual ~ModelBackendOneFile();


    virtual void                init                ( TBooks* );
    virtual void                addDir              ( const QString& );
    virtual void                removeDir           ( const QString& );
    virtual void                sync                ( const QString& = "", qint64 serverTimestamp = 0 );
    virtual void                add                 ( const BookInfo* );
    virtual void                update              ( const BookInfo* );
    virtual void                archive             ( const BookInfo* );
    virtual void                unArchive           ( const BookInfo* );
    virtual void                remove              ( const BookInfo* );
    virtual void                close               ( const BookInfo* );
    QStringList                 getCollections      ();
    qint64                      getServerTimestamp  () const;

    void                        addTransaction      ( const QString& isbn, bool filed );
    void                        removeTransaction   ( const QString& isbn );
    void                        writeTransactions   ( );
    void                        clearTransactions   ( );
    bool                        writeNotesToFile    ( const BookInfo*, const QString&, const QString& );
    void                        addCollection       (QString collection);
    void                        removeCollection    (QString collection);
    QHash<QString, bool>*       m_transactions;

private:

    QHash<QString, bool>        m_pathsMount;
    TBooks*                     m_books;
    TBooks                      m_infos;

    bool                        load                ( const QStringList& booksPaths );
    bool                        booksToLoad         ( const QString&, QStringList& booksPaths );
    void                        loadArchivedBooks   ();
    bool                        loadMetaInfo        ( const QString& );
    BookInfo*                   loadBook            ( QXmlStreamReader& );
    BookLocation*               loadMark            ( QXmlStreamReader& );
    void                        save                ( const QString&, long long serverTimestamp = 0 ) const;
    void                        saveBookInfo        ( const BookInfo*, QXmlStreamWriter& ) const;
    void                        saveLocation        ( const BookLocation*, QXmlStreamWriter& ) const;
    void                        writeXmlFile        ( const BookInfo*, const QString& );
    void                        writeHTMLFile       ( const BookInfo*, const QString& );
    void                        exportXmlLocation   ( const BookLocation*, QXmlStreamWriter& ) const;
    void                        exportHtmlLocation  ( const BookLocation*, QString& ) ;
    BookInfo*                   loadDefaultInfo     ( const QString& );
    void                        messDir             ( const QString&, const int& );
    QStringList*                m_collections;
    QString                     _privatePartition;

signals:
    void                        loadingBooks        ( int );
    void                        downloadProgress    ( int );
    void                        loadFinished        ( );
};

#endif // LIBRARYBACKENDONEFILE_H
