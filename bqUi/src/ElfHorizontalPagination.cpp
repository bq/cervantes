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

#include "ElfHorizontalPagination.h"

#include "MouseFilter.h"
#include "QBookApp.h"

#include <QDebug>

ElfHorizontalPagination::ElfHorizontalPagination (QWidget *parent) : QWidget(parent), m_totalPages(0), m_loopable(false){
        qDebug() << "--->" << Q_FUNC_INFO;
        setupUi(this);

        connect(this->next, SIGNAL(clicked()), this, SLOT(nextPage()));
        connect(this->previous, SIGNAL(clicked()), this, SLOT(previousPage()));
        connect(searchButton, SIGNAL(clicked()), this, SIGNAL(launchSearch()));
        connect(QBookApp::instance(),SIGNAL(swipe(int)),this,SLOT(handleSwipe(int)));

}

ElfHorizontalPagination::~ElfHorizontalPagination (){
        /// Do nothing
        qDebug() << "--->" << Q_FUNC_INFO;
}

void ElfHorizontalPagination::nextPage ()
{
    int pageNum = this->currentPage->text().toInt() ;

    if(pageNum  < m_totalPages)
    {
        this->currentPage->setText(QString("%1").arg(pageNum + 1));
        qDebug() << Q_FUNC_INFO << "NextPage: " <<  pageNum;
        emit nextPageReq();
    }
    else if(m_loopable)
    {
        this->currentPage->setText("1");
        qDebug() << Q_FUNC_INFO << "NextPage: " <<  pageNum;
        emit firstPage();
    }
}

void ElfHorizontalPagination::previousPage ()
{
    int pageNum = this->currentPage->text().toInt() ;

    if(pageNum  > 1)
    {
        this->currentPage->setText(QString("%1").arg(pageNum - 1));
        qDebug() << Q_FUNC_INFO << "PreviousPage: " <<  pageNum;
        emit previousPageReq();
    }
    else if(m_loopable)
    {
        this->currentPage->setText(QString("%1").arg(m_totalPages));
        qDebug() << Q_FUNC_INFO << "PreviousPage: " <<  pageNum;
        emit lastPage();
    }
}

void ElfHorizontalPagination::setup (int totalPages, bool searchable, bool loopable){
        qDebug() << "--->" << Q_FUNC_INFO;

        m_totalPages = (totalPages != 0) ? totalPages : 1;
        m_loopable = loopable;

        this->currentPage->setText("1");
        this->totalPages->setText(QString("%1").arg(totalPages));

        this->page->show();
        this->currentPage->show();
        this->totalPages->show();
        this->connector->show();

        this->next->show();
        this->previous->show();
        this->searchButton->setVisible(searchable);
}

void ElfHorizontalPagination::hidePages ()
{
    this->page->hide();
    this->currentPage->hide();
    this->totalPages->hide();
    this->connector->hide();
}

void ElfHorizontalPagination::handleSwipe(int direction){
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
