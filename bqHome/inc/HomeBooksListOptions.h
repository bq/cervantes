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

#ifndef HomeBooksListOptions_H
#define HomeBooksListOptions_H

#include <QWidget>
#include "ui_HomeBooksListOptions.h"
#include "Home.h"
// Predeclarations

class BookInfo;
class Home;
class QWidget;

class HomeBooksListOptions : public QWidget, protected Ui::HomeBooksListOptions{

    Q_OBJECT

public:

                HomeBooksListOptions                        (QWidget* parent = 0);
    virtual     ~HomeBooksListOptions                       ();
    void        setupOptions                                (bool /*recent*/, bool /*category1*/, const QString& category1Name, bool /*category2*/, const QString& category2Name, bool /*topsales*/, const QString& category3Name, Home::VIEW);

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    void        paintEvent                                  ( QPaintEvent * );
    void        keyReleaseEvent                             ( QKeyEvent* );
    void        showEvent                                   ( QShowEvent * );

private slots:

    void        selectedCategory1                           ( );
    void        selectedCategory2                           ( );
    void        selectedCategory3                           ( );
    void        selectedRecents                             ( );


signals:

    void        viewSelected                                (Home::VIEW);
    void        hideMe                                      ();

};
#endif // HomeBooksListOptions_H
