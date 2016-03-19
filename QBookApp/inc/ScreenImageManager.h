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

#ifndef SCREENIMAGEMANAGER_H
#define SCREENIMAGEMANAGER_H

#include <QWidget>
#include <QPixmap>

// Predeclarations
class QPaintEvent;

class ScreenImageManager : public QWidget
{
    Q_OBJECT

public:
    ScreenImageManager();
    virtual ~ScreenImageManager();

    bool                                    setImage                        ( const QString& path );

protected:
    virtual void                            paintEvent                      ( QPaintEvent* event );

private:
    QPixmap                                 m_pixmap;

};

#endif // SCREENIMAGEMANAGER_H
