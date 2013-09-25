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

#ifndef VIEWERANNOTATIONSLIST_H
#define VIEWERANNOTATIONSLIST_H

#include "ViewerContentsPopupWidget.h"
#include "ui_ViewerAnnotationsList.h"

#include "BookLocation.h"
#include <QList>

#define NOTE_ITEMS_PER_PAGE (12)

// Predeclarations
class QWidget;
class ViewerContentsPopup;
class ViewerAnnotationItem;
class ViewerAnnotationActions;
class QPushButton;
class QPaintEvent;
class QButtonGroup;
class BookInfo;

class ViewerAnnotationsList : public ViewerContentsPopupWidget,
                              protected Ui::ViewerAnnotationsList
{
      Q_OBJECT

public:
    ViewerAnnotationsList(QWidget* parent);
    virtual ~ViewerAnnotationsList();

    void                                                setBook                             ( const BookInfo* book );
    void                                                setCurrentView                      ();

    virtual int                                         getTotalPages                       () const;

    virtual void                                        paint                               ( int currentPage);
    bool                                                hideActionsPopup                    ();


private slots:
    void                                                paintMarks                          ( int );
    void                                                handleTap                           (const QString& );
    void                                                handleLongPress                     ( BookLocation* );
    void                                                goToMark                            ( BookLocation* );
    void                                                deleteMark                          ( BookLocation* );
    void                                                editMark                            ( BookLocation* );

private:
    QList<ViewerAnnotationItem*>                        m_marksList;
    QList<const BookLocation*>*                         m_locationsList;
    QList<const BookLocation*>                          m_locationsPaintList;
    int                                                 m_locationsTypeCount[BookLocation::TYPE_COUNT];
    BookLocation::Type                                  m_currentView;
    QButtonGroup*                                       m_buttonGroup;

    void                                                paintEvent                          (QPaintEvent* );
    void                                                mouseReleaseEvent                   (QMouseEvent* );
    void                                                setBookmarksCheck                   ( );
    void                                                setAllNotesCheck                    ( );
    ViewerAnnotationActions*                            m_actions;

signals:
    void                                                navigateToMark                      ( const QString& );
    void                                                resetPage                           ();
    void                                                editNote                            (BookLocation *);
    void                                                deleteNote                          (BookLocation *);
};

#endif // VIEWERANNOTATIONSLIST_H
