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

#ifndef VIEWERBOOKSUMMARY_H
#define VIEWERBOOKSUMMARY_H

#include "ui_ViewerBookSummary.h"
#include "ViewerMenuPopUp.h"
#include "BookInfo.h"

class Viewer;
class ViewerBookListActions;
class ViewerCollectionLayer;
class PowerManagerLock;

class ViewerBookSummary : public ViewerMenuPopUp,
                          protected Ui::ViewerBookSummary
{
    Q_OBJECT

public:
    ViewerBookSummary(Viewer* viewer);
    virtual ~ViewerBookSummary();

    void                                                        setBook                             ( const BookInfo* );

    virtual void                                                setup                               ();
    virtual void                                                start                               ();
    virtual void                                                stop                                ();

    void                                                        setActionsBtnText                   ( BookInfo::readStateEnum );
    const BookInfo*                                             getBookInfo                         () { return m_book; }

public slots:
    void                                                        previousPage                        ();
    void                                                        nextPage                            ();
    void                                                        buyBook                             ();

private:
    ViewerBookListActions*                                      m_viewerListActions;
    ViewerCollectionLayer*                                      m_collectionLayer;
    const BookInfo*                                             m_book;
    void                                                        setupPagination                     ();
    void                                                        paintEvent                          ( QPaintEvent* );
    PowerManagerLock*                                           m_powerLock;
    QScrollBar*                                                 vbar;

signals:
    void                                                        addNewCollection                    (const BookInfo*);

private slots:
    void                                                        moreActionsClicked                  ();
    void                                                        addToCollectionClicked              ();
    void                                                        copyBook                            ();
    void                                                        storeBook                           ();
    void                                                        deleteBook                          ();
    void                                                        exportNotes                         ();
    void                                                        changeReadState                     ( int );
    void                                                        setCollectionLayerBtnText           (QStringList collectionList);
    void                                                        addBookToCollection                 (QString collectionName);
    void                                                        removeBookToCollection              (QString collectionName);
    void                                                        createNewCollection                 ();
};

#endif // VIEWERBOOKSUMMARY_H
