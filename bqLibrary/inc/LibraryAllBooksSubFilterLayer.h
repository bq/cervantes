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

#ifndef LIBRARALLYBOOKSSUBFILTERLAYER_H
#define LIBRARALLYBOOKSSUBFILTERLAYER_H

#include <QWidget>
#include "ui_LibraryAllBooksSubFilterLayer.h"

class LibraryAllBooksSubFilterLayer : public QWidget, public Ui::LibraryAllBooksSubFilterLayer
{
    Q_OBJECT

public:
                                        LibraryAllBooksSubFilterLayer           (QWidget* parent);
    virtual                             ~LibraryAllBooksSubFilterLayer          ();

    void                                setAllBooksNumber                       ( int size );
    void                                setAllActiveBooksNumber                 ( int size );
    void                                setNewBooksNumber                       ( int size );
    void                                setReadBooksNumber                      ( int size );

    void                                setAllBooksChecked                      ();
    void                                setAllActiveBooksChecked                ();

    QString                             getAllBooksName                         () const;
    QString                             getAllActiveBooksName                   () const;
    QString                             getNewBooksName                         () const;
    QString                             getReadBooksName                        () const;


signals:
    void                                allBooksSelected                        ();
    void                                allNewBooksSelected                     ();
    void                                allActiveBooksSelected                  ();
    void                                readBooksSelected                       ();

private slots:
    void                                allBooksClicked                         ();
    void                                newBooksClicked                         ();
    void                                allActiveBooksClicked                   ();
    void                                alreadyReadClicked                      ();

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                        paintEvent                              ( QPaintEvent* );

    QString                             m_allBooksName;
    QString                             m_allActiveBooksName;
    QString                             m_newBooksName;
    QString                             m_readBooksName;
};

#endif // LIBRARALLYBOOKSSUBFILTERLAYER_H
