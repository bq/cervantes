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

#ifndef LIBRARYBOOKLISTACTIONS_H
#define LIBRARYBOOKLISTACTIONS_H

#include "BookInfo.h"
#include <QWidget>
#include "ui_LibraryBookListActions.h"
#include <QButtonGroup>

class LibraryBookListActions : public QWidget, protected Ui::LibraryBookListActions
{
    Q_OBJECT
public:
                                        LibraryBookListActions              ( QWidget* parent );
    virtual                             ~LibraryBookListActions             ();

    void                                setButtonsState                     ( BookInfo::readStateEnum );

signals:
    void                                readStateChanged                    ( int );

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                        paintEvent                          ( QPaintEvent* );
    QButtonGroup*                       g_readButtons;
};

#endif // LIBRARYBOOKLISTACTIONS_H
