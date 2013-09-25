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

#ifndef VIEWERANNOTATIONACTIONS_H
#define VIEWERANNOTATIONACTIONS_H

#include <QWidget>
#include "ui_ViewerAnnotationActions.h"

class BookLocation;

class ViewerAnnotationActions : public QWidget, protected Ui::ViewerAnnotationActions
{
    Q_OBJECT
public:
                                        ViewerAnnotationActions              ( QWidget* parent );
    virtual                             ~ViewerAnnotationActions             ();

public slots:
    void                                setBookmark                         (BookLocation*);
    void                                setNote                             (BookLocation*);
    int                                 getPopUpWidth                       ();
    int                                 getPopUpHeight                      ();

signals:
    void                                deleteMark                          (BookLocation*);
    void                                editMark                            (BookLocation*);
    void                                goToMark                            (BookLocation*);

protected slots:
    void                                deleteMark                          ();
    void                                editMark                            ();
    void                                goToMark                            ();

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                        paintEvent                          ( QPaintEvent* );
    BookLocation*                       m_location;

};
#endif // VIEWERANNOTATIONACTIONS_H
