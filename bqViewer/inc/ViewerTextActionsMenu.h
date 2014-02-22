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

#ifndef VIEWERTEXTACTIONSMENU_H
#define VIEWERTEXTACTIONSMENU_H

#include "ViewerTextActionsPopup.h"
#include "ui_ViewerTextActionsMenu.h"

class QWidget;

class ViewerTextActionsMenu : public QWidget,
                              protected Ui::ViewerTextActionsMenu
{
    Q_OBJECT

public:
    ViewerTextActionsMenu(QWidget* parent);

    void                                    setup                       ( bool newMark, bool oneWord, const QPoint&, const QRect&, const int );

protected:
    virtual void                            showEvent                   ( QShowEvent* event );
    virtual void                            paintEvent                  ( QPaintEvent* event );

    bool                                    m_above;
    bool                                    m_below;
    QRect                                   m_bbox;
    int                                     m_delimiterHeight;

signals:
    void                                    notesActionClicked          ();
    void                                    highlightActionClicked      ();
    void                                    searchActionClicked         ();
    void                                    deleteActionClicked         ();
    void                                    dictioActionClicked         ();
    void                                    wikipediaClicked            ();
};

#endif // VIEWERTEXTACTIONSMENU_H
