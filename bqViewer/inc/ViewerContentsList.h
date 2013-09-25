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

#ifndef VIEWERCONTENTSLIST_H
#define VIEWERCONTENTSLIST_H

#include "ViewerContentsPopupWidget.h"
#include "ui_ViewerContentsList.h"

// Predeclaration
class ViewerContentsItem;
class QPaintEvent;
class BookInfo;
class BookLocation;
class ViewerContentsList : public ViewerContentsPopupWidget,
                           public Ui::ViewerContentsList
{
    Q_OBJECT

public:
    ViewerContentsList(QWidget * parent = 0);
    virtual ~ViewerContentsList();

public:
    void                                                setTableOfContent                   ( QDocView::TableOfContent* toc );
    void                                                setLocationList                     ( QList<const BookLocation*>* locationList );
    void                                                setBook                             ( const BookInfo* );

    virtual int                                         getTotalPages                       () const;
    const QList<QDocView::Location*>&                   getContentList                      () const { return m_contentList; }

    virtual void                                        paint                               ( int currentPage);
    int                                                 paintFirst                          ();
    void                                                countMarks                          ( double, double finalPage = 0);
    QString                                             m_currentChapter;
    int                                                 m_pageStart;

signals:
    void                                                chapterPressed                      ( const QString& );
    void                                                navigateToChapter                   ( const QString& );

protected:
    virtual void                                        paintEvent                          ( QPaintEvent * );

private:
    void                                                flattenToc                          ( int, QDocView::TableOfContent* );

    QList<ViewerContentsItem*>                          m_bookIndexList;
    QList<QDocView::Location*>                          m_contentList;
    QList<const BookLocation*>*                         m_locations;
    int                                                 m_iBookmarks;
    int                                                 m_iNotes;
    int                                                 m_iHighlights;
    const BookInfo*                                     m_book;
};
#endif // VIEWERCONTENTSLIST_H
