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

#ifndef LIBRARYBOOKSFILTERLAYER_H
#define LIBRARYBOOKSFILTERLAYER_H

#include <QWidget>
#include "ui_LibraryBooksFilterLayer.h"

// Predeclaration
class QButtonGroup;

class LibraryBooksFilterLayer : public QWidget, public Ui::LibraryBooksFilterLayer
{
    Q_OBJECT

public:
                                        LibraryBooksFilterLayer             (QWidget* parent);
    virtual                             ~LibraryBooksFilterLayer            ();

    void                                setAllBooksChecked                  ();

    void                                setAllBooksNumber                   ( int size );
    void                                setStoreBooksNumber                 ( int size );
    void                                setStoreBooksBtnEnabled             ( bool enabled );

    QString                             getAllBooksName                     () const;
    QString                             getStoreBooksName                   () const;
    QString                             getMyCollectionsName                () const;
    QString                             getBrowserFileName                  () const;

signals:
    void                                myBooksSelected                     ();
    void                                storeBooksSelected                  ();
    void                                myCollectionsSelected               ();
    void                                browserFileSelected                 ();

private slots:
    void                                myBooksClicked                      ();
    void                                storeBooksClicked                   ();
    void                                myCollectionsClicked                ();
    void                                browserFileClicked                  ();

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                        paintEvent                          ( QPaintEvent* );

    QString                             m_allBooksName;
    QString                             m_storeBooksName;
    QString                             m_browserBooksName;

};

#endif // LIBRARYBOOKSFILTERLAYER_H
