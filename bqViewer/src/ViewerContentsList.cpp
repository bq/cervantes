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

#include "ViewerContentsList.h"

#include "ViewerContentsItem.h"
#include "QBookApp.h"
#include "QDocView.h"

#include <QPainter>
#include <QWidget>
#include <QDebug>

#define CHECKED "background-color: #DADADA"
#define CHECKED9_HD "background-color: #DADADA; border-radius-bottom: 6px;"
#define CHECKED9_SD "background-color: #DADADA; border-radius-bottom: 5px;"

const int itemsPerPage = 9;

ViewerContentsList::ViewerContentsList(QWidget* parent) :
    ViewerContentsPopupWidget(parent)
  , m_locations(NULL)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);
    m_bookIndexList.append(BookIndexItem1);
    m_bookIndexList.append(BookIndexItem2);
    m_bookIndexList.append(BookIndexItem3);
    m_bookIndexList.append(BookIndexItem4);
    m_bookIndexList.append(BookIndexItem5);
    m_bookIndexList.append(BookIndexItem6);
    m_bookIndexList.append(BookIndexItem7);
    m_bookIndexList.append(BookIndexItem8);
    m_bookIndexList.append(BookIndexItem9);

    for(int i=0; i<itemsPerPage; ++i)
        connect(m_bookIndexList.at(i), SIGNAL(chapterPressed(const QString&)), this, SIGNAL(navigateToChapter(const QString&)));
}

ViewerContentsList::~ViewerContentsList()
{
    qDebug() << Q_FUNC_INFO;
    m_bookIndexList.clear();

    clearTableOfContent();

    delete m_locations;
    m_locations = NULL;
}

void ViewerContentsList::setBook(const BookInfo* book) {
    m_book = book;
}

void ViewerContentsList::clearTableOfContent()
{
    qDeleteAll(m_contentList.begin(), m_contentList.end());
    m_contentList.clear();
}

void ViewerContentsList::setTableOfContent( QDocView::TableOfContent* toc )
{
    qDebug() << Q_FUNC_INFO;

    clearTableOfContent();

    // TODO: Launch it in other thread so the book opening is not affected.

    flattenToc(0, toc);
}

void ViewerContentsList::setLocationList( QList<const BookLocation*>* locationList )
{
    qDebug() << Q_FUNC_INFO;

    delete m_locations;
    m_locations = locationList;
}

void ViewerContentsList::flattenToc(int depth, QDocView::TableOfContent* toc)
{
    if(!toc) return;

    int count = toc->itemCount();
    for(int i = 0; i<count; ++i)
    {
        if(!toc->itemAt(i)->title().isEmpty())
            m_contentList.append(toc->itemAt(i)->location());

        if(toc->itemAt(i)->itemCount() > 0)
            flattenToc(++depth, toc->itemAt(i));
    }
}

int ViewerContentsList::getTotalPages() const
{
    qDebug() << Q_FUNC_INFO << m_contentList.count();
    if( m_contentList.count() == 0 )
        return 0;

    int count = m_contentList.count();
    if( count % itemsPerPage == 0)
        return count / itemsPerPage;
    else
        return (count / itemsPerPage) + 1;
}

#include "Viewer.h"

void ViewerContentsList::paint( int currentPage)
{
    qDebug() << Q_FUNC_INFO << m_currentChapter;

    int size = m_contentList.count();
    int offset = (currentPage * itemsPerPage);
    for(int i = 0; i < itemsPerPage; ++i)
    {
        int pos = offset + i;
        m_bookIndexList.at(i)->hide();
        if( pos < size )
        {
            QDocView::Location *location;
            double finalPos = 0;
            int finalPage = 0;
            //Try to find a location with a correct pos, ignore the location
            //which page is the last page of the book, and is not the last location in the list.
            for(int j = pos + 1; j < size ; j++) {
                if(m_contentList.at(j)) {
                    location = m_contentList.at(j);
                    finalPage = location->page;
                    if(finalPage != m_book->pageCount) {
                        finalPos = location->pos;
                        break;
                    }
                }
            }
            location = m_contentList.at(pos);
            if(location){
                countMarks(location->pos, finalPos);

                if(location->preview == m_currentChapter && location->page == m_pageStart){
                    if(i == 8){
                        if(QBook::getResolution()==QBook::RES600x800)
                            m_bookIndexList.at(i)->setStyleSheet(CHECKED9_SD);
                        else
                            m_bookIndexList.at(i)->setStyleSheet(CHECKED9_HD);
                    }
                    if(!Viewer::isUsingCR3(m_book->format)) m_bookIndexList.at(i)->setStyleSheet(CHECKED);
                }
                else
                    m_bookIndexList.at(i)->setStyleSheet("");
                if(m_bookIndexList.at(i)->setItem(location, m_iBookmarks, m_iNotes, m_iHighlights))
                    m_bookIndexList.at(i)->paint();
            }else{
                m_bookIndexList.at(i)->setItem(0, 0, 0, 0);
            }
        }
    }
}

int ViewerContentsList::paintFirst()
{
    qDebug() << Q_FUNC_INFO << m_currentChapter << m_pageStart;

    int size = m_contentList.count();
    bool currentChapter;

    for(int j = 1; j <= getTotalPages(); j++)
    {
        for(int i = 0; i < itemsPerPage; ++i)
        {
            int pos = (j-1)*itemsPerPage + i;
            if( pos < size )
            {
                if(!m_contentList.at(pos))
                    break;

                currentChapter = (m_contentList.at(pos)->preview == m_currentChapter && m_contentList.at(pos)->page == m_pageStart);
                if(currentChapter)
                    break;
            }
        }
        if(currentChapter) {
            paint(j-1);
            return j;
        }
    }

    paint(0);
    return 1;
}

void ViewerContentsList::countMarks(double initialPos, double finalPos)
{
    qDebug() << Q_FUNC_INFO;

    double marksPos = 0;
    m_iBookmarks = 0;
    m_iNotes = 0;
    m_iHighlights = 0;

    if(Viewer::isUsingCR3(m_book->format)) return;

    QList<const BookLocation*>::iterator it = m_locations->begin();
    QList<const BookLocation*>::iterator itEnd = m_locations->end();

    for(; it != itEnd; ++it){
        marksPos = (*it)->pos;

        if(marksPos >= initialPos && (marksPos < finalPos || finalPos == 0))
        {
            switch((*it)->type)
            {
            case BookLocation::BOOKMARK:
                m_iBookmarks++;
                break;
            case BookLocation::NOTE:
                m_iNotes++;
                break;
            case BookLocation::HIGHLIGHT:
                m_iHighlights++;
                break;
            }
        }
    }

}

void ViewerContentsList::paintEvent (QPaintEvent *)
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
