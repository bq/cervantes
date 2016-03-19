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

#ifndef LIBRARYIMAGEFULLSCREEN_H
#define LIBRARYIMAGEFULLSCREEN_H

#include "GestureWidget.h"
#include "ui_LibraryImageFullScreen.h"


class LibraryImageFullScreen : public GestureWidget, protected Ui::LibraryImageFullScreen
{
    Q_OBJECT

public:
                                        LibraryImageFullScreen                  ( QWidget* parent );
    virtual                             ~LibraryImageFullScreen                 ();
    void                                setup                                   ( QString path, int, int );


protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                        paintEvent                          ( QPaintEvent* );
    QString                             currentImagePath;

protected slots:
    void                                previousImage                       ( );
    void                                nextImage                           ( );
    void                                handleSwipe                         ( int );

signals:
    void                                hideMe                              ( );
    void                                nextImageRequest                    ( const QString& );
    void                                previousImageRequest                ( const QString& );
};

#endif // LIBRARYIMAGEFULLSCREEN_H
