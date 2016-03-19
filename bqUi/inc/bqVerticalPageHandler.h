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

#ifndef BQVERTICALPAGINATION_H
#define BQVERTICALPAGINATION_H

#include "ui_bqVerticalPageHandler.h"
#include <QWidget>
#include <QFrame>

class bqVerticalPageHandler : public QWidget,
                        protected Ui::bqVerticalPageHandler
{
    Q_OBJECT

public:
    bqVerticalPageHandler(QWidget*);
    virtual ~bqVerticalPageHandler();
    void setup(int,bool initial = true);
    void initialButtons();
    void hidePages();
    int getCurrentPage();
    QFrame* getVerticalPageCont(){return verticalPageCont;}
    int getTotalPages() {return totalPages->text().toInt();}


signals:
    void nextPageReq();
    void previousPageReq();
    void firstPage();
    void lastPage();

public slots:

    void nextPage();
    void previousPage();
    void setPage(int);
    void handleSwipe(int);

};

#endif // BQVERTICALPAGINATION_H
