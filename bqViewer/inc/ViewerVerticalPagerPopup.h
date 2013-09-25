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

#ifndef VIEWERVERTICALPAGERPOPUP_H
#define VIEWERVERTICALPAGERPOPUP_H

#include "ui_ViewerVerticalPagerPopup.h"
#include <QWidget>
#include <QFrame>

class QPainter;

class ViewerVerticalPagerPopup : public QWidget,
                                 protected Ui::ViewerVerticalPagerPopup
{
    Q_OBJECT

public:
    ViewerVerticalPagerPopup(QWidget*);
    virtual ~ViewerVerticalPagerPopup();

    void                                                setup                                       (int totalPages, int currentPage, bool loopable, bool scrollBar = false);
    void                                                hideLabel                                   ();
    void                                                hideLabels                                  ();
    void                                                setPage                                     ();

signals:
    void                                                nextPageReq                                 ();
    void                                                previousPageReq                             ();
    void                                                firstPage                                   ();
    void                                                lastPage                                    ();

public slots:

    void                                                nextPage                                    ();
    void                                                previousPage                                ();
    void                                                handleSwipe                                 (int);

private:
    void                                                paintEvent                                  (QPaintEvent *);

    int                                                 m_currentPage;
    int                                                 m_totalPages;
    bool                                                m_loopable;
    bool                                                b_scrollBar;
    QString                                             m_strPagesText;
};

#endif // VIEWERVERTICALPAGERPOPUP_H
