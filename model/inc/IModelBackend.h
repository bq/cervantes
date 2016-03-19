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

#ifndef IMODELBACKEND_H
#define IMODELBACKEND_H

#include <QObject>
#include <QMap>
#include <QHash>
#include <QString>
#define MODEL_VERSION 2

class BookInfo;

class IModelBackend : public QObject
{
    Q_OBJECT

protected:
    typedef QMap<QString, BookInfo*> TBooks;

public:
    virtual void                   init                 ( TBooks*) = 0;
    virtual void                   addDir               ( const QString& ) = 0;
    virtual void                   removeDir            ( const QString& ) = 0;
    virtual void                   sync                 ( const QString& = "", qint64 serverTimestamp = 0 ) = 0;
    virtual void                   add                  ( const BookInfo* ) = 0;
    virtual void                   update               ( const BookInfo* ) = 0;
    virtual void                   archive              ( const BookInfo* ) = 0;
    virtual void                   unArchive            ( const BookInfo* ) = 0;
    virtual void                   remove               ( const BookInfo* ) = 0;
    virtual void                   close                ( const BookInfo* ) = 0;
    virtual qint64                 getServerTimestamp   () const = 0;
    virtual void                   clearTransactions    () = 0;
    virtual void                   addTransaction       ( const QString& isbn, bool filed ) = 0;
    virtual void                   removeTransaction    ( const QString& isbn) = 0;
    virtual BookInfo*              loadDefaultInfo      ( const QString& ) = 0;
    virtual bool                   writeNotesToFile     ( const BookInfo*, const QString&, const QString& ) = 0;
    virtual QStringList            getCollections       () = 0;
    virtual void                   addCollection        (QString) = 0;
    virtual void                   removeCollection     (QString) = 0;


signals:
    void                           modelChanged         ( const QString&, const int&);
    void                           addedBook            ();

protected:
    unsigned int                   m_fileModelVersion;

};

#endif // IMODELBACKEND_H
