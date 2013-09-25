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

#ifndef VIEWERTEXTACTIONSPOPUP_H
#define VIEWERTEXTACTIONSPOPUP_H

#include <QWidget>
#include "ui_ViewerTextActionsPopup.h"

class ViewerTextActionsPopup : public QWidget,
                               protected Ui::ViewerTextActionsPopup
{
    Q_OBJECT

public:
    ViewerTextActionsPopup(QWidget*);
    virtual ~ViewerTextActionsPopup();
    void                                        setup                                       ( const QString& dictionaryDefinition, const QPoint&, const QRect&, const int );

signals:
    void                                        hideMe                                      ();
    void                                        createNoteClicked                           ();
    void                                        highlightedClicked                          ();
    void                                        searchClicked                               ();
    void                                        completeDefinitionClicked                   ();
    void                                        wordToSearch                                ( const QString&, const QString& );

protected:
    virtual void                                showEvent                                   ( QShowEvent* event );

    bool                                        m_above;
    bool                                        m_below;
    QRect                                       m_bbox;
    QScrollBar*                                 vbar;
    int                                         m_delimiterHeight;

protected slots:
    void                                        nextPage                            ();
    void                                        previousPage                        ();
    void                                        handleSwipe                         (int);
};

#endif // VIEWERTEXTACTIONSPOPUP
