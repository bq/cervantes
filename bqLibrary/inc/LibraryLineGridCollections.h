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

#ifndef LIBRARYLINEGRIDCOLLECTIONS_H
#define LIBRARYLINEGRIDCOLLECTIONS_H

#include "ui_LibraryLineGridCollections.h"
#include "LibraryGridViewer.h"

class QWidget;

class LibraryLineGridCollections : public LibraryGridViewer , protected Ui::LibraryLineGridCollections
{
    Q_OBJECT

public:

                                                LibraryLineGridCollections          ( QWidget* parent = 0 );
    virtual                                     ~LibraryLineGridCollections         ();

    void                                        setup                               (QStringList);

signals:

    void                                        deleteCollection                        (const QString&);
    void                                        editCollection                          (const QString&);

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                                paintEvent                          ( QPaintEvent* );
    virtual LibraryPageHandler*                 getPageHandler                      ();
    virtual int                                 getItemsPerPage                     ();
    virtual void                                getBookItems                        ( LibraryGridViewerItem* books[] );

};
#endif // LIBRARYLINEGRIDCOLLECTIONS_H
