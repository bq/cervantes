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

#include "bqVerticalPageHandler.h"

#include "QBook.h"
#include "MouseFilter.h"
#include "QBookApp.h"

#include <QDebug>
#include <QWidget>

bqVerticalPageHandler::bqVerticalPageHandler(QWidget* parent) : QWidget(parent)
{
    qDebug() << "--->" << Q_FUNC_INFO;
    setupUi(this);

    connect(this->downPageBtn, SIGNAL(clicked()), this, SLOT(nextPage()));
    connect(this->upPageBtn, SIGNAL(clicked()), this, SLOT(previousPage()));
    connect(QBookApp::instance(),SIGNAL(swipe(int)),this,SLOT(handleSwipe(int)));
}

bqVerticalPageHandler::~bqVerticalPageHandler()
{
    /// Do nothing
    qDebug() << "--->" << Q_FUNC_INFO;
}

void bqVerticalPageHandler::setPage(int pageNum){

    int totalPage = this->totalPages->text().toInt();

    qDebug() <<  pageNum;

    if((pageNum > 0)
        && (pageNum <= totalPage)){
        this->currentPage->setText(QString("%1").arg(pageNum));
    }
    else
        qWarning() << Q_FUNC_INFO << "Page out of index";
}

void bqVerticalPageHandler::nextPage(){

    int pageNum = this->currentPage->text().toInt();
    int totalPage = this->totalPages->text().toInt();

    qDebug() <<  "nextPage" <<  pageNum;

    if(pageNum  < totalPage){
        this->currentPage->setText(QString("%1").arg(pageNum + 1));
        emit nextPageReq();
    }
}

void bqVerticalPageHandler::previousPage(){

    int pageNum = this->currentPage->text().toInt();

    qDebug() <<  "previousPage" <<  pageNum;

    if(pageNum  > 1){
        this->currentPage->setText(QString("%1").arg(pageNum -1));
        emit previousPageReq();
    }
}

void bqVerticalPageHandler::setup(int totalPages, bool initial){
    qDebug() << "--->" << Q_FUNC_INFO;

    if(initial)
        this->currentPage->setText("1");
    if(totalPages != 0){
        this->totalPages->setText(QString("%1").arg(totalPages));
    }
    else{
        this->totalPages->setText("1");
    }
    this->downPageBtn->show();
    this->upPageBtn->show();
}

void bqVerticalPageHandler::hidePages()
{
    this->pageText->hide();
    this->currentPage->hide();
    this->totalPages->hide();
    this->connector->hide();
}

int bqVerticalPageHandler::getCurrentPage()
{
    return currentPage->text().toInt();
}

void bqVerticalPageHandler::handleSwipe(int direction){
    qDebug()<< Q_FUNC_INFO << "Direction" << direction;

    if(isVisible()){
        switch(direction){
        case MouseFilter::SWIPE_R2L:
        case MouseFilter::SWIPE_D2U:
            nextPage();
            break;

        case MouseFilter::SWIPE_L2R:
        case MouseFilter::SWIPE_U2D:
            previousPage();
            break;

        default:
            //ignore
            break;
        }
    }

}

