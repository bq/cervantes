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

#ifndef LIBRARYACTIONSMENU_H
#define LIBRARYACTIONSMENU_H

#include <QWidget>
#include "ui_LibraryActionsMenu.h"

class LibraryActionsMenu : public QWidget, protected Ui::LibraryActionsMenu
{
    Q_OBJECT
public:
                                        LibraryActionsMenu              ( QWidget* parent );
    virtual                             ~LibraryActionsMenu             ();
    void                                setFile                         (QString);

signals:
    void                                deleteFile                          (const QString&);
    void                                copyFile                            (const QString&);

protected slots:
    void                                copyFile                            ();
    void                                deleteFile                          ();

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                        paintEvent                          ( QPaintEvent* );
    QString                             m_path;

};
#endif // LIBRARYACTIONSMENU_H
