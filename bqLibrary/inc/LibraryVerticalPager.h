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

#ifndef LIBRARYVERTICALPAGER_H
#define LIBRARYVERTICALPAGER_H

#include <QWidget>
#include "ui_LibraryVerticalPager.h"
#include <QTimer>

class FullScreenWidget;

class LibraryVerticalPager : public QWidget, protected Ui::LibraryVerticalPager
{
    Q_OBJECT

public:
                                        LibraryVerticalPager                ( QWidget* parent = 0 );
    virtual                             ~LibraryVerticalPager               ();

    void                                setup                               ( int totalPages, int currentPage, bool loopable = false );
    void                                hidePages                           ();
    void                                setPage                             ( int page );

signals:
    void                                nextPageReq                         ( );
    void                                previousPageReq                     ( );
    void                                firstPage                           ( );
    void                                lastPage                            ( );

protected slots:
    void                                nextPage                            ();
    void                                previousPage                        ();
    void                                nextPagePressed                     ();
    void                                previousPagePressed                 ();
    void                                nextPageReleased                    ();
    void                                previousPageReleased                ();
    void                                goToLastPage                        ();
    void                                goToFirstPage                       ();

public slots:
    void                                handleSwipe                         (int);

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                        paintEvent                          ( QPaintEvent* );

private:
    int                                 m_currentPage;
    int                                 m_totalPages;
    bool                                m_loopable;
    QString                             m_strPagesText;
    QTimer                              nextPageTimer;
    QTimer                              previousPageTimer;
};


#endif // LIBRARYVERTICALPAGER_H
