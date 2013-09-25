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

#include <QDebug>
#include <QWidget>
#include <QList>
#include <QPainter>
#include "QBookApp.h"
#include "MouseFilter.h"
#include "HomeBooksList.h"
#include "HomeBooksListItem.h"
#include "Home.h"


QString NEXT_DISABLED("background: none");
QString NEXT_ENABLED("background-image: url(:/res/next_book.png)");
QString PREVIOUS_DISABLED("background: none");
QString PREVIOUS_ENABLED("background-image: url(:/res/previous_book.png)");

#define BOOKS_PER_PAGE 3

HomeBooksList::HomeBooksList(QWidget *parent)
    : GestureWidget(parent)
    , currentPage(0)
{
        setupUi(this);

        items.append(cover1);
        items.append(cover2);
        items.append(cover3);

        connect(previousBtn, SIGNAL(clicked()), this, SLOT(previousPage()));
        connect(nextBtn, SIGNAL(clicked()), this, SLOT(nextPage()));
        connect(QBookApp::instance(),SIGNAL(swipe(int)),this,SLOT(handleSwipe(int)));
        connect(cover1, SIGNAL(openContent(const BookInfo*)), this, SLOT(openContent(const BookInfo*)));
        connect(cover2, SIGNAL(openContent(const BookInfo*)), this, SLOT(openContent(const BookInfo*)));
        connect(cover3, SIGNAL(openContent(const BookInfo*)), this, SLOT(openContent(const BookInfo*)));
}

HomeBooksList::~HomeBooksList(){
    // do nothing
}

void HomeBooksList::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }

void HomeBooksList::setBooks(const QList<const BookInfo*>& list)
{
        qDebug() << "---->" << Q_FUNC_INFO;
        books = list;
        currentPage = 0;
        hide(BOOKS_PER_PAGE - books.size());
}

void HomeBooksList::showList(){
        qDebug() << "---->" << Q_FUNC_INFO;
        showPaginator();

        const int librarySize = books.size();
        const int pageOffset = currentPage*items.size();
        const int size = items.size()-1;
        int itemsToHide = 3;

        for(int i=0; i <= size; ++i){

            int pos = pageOffset + i;
            if(pos < librarySize)
            {
                items[i]->paintBook(books.at(pos));
                items[i]->show();
                itemsToHide--;
            }else
            {
                items[i]->paintBook(0);
                items[i]->hide();
            }
        }
        hide(itemsToHide);
}

void HomeBooksList::hide(int elements){
        qDebug() << "---->" << Q_FUNC_INFO << elements;
        showPaginator();

        switch(elements) {
        case 3:
            cover3Cont->hide();
            cover2Cont->hide();
            cover1Cont->hide();
            break;
        case 2:
            cover3Cont->hide();
            cover2Cont->hide();
            cover1Cont->show();
            break;
        case 1:
            cover3Cont->hide();
            cover2Cont->show();
            cover1Cont->show();
            break;
        default:
            cover3Cont->show();
            cover2Cont->show();
            cover1Cont->show();
            break;
        }

}


void HomeBooksList::openContent(const BookInfo* book){
    qDebug() << "---->" << Q_FUNC_INFO;
    emit openBook(book);
}

void HomeBooksList::nextPage(){
        qDebug() << "---->" << Q_FUNC_INFO;
        ++currentPage;
        showList();
}

void HomeBooksList::previousPage(){
        qDebug() << "---->" << Q_FUNC_INFO;
        --currentPage;
        showList();
}

void HomeBooksList::handleSwipe(int direction)
{
    if(!isVisible())
        return;

    qDebug()<< Q_FUNC_INFO << "Direction" << direction;

    switch(direction)
    {
    case MouseFilter::SWIPE_R2L:
    case MouseFilter::SWIPE_D2U:
        if(nextBtn->isEnabled())
            nextPage();
        break;

    case MouseFilter::SWIPE_L2R:
    case MouseFilter::SWIPE_U2D:
        if(previousBtn->isEnabled())
            previousPage();
        break;

    default:
        //ignore
        break;
    }
}

void HomeBooksList::showPaginator(){
        qDebug() << "---->" << Q_FUNC_INFO;
        if(currentPage == 0){
            qDebug() << "Previous Disabled";
            previousBtn->setStyleSheet(PREVIOUS_DISABLED);
            previousBtn->setEnabled(false);
        }else{
            qDebug() << "Previous Enabled";
            previousBtn->setStyleSheet(PREVIOUS_ENABLED);
            previousBtn->setEnabled(true);
        }

        if(books.size() <= (currentPage + 1)*items.size()){
            qDebug() << "Next Disabled";
            nextBtn->setStyleSheet(NEXT_DISABLED);
            nextBtn->setEnabled(false);
        }else{
            qDebug() << "Next Enabled";
            nextBtn->setStyleSheet(NEXT_ENABLED);
            nextBtn->setEnabled(true);
        }
}
