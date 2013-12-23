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

#ifndef VIEWERCONTENTSPOPUP_H
#define VIEWERCONTENTSPOPUP_H

#include "ui_ViewerContentsPopup.h"
#include "ViewerMenuPopUp.h"
#include "QDocView.h"

// Predeclarations
class ViewerContentsPopupWidget;
class BookInfo;
class BookLocation;
class Viewer;
class QKeyEvent;
class ViewerContentsList;
class ViewerAnnotationsList;
class QButtonGroup;

class ViewerContentsPopup : public ViewerMenuPopUp,
                            protected Ui::ViewerContentsPopup
{
    Q_OBJECT

public:
    ViewerContentsPopup(Viewer* viewer);
    virtual ~ViewerContentsPopup();

    virtual void                                        setup                                   ();
    virtual void                                        start                                   ();
    virtual void                                        stop                                    ();

    const QList<QDocView::Location*>&                   getContentList                          () const;
    void                                                clearContentList                        ();
    void                                                repaint                                 ();
    void                                                calculatePageForChapters                ();

signals:
    void                                                goToMark                                (QString);
    void                                                editNote                                (bool);
    void                                                deleteNote                              (bool);

protected slots:
    void                                                showPreviousPage                        ();
    void                                                showNextPage                            ();
    void                                                navigateToChapter                       ( const QString& );
    void                                                navigateToMark                          ( const QString& );
    void                                                editNote                                (BookLocation*);
    void                                                deleteNote                              (BookLocation*);
    void                                                resetPage                               ();
    void                                                viewMarks                               ();
    void                                                viewIndex                               ();

protected:
    void                                                paint                                   ();
    virtual void                                        paintEvent                              (QPaintEvent *);

    BookInfo*                                           m_book;// Weak ptr
    QString                                             m_bookPath;
    int                                                 m_bookType;

private:
    void                                                initContentList                         ();
    void                                                calculateMarksPage                      ();
    void                                                setCurrentChapter                       ();
    void                                                setCurrentView                          ();

    ViewerContentsPopupWidget*                          m_currentView;// Weak ptr
    ViewerContentsList*                                 m_chaptersWidget;
    ViewerAnnotationsList*                              m_marksWidget;
    int                                                 m_page;
    QButtonGroup*                                       m_buttonGroup;
};
#endif // VIEWERCONTENTSPOPUP_H
