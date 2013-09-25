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

#ifndef VIEWEREDITNOTEPOPUP_H
#define VIEWEREDITNOTEPOPUP_H

#include "ui_ViewerEditNotePopup.h"
#include "PopUp.h"

#include <QWidget>

class ViewerEditNotePopup : public PopUp,
                            protected Ui::ViewerEditNotePopup
{
    Q_OBJECT

public:
    ViewerEditNotePopup(QWidget* parent);
    virtual ~ViewerEditNotePopup();

    void                                setUpCreateNote                     ();
    void                                setUpEditNote                       ( const QString& note );

    QString                             getNoteText                         () const;

protected:
    virtual void                        paintEvent                          ( QPaintEvent* );

signals:
    void                                saveNote                            ();

    void                                hideMe                              ();
};

#endif // VIEWEREDITNOTEPOPUP_H

