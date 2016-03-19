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

#include "ViewerCollectionLayer.h"

#include <QPainter>
#include <QDebug>
#include "QBook.h"
#include "QBookApp.h"
#include "ViewerCollectionItem.h"
#include "Model.h"
#include <QButtonGroup>

#define ITEMS_PER_PAGE 4
#define BORDER_HIDE "border-color:#FFF;"

ViewerCollectionLayer::ViewerCollectionLayer( QWidget* parent ) :
    GestureWidget(parent) , m_page(0)
{
    setupUi(this);

    items.append(collectionItem0);
    items.append(collectionItem1);
    items.append(collectionItem2);
    items.append(collectionItem3);

    connect(collectionItem0,        SIGNAL(tap(int)),           this, SLOT(changeCollection(int)));
    connect(collectionItem1,        SIGNAL(tap(int)),           this, SLOT(changeCollection(int)));
    connect(collectionItem2,        SIGNAL(tap(int)),           this, SLOT(changeCollection(int)));
    connect(collectionItem3,        SIGNAL(tap(int)),           this, SLOT(changeCollection(int)));
    connect(VerticalPagerPopup,     SIGNAL(nextPageReq()),      this, SLOT(showNextPage()));
    connect(VerticalPagerPopup,     SIGNAL(previousPageReq()),  this, SLOT(showPreviousPage()));
    connect(addNewCollection,       SIGNAL(clicked()),          this, SIGNAL(createCollection()));
}

ViewerCollectionLayer::~ViewerCollectionLayer()
{
}

void ViewerCollectionLayer::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ViewerCollectionLayer::paint()
{
    qDebug() << Q_FUNC_INFO;

    const int pageOffset = m_page * items.size();
    const int size = items.size()-1;

    for(int i=0; i <= size; ++i)
    {

        int pos = pageOffset + i;
        QString collectionName = "";
        if(pos < m_modelCollections.size())
        {
            collectionName = m_modelCollections.at(pos);
            items[i]->paintCollection(collectionName,i);
            items[i]->show();
            if(m_bookCollections.contains(m_modelCollections[pos]))
                items[i]->setChecked(true);
            else
                items[i]->setChecked(false);
        }
        else
        {
            if(m_page == 0)
                items[i]->hide();
            else
            {
                items[i]->paintCollection("",i);
                items[i]->show();
                items[i]->setChecked(false);
                items[i]->setStyleSheet(BORDER_HIDE);
            }
        }
    }
}

void ViewerCollectionLayer::setup(QHash<QString, double> collectionList)
{
    qDebug() << Q_FUNC_INFO << collectionList;
    m_bookCollections = collectionList;
    m_modelCollections = QBookApp::instance()->getModel()->getCollections();
    m_page = 0;
    if(m_modelCollections.size() <= ITEMS_PER_PAGE)
        VerticalPagerPopup->hide();
    else
    {
        VerticalPagerPopup->show();
        int totalPages = 0;
        if(m_modelCollections.size()%ITEMS_PER_PAGE == 0)
            totalPages = m_modelCollections.size()/ITEMS_PER_PAGE;
        else
            totalPages = m_modelCollections.size()/ITEMS_PER_PAGE + 1;
        VerticalPagerPopup->setup(totalPages, m_page+1, false, false);
    }
}

void ViewerCollectionLayer::showNextPage()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    m_page++;
    paint();
    VerticalPagerPopup->setPage();
    Screen::getInstance()->setMode(Screen::MODE_QUICK, true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void ViewerCollectionLayer::showPreviousPage()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    m_page--;
    paint();
    VerticalPagerPopup->setPage();
    Screen::getInstance()->setMode(Screen::MODE_QUICK, true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void ViewerCollectionLayer::changeCollection(int idItem)
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();
    ViewerCollectionItem* item = items[idItem];
    QString collectionName = item->getText();
    QHash<QString, double>::iterator it = m_bookCollections.find(collectionName);
    if(it != m_bookCollections.end() && it.key() == collectionName)
    {
        item->setChecked(false);
        it = m_bookCollections.erase(it);
        emit removeCollection(collectionName);
    }
    else
    {
        item->setChecked(true);
        double index = QBookApp::instance()->getModel()->getBooksInCollectionCount(collectionName) + 1;
        m_bookCollections.insert(collectionName, index);
        emit addCollection(collectionName, index);
    }
    paint();
    Screen::getInstance()->setMode(Screen::MODE_QUICK, true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}
