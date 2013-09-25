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

#ifndef HORIZONTALPAGINATION_H
#define HORIZONTALPAGINATION_H

#include "ui_HorizontalPagination.h"
#include <QWidget>

class HorizontalPagination : public QWidget,
                        protected Ui::HorizontalPagination
{
    Q_OBJECT

public:
    HorizontalPagination(QWidget*);
    ~HorizontalPagination();
    void setup(int);
    void initialButtons();
    void hidePages();
    QPushButton* getNextBtn() {return next;}
    QPushButton* getPreviousBtn() {return previous;}


signals:
    void nextPageReq();
    void previousPageReq();
    void firstPage();
    void lastPage();

public slots:

    void nextPage();
    void previousPage();

};

#endif // HORIZONTALPAGINATION_H
