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

#ifndef VIEWERNOTEACTIONSPOPUP_H
#define VIEWERNOTEACTIONSPOPUP_H

#include "PopUp.h"
#include "ui_ViewerNoteActionsPopup.h"

class ViewerNoteActionsPopup : public PopUp, protected Ui::ViewerNoteActionsPopup
{
    Q_OBJECT
public:
    ViewerNoteActionsPopup(QWidget* parent);

    void                                    setup                       ( const QString& noteText, bool oneWord, const QPoint&, const QRect&, const int );

protected:
    virtual void                            showEvent                   ( QShowEvent* event );
    virtual void                            paintEvent                  ( QPaintEvent* event );

    bool                                    m_above;
    QRect                                   m_bbox;
    int                                     m_delimiterHeight;

signals:
    void                                    hideMe                      ();
    void                                    dictioActionClicked         ();
    void                                    searchActionClicked         ();
    void                                    editActionClicked           ();
    void                                    deleteActionClicked         ();
};

#endif // VIEWERNOTEACTIONSPOPUP_H
