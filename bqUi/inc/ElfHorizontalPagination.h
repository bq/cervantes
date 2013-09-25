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

#ifndef ELFHORIZONTALPAGINATION_H
#define ELFHORIZONTALPAGINATION_H

#include "ui_ElfHorizontalPagination.h"
#include <QWidget>

class ElfHorizontalPagination : public QWidget, protected Ui::ElfHorizontalPagination{

    Q_OBJECT

public:
    ElfHorizontalPagination(QWidget*);
    ~ElfHorizontalPagination();
    void setup(int, bool, bool loopable = false);
    void initialButtons();
    void hidePages();
    QPushButton* getNextBtn() {return next;}
    QPushButton* getPreviousBtn() {return previous;}
    QPushButton* getSearchBtn() {return searchButton;}


signals:
    void nextPageReq();
    void previousPageReq();
    void firstPage();
    void lastPage();
    void launchSearch();

public slots:
    void nextPage();
    void previousPage();
    void handleSwipe(int);

private:
    int m_totalPages;
    bool m_loopable;

};

#endif // ELFHORIZONTALPAGINATION_H
