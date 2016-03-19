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

#include "QBook.h"
#include "MouseFilter.h"
#include "QBookApp.h"
#include "SettingsPagerCont.h"

#include <QPainter>
#include <QDebug>
#include <QWidget>

SettingsPagerCont::SettingsPagerCont(QWidget* parent) : QWidget(parent){
        qDebug() << "--->" << Q_FUNC_INFO;
        setupUi(this);

        connect(this->downPageBtn, SIGNAL(clicked()), this, SLOT(nextPage()));
        connect(this->upPageBtn, SIGNAL(clicked()), this, SLOT(previousPage()));
        connect(QBookApp::instance(),SIGNAL(swipe(int)),this,SLOT(handleSwipe(int)));
}

SettingsPagerCont::~SettingsPagerCont()
{
    /// Do nothing
    qDebug() << "--->" << Q_FUNC_INFO;
}

void SettingsPagerCont::nextPage(){
        int pageNum = this->currentPageLbl->text().toInt() ;
        int totalPage = this->totalPagesLbl->text().toInt();

        qDebug() <<  "nextPage" <<  pageNum;

        if(pageNum  < totalPage){
            this->currentPageLbl->setText(QString("%1").arg(pageNum + 1));
            emit nextPageReq();
        }
}

void SettingsPagerCont::previousPage(){
        int pageNum = this->currentPageLbl->text().toInt() ;

        qDebug() <<  "previousPage" <<  pageNum;

        if(pageNum  > 1){
            this->currentPageLbl->setText(QString("%1").arg(pageNum -1));
            emit previousPageReq();
        }
}

void SettingsPagerCont::setup(int totalPages){
        qDebug() << "--->" << Q_FUNC_INFO;

        this->currentPageLbl->setText("1");
        if(totalPages != 0){
            this->totalPagesLbl->setText(QString("%1").arg(totalPages));
        }
        else{
            this->totalPagesLbl->setText("1");
        }
        this->downPageBtn->show();
        this->upPageBtn->show();

}

void SettingsPagerCont::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }

void SettingsPagerCont::handleSwipe(int direction){
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

void SettingsPagerCont::hidePages()
{
    currentPageLbl->hide();
    totalPagesLbl->hide();
    pageLbl->hide();
    ofLbl->hide();
}
