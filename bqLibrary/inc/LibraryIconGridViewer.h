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

#ifndef LIBRARYICONGRIDVIEWER_H
#define LIBRARYICONGRIDVIEWER_H

#include "LibraryGridViewer.h"
#include "ui_LibraryIconGridViewer.h"

#include <QFuture>

// Predeclarations
class PowerManagerLock;

class LibraryIconGridViewer : public LibraryGridViewer, protected Ui::LibraryIconGridViewer
{
    Q_OBJECT

public:
                                        LibraryIconGridViewer               ( QWidget* parent = 0 );
    virtual                             ~LibraryIconGridViewer              ();

    virtual void                        resume                              ();
    virtual void                        pause                               ();

    void                                pauseThumbnailGeneration            ();
    void                                resumeThumbnailGeneration           ();

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                        paintEvent                          ( QPaintEvent* );
    virtual bool                        event                               ( QEvent* event );

    virtual LibraryPageHandler*         getPageHandler                      ();
    virtual int                         getItemsPerPage                     ();
    virtual void                        getBookItems                        ( LibraryGridViewerItem* books[] );

protected slots:
    void                                runCoverPage                        ( QObject* receiver, const QString& bookInfoPath, LibraryIconGridViewerItem* itemToUpdate, int gridPage );
    void                                runGetThumbnail                     ();

private:
    QFuture<void>                       m_futureThumbnail;
    bool                                m_isThumbnailGenerationStopped;
    void                                waitForThumbnailRunner              ();

    PowerManagerLock*                   m_powerLock;
};

#endif // LIBRARYICONGRIDVIEWER_H
