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

#ifndef LIBRARYREADINGICONGRIDVIEWER_H
#define LIBRARYREADINGICONGRIDVIEWER_H

#include "LibraryGridViewer.h"
#include "ui_LibraryReadingIconGridViewer.h"

// Predeclarations
class QWidget;

class LibraryReadingIconGridViewer : public LibraryGridViewer, protected Ui::LibraryReadingIconGridViewer
{
    Q_OBJECT

public:
                                        LibraryReadingIconGridViewer        ( QWidget* parent = 0 );
    virtual                             ~LibraryReadingIconGridViewer       ();

private slots:
    void                                openMostRecent                      ();
    void                                showSummary                         ();
protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                        paintEvent                          ( QPaintEvent* );

    virtual LibraryPageHandler*         getPageHandler                      ();
    virtual int                         getItemsPerPage                     ();
    virtual int                         getItemsPerPageOffset               () { return 1; }
    virtual void                        getBookItems                        ( LibraryGridViewerItem* books[] );

    virtual void                        paintBooks                          ();
};

#endif // LIBRARYREADINGICONGRIDVIEWER_H
