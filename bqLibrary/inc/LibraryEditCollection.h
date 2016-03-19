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

#ifndef LIBRARYEDITCOLLECTION_H
#define LIBRARYEDITCOLLECTION_H

#include "ui_LibraryEditCollection.h"
#include "LibraryBookToCollectionItem.h"
#include "GestureWidget.h"
#include "LibraryVerticalPager.h"

class Keyboard;

class LibraryEditCollection : public GestureWidget , protected Ui::LibraryEditCollection
{
    Q_OBJECT

public:

                                                LibraryEditCollection               (QWidget* parent=0);
    virtual                                     ~LibraryEditCollection              ();

    void                                        setup                               (const QString&, const BookInfo *book=NULL);
    void                                        paint                               ();
    void                                        checkBookToAdd                      (const BookInfo* bookInfo);
    bool                                        isFromBookSummary                   () { return b_fromBookSummary; }
    bool                                        collectionSaved                     () { return b_collectionSaved; }

signals:
    void                                        hideMe                              ();

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                                paintEvent                          ( QPaintEvent* );
    void                                        changeOldCollection                 ();
    void                                        saveItemsInfo                       ();
    bool                                        isSaveInHash                        ();
    void                                        paintFromHash                       ();
    bool                                        checkCollectionExist                ();

protected slots:
    void                                        handleCollectionNameLine            ();
    void                                        hideKeyboard                        ();
    void                                        saveCollection                      ();
    void                                        firstPage                           ();
    void                                        lastPage                            ();
    void                                        previousPage                        ();
    void                                        nextPage                            ();
    void                                        selectAll                           ();
    void                                        unSelectAll                         ();
    void                                        clearTextEdit                       ();
    void                                        setPageChanged                      ();
    void                                        handleClose                         ();
    void                                        checkCollectionName                 ();

private:
    QList<const BookInfo*>                      m_books;
    QList<LibraryBookToCollectionItem*>         items;
    QHash<QString, bool>*                       m_listBooks;
    int                                         m_page;
    int                                         m_totalPages;
    QString                                     m_collection;
    QString                                     m_initialCollectionName;
    QString                                     m_initialText;
    Keyboard*                                   m_keyboard;
    bool                                        m_newCollection;
    bool                                        m_pageChanged;
    bool                                        b_fromBookSummary;
    bool                                        b_collectionSaved;

};

#endif // LIBRARYEDITCOLLECTION_H
