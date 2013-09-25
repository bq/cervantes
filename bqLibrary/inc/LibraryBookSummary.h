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

#ifndef LIBRARYBOOKSUMMARY_H
#define LIBRARYBOOKSUMMARY_H

#include "GestureWidget.h"
#include "ui_LibraryBookSummary.h"
#include "BookInfo.h"

// Predeclarations
class LibraryBookListActions;

class LibraryBookSummary : public GestureWidget, protected Ui::LibraryBookSummary
{
    Q_OBJECT

public:
                                        LibraryBookSummary                  ( QWidget* parent );
    virtual                             ~LibraryBookSummary                 ();

    void                                setBook                             ( const BookInfo* book );
    void                                hideListActionsMenu                 ( );
    void                                setActionsBtnText                   ( BookInfo::readStateEnum );

signals:
    void                                hideMe                              ();
    void                                buyBook                             ( const BookInfo* );
    void                                archiveBook                         ( const BookInfo* );
    void                                unarchiveBook                       ( const BookInfo* );
    void                                removeBook                          ( const BookInfo* );
    void                                copyBook                            ( const QString& );
    void                                exportNotes                         ( const QString );
    void                                changeReadState                     ( const BookInfo*, BookInfo::readStateEnum );

public slots:
    void                                close                               ();

protected slots:
    void                                buyBookClicked                      ();
    void                                moreActionsClicked                  ();
    void                                archiveBook                         ();

    void                                removeBook                          ();
    void                                copyBook                            ();
    void                                exportNotes                         ();

    void                                synopsisDown                        ();
    void                                synopsisUp                          ();
    void                                setupPagination                     (int);
    void                                changeReadState                     (int);

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                        paintEvent                          ( QPaintEvent* );
    virtual void                        mouseReleaseEvent                   ( QMouseEvent* event );

    void                                setThumbnailElements                ( bool visible);

    const BookInfo*                     m_bookInfo;
    LibraryBookListActions*             m_bookListActions;
    QScrollBar*                         vbar;

    enum ELibraryBookSummaryAction
    {
        DELETE,
        ARCHIVE,
        UNARCHIVE
    };
};

#endif // LIBRARYBOOKSUMMARY_H
