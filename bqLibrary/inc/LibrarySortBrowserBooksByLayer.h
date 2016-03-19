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

#ifndef LIBRARYSORTBROWSERBOOKSBYLAYER_H
#define LIBRARYSORTBROWSERBOOKSBYLAYER_H

#include <QWidget>
#include "ui_LibrarySortBrowserBooksByLayer.h"

// Predeclaration
class QButtonGroup;

class LibrarySortBrowserBooksByLayer : public QWidget, public Ui::LibrarySortBrowserBooksByLayer
{
    Q_OBJECT

public:
                                        LibrarySortBrowserBooksByLayer      (QWidget* parent);
    virtual                             ~LibrarySortBrowserBooksByLayer     ();

    void                                setNameChecked                      ();
    void                                setDateChecked                      ();

    QString                             getNameSortName                     () const;
    QString                             getDateSortName                     () const;

signals:
    void                                nameSelected                        ();
    void                                dateSelected                        ();

private slots:
    void                                nameClicked                         ();
    void                                dateClicked                         ();

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                        paintEvent                          ( QPaintEvent* );

    QString                             m_nameName;
    QString                             m_dateName;

};

#endif // LIBRARYSORTBROWSERBOOKSBYLAYER_H
