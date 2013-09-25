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

#ifndef LIBRARYICONGRIDVIEWERITEM_H
#define LIBRARYICONGRIDVIEWERITEM_H

#include "LibraryGridViewerItem.h"
#include "ui_LibraryIconGridViewerItem.h"

// Predeclaration
class BookInfo;
class QFileInfo;

class LibraryIconGridViewerItem : public LibraryGridViewerItem, protected Ui::LibraryIconGridViewerItem
{
    Q_OBJECT

public:
                                        LibraryIconGridViewerItem           ( QWidget* parent = 0 );
    virtual                             ~LibraryIconGridViewerItem          ();

    virtual void                        setBook                             ( const BookInfo* book, Library::ELibraryFilterMode filter);
    virtual void                        setFile                             ( const QFileInfo* file );
    virtual void                        setArrow                            ( const QString& path );
    virtual void                        setCollection                       ( const QString&, int ) {}

    virtual void                        select                              ();
    virtual void                        unselect                            ();

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                        paintEvent                          ( QPaintEvent* );
    void                                setThumbnailElements                ( bool visible);
};

#endif // LIBRARYICONGRIDVIEWERITEM_H
