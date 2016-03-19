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

#include "OpenBooks.h"
#include "ui_CdlOpenBooks.h"
#include "QBookApp.h"
#include "BookInfo.h"

#include <QPushButton>
#include <QObject>

#define MAX_OPEN_BOOKS 5

OpenBooks::OpenBooks(QBookApp *parent) : PopUp(parent) {
    setupUi(this);
}

void OpenBooks::refreshOpenBooks (QList<const BookInfo*> books){
        qDebug() << " <--- " << Q_FUNC_INFO;
        m_bookList = books;

        if(m_bookList.size()>MAX_OPEN_BOOKS)
            m_bookList = m_bookList.mid(0,MAX_OPEN_BOOKS);

        qDebug()<< "--->" << Q_FUNC_INFO << m_bookList.size() << "open books found";

        connect(closePopup,SIGNAL(clicked()),this,SLOT(hideSlot()));

        // Is not generic because it will be just for CDL
        m_bookButtons.clear();
        m_bookButtons.append(book1);
        m_bookButtons.append(book2);
        m_bookButtons.append(book3);
        m_bookButtons.append(book4);
        m_bookButtons.append(book5);

        prepareBooks();
}

OpenBooks::~OpenBooks (){
    //does nothing
}

void OpenBooks::hideSlot(){
        qDebug() << " <--- " << Q_FUNC_INFO;
        hide();
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
}

void OpenBooks::prepareBooks(){
        qDebug() << "*************" << Q_FUNC_INFO;
        for(int i = 0; i < MAX_OPEN_BOOKS;i++)
        {
            if(i < m_bookList.size()){
                m_bookButtons.at(i)->setText(m_bookList.at(i)->title);
                m_bookButtons.at(i)->show();
                connect(m_bookButtons.at(i),SIGNAL(clicked()),this,SLOT(selectedBook()));
            } else {
                m_bookButtons.at(i)->hide();
            }
        }
}

void OpenBooks::selectedBook(){
        QPushButton* senderBtn = (QPushButton*)sender();
        int index = m_bookButtons.indexOf(senderBtn);
        hide();

        emit openBook(m_bookList[index]);
}
