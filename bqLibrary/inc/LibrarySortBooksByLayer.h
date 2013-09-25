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

#ifndef LIBRARYSORTBOOKSBYLAYER_H
#define LIBRARYSORTBOOKSBYLAYER_H

#include <QWidget>
#include "ui_LibrarySortBooksByLayer.h"

// Predeclaration
class QButtonGroup;

class LibrarySortBooksByLayer : public QWidget, public Ui::LibrarySortBooksByLayer
{
    Q_OBJECT

public:
                                        LibrarySortBooksByLayer             (QWidget* parent);
    virtual                             ~LibrarySortBooksByLayer            ();

    void                                enableRecent                        (bool enable);
    void                                setRecentChecked                    ();
    void                                setTitleChecked                     ();
    void                                setAuthorChecked                    ();

    QString                             getRecentSortName                   () const;
    QString                             getTitleSortName                    () const;
    QString                             getAuthorSortName                   () const;

signals:
    void                                recentSelected                      ();
    void                                titleSelected                       ();
    void                                authorSelected                      ();

private slots:
    void                                recentClicked                       ();
    void                                titleClicked                        ();
    void                                authorClicked                       ();

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                        paintEvent                          ( QPaintEvent* );

    QString                             m_recentName;
    QString                             m_titleName;
    QString                             m_authorName;

};

#endif // LIBRARYSORTBOOKSBYLAYER_H
