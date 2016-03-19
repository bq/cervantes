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

#ifndef LIBRARYGRIDVIEWER_H
#define LIBRARYGRIDVIEWER_H

#include "GestureWidget.h"

#include <QDateTime>

// Predeclarations
class Library;
class LibraryGridViewerItem;
class LibraryPageHandler;
class LibraryActionsMenu;

class LibraryGridViewer : public GestureWidget
{
    Q_OBJECT

public:
    LibraryGridViewer(QWidget* parent);

    void                                setLibrary                          ( Library* library );

    virtual void                        init                                ();
    virtual void                        done                                ();
    virtual LibraryPageHandler*         getPageHandler                      () = 0;
    virtual int                         getItemsPerPage                     () = 0;

    virtual void                        start                               ( int startPage = 0 );
    virtual void                        paint                               ();
    virtual void                        pause                               ();
    virtual void                        stop                                ();
    void                                setCollections                      ( QStringList );

public slots:
    virtual void                        resume                              ();
    bool                                clearActionsMenu                    ();

signals:
    void                                itemClicked                         ( const QString& path );
    void                                itemLongPress                       ( const QString& path );
    void                                copyFile                            ( const QString& path );
    void                                removeFile                          ( const QString& path );

protected slots:
    void                                firstPage                           ();
    void                                lastPage                            ();
    void                                nextPage                            ();
    void                                previousPage                        ();
    void                                goLibraryPage                       ( int page );
    void                                fileLongPress                       ( const QString& path);
    void                                hideElements                        ();

protected:
    Library*                            m_library;
    LibraryActionsMenu*                 m_actionsMenu;

    void                                setupPager                          ( int librarySize, int startPage = 0 );
    virtual int                         getItemsPerPageOffset               () { return 0; }
    virtual void                        getBookItems                        ( LibraryGridViewerItem* books[] ) = 0;

    virtual void                        paintBooks                          ();
    void                                paintFiles                          ();
    void                                paintCollections                    ();

    QString                             getDateStyled                       ( const QDateTime&, bool );
    QStringList                         m_collectionsList;
};

#endif // LIBRARYGRIDVIEWER_H
