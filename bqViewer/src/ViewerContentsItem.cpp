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

#include "ViewerContentsItem.h"

#include "bqUtils.h"

#include <QPainter>
#include <QDebug>

#define BOOKINDEXITEM_MAX_LENGTH 34

ViewerContentsItem::ViewerContentsItem(QWidget* parent) : GestureWidget(parent)
  , m_location(NULL)
{
        qDebug() << Q_FUNC_INFO;
        connect(this, SIGNAL(tap()), this, SLOT(handleTap()));
        setupUi(this);
}

ViewerContentsItem::~ViewerContentsItem()
{
    qDebug() << Q_FUNC_INFO;

    m_location = NULL;// Weak ptr.
}

void ViewerContentsItem::handleTap()
{
    qDebug() << Q_FUNC_INFO;
    emit chapterPressed(m_location->ref);
}

bool ViewerContentsItem::setItem(QDocView::Location* subContent, int bookmarks, int notes, int highligths)
{
    qDebug() << Q_FUNC_INFO;

    m_location = NULL;

    if (!subContent)
         return false;

    if(subContent->preview.isEmpty())
        return false;

    m_location = subContent;
    m_iNotes = notes;
    m_iHighlights = highligths;
    m_iBookmarks = bookmarks;
    return m_location;
}

void ViewerContentsItem::paint()
{
     qDebug() << Q_FUNC_INFO << currentChapter;

     QString spacer;
     int depth = m_location->depth;
     switch(depth) {
     case 0:
         break;
     case 1:
         spacer.append(" - ");
         break;
     case 2:
         spacer.append("   > ");
         break;
     default:
         if(depth >= 0) spacer.append("     + ");
         break;
     }

     chapterLbl->setText(spacer  + bqUtils::truncateStringToLength(m_location->preview, BOOKINDEXITEM_MAX_LENGTH - spacer.length()));

     if(m_iBookmarks > 0){
        bookmarksNumberLbl->setText(QString::number(m_iBookmarks));
        bookmarksNumberLbl->show();
     }else
         bookmarksNumberLbl->hide();

     if(m_iHighlights > 0){
         hightlightedNumberLbl->setText(QString::number(m_iHighlights));
         hightlightedNumberLbl->show();
     }else
         hightlightedNumberLbl->hide();

     if(m_iNotes > 0){
        notesNumberLbl->setText(QString::number(m_iNotes));
        notesNumberLbl->show();
     }else
        notesNumberLbl->hide();

     show();
}

//void ViewerContentsItem::showEvent(QShowEvent* )
//{
//    QString label = bqUtils::truncateStringToLength(chapterPageNumber->text(),BOOKINDEXITEM_MAX_LENGTH);
//    chapterPageNumber->setText(label);
//}

void ViewerContentsItem::paintEvent (QPaintEvent *)
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}



