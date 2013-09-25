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

#include "QBook.h"
#include "HorizontalPagination.h"

#include <QDebug>
#include <QWidget>

HorizontalPagination::HorizontalPagination(QWidget *parent) : QWidget(parent)
{
    qDebug() << "--->" << Q_FUNC_INFO;
    setupUi(this);

    connect(this->next, SIGNAL(clicked()), this, SLOT(nextPage()));
    connect(this->previous, SIGNAL(clicked()), this, SLOT(previousPage()));
}

HorizontalPagination::~HorizontalPagination()
{
    /// Do nothing
    qDebug() << "--->" << Q_FUNC_INFO;
}

void HorizontalPagination::nextPage(){

    int pageNum = this->currentPage->text().toInt() ;
    int totalPage = this->totalPages->text().toInt();

    qDebug() <<  "nextPage" <<  pageNum;

    if(pageNum  < totalPage){
        //this->next->hide();
        //emit lastPage();

     //   this->previous->show();
        this->currentPage->setText(QString("%1").arg(pageNum + 1));
        emit nextPageReq();
    }
}

void HorizontalPagination::previousPage(){

    int pageNum = this->currentPage->text().toInt() ;

    qDebug() <<  "previousPage" <<  pageNum;

    if(pageNum  > 1){
        this->currentPage->setText(QString("%1").arg(pageNum -1));
        emit previousPageReq();
    }
}

void HorizontalPagination::setup(int totalPages){
    qDebug() << "--->" << Q_FUNC_INFO;

    this->currentPage->setText("1");
    if(totalPages != 0){
        this->totalPages->setText(QString("%1").arg(totalPages));
    }
    else{
        this->totalPages->setText("1");
    }
    this->next->show();
    this->previous->show();
   /*f(totalPages == 1){
        this->next->hide();
    }*/
}

void HorizontalPagination::hidePages()
{
    this->page->hide();
    this->currentPage->hide();
    this->totalPages->hide();
    this->connector->hide();
}

