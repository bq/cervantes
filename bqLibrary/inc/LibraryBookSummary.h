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

#include <QWidget>
#include "ui_LibraryBookSummary.h"
#include "BookInfo.h"

// Predeclarations
class LibraryBookListActions;
class LibraryCollectionLayer;

class LibraryBookSummary : public QWidget, protected Ui::LibraryBookSummary
{
    Q_OBJECT

public:
                                        LibraryBookSummary                  ( QWidget* parent );
    virtual                             ~LibraryBookSummary                 ();

    void                                setBook                             ( const BookInfo* book, int currentBook, int totalBooks );
    bool                                hideElements                        ( );
    void                                setActionsBtnText                   ( BookInfo::readStateEnum );
    void                                setCollectionLayerBtnText           (QStringList collectionList);
    int                                 getCurrentBookOffset                ( ) { return  m_currentBook; }

signals:
    void                                hideMe                              ();
    void                                buyBook                             ( const BookInfo* );
    void                                archiveBook                         ( const BookInfo* );
    void                                unarchiveBook                       ( const BookInfo* );
    void                                removeBook                          ( const BookInfo* );
    void                                copyBook                            ( const QString& );
    void                                exportNotes                         ( const QString );
    void                                changeReadState                     ( const BookInfo*, BookInfo::readStateEnum );
    void                                addNewCollection                    ( const BookInfo* );
    void                                nextBookRequest                     ( const BookInfo* );
    void                                previousBookRequest                 ( const BookInfo* );
    void                                openBook                            ( const QString& );

public slots:
    void                                close                               ();

protected slots:
    void                                buyBookClicked                      ();
    void                                moreActionsClicked                  ();
    void                                addToCollectionClicked              ();
    void                                archiveBook                         ();

    void                                removeBook                          ();
    void                                copyBook                            ();
    void                                exportNotes                         ();

    void                                synopsisDown                        ();
    void                                synopsisUp                          ();
    void                                setupPagination                     (int);
    void                                changeReadState                     (int);
    void                                addBookToCollection                 (QString collectionName);
    void                                removeBookToCollection              (QString collectionName);
    void                                createNewCollection                 ();
    void                                previousBook                        ();
    void                                nextBook                            ();

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                        paintEvent                          ( QPaintEvent* );
    virtual void                        mouseReleaseEvent                   ( QMouseEvent* event );

    void                                setThumbnailElements                ( bool visible);

    const BookInfo*                     m_bookInfo;
    LibraryBookListActions*             m_bookListActions;
    LibraryCollectionLayer*             m_collectionLayer;
    QScrollBar*                         vbar;
    int                                 m_currentBook;
    int                                 m_totalBooks;

    enum ELibraryBookSummaryAction
    {
        DELETE,
        ARCHIVE,
        UNARCHIVE
    };
};

#endif // LIBRARYBOOKSUMMARY_H
