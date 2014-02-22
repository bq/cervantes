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

#ifndef VIEWERMARKHANDLER_H
#define VIEWERMARKHANDLER_H

#include <QObject>
#include "ui_ViewerEditNotePopup.h"
#include "Viewer.h"
#include "ViewerDelimiter.h"

class QWidget;
class ViewerTextActionsMenu;
class ViewerContentsPopup;
class ViewerTextActionsPopup;
class ViewerEditNotePopup;
class ViewerNoteActionsPopup;
class BookLocation;
class QString;
class QStringList;
class QDocView;
class QPoint;


class ViewerMarkHandler : public QObject
{
    Q_OBJECT

public:
    ViewerMarkHandler(Viewer*);
    virtual ~ViewerMarkHandler();

    void                            setCurrentDoc                           ( QDocView* );
    void                            setCurrentMark                          ( BookLocation* );

    void                            addTempHighlight                        ( const QString& );
    void                            applyMarks                              ();
    void                            reloadMarks                             ();

    bool                            isPointingText                          ( const QPoint& ) const;
    int                             isHighlighted                           ( const QPoint& ) const;
    int                             startHighlight                          ( const QPoint& );
    int                             trackHighlight                          ( const QPoint& );
    int                             finishHighlight                         ( const QPoint& );

    void                            showTextActions                         ( HighlightPopupType action, int hiliIndex, const QPoint& endPoint, const QRect& bbox, const QString& word = 0, const bool isPdf = false);
    void                            hideTextDelimiters                      ();
    void                            hideTextActions                         ();

protected:
    void                            showTextActionsMenu                     ( int hiliIndex, const QPoint& endPoint, const QRect& bbox );
    void                            showTextActionsPopup                    ( int hiliIndex, const QPoint& endPoint, const QRect& bbox, const QString& word);

signals:
    void                            numPageMarks                            (int, int);
    void                            dictioSearchReq                         (QStringList);
    void                            wikiSearchReq                           (QString);
    void                            bookSearchReq                           (const QString&);
    void                            markShareReq                            (BookLocation*);
    void                            setBookmark                             (bool);

public slots:
    void                            handleMarksPerPage                      ();
    void                            handleBookmark                          ();
    void                            cancelMarkOp                            ();

protected slots:
    void                            handleNoteAction                        (bool fromContentList = false);
    void                            handleChangeNoteToHighlight             ();
    void                            handleHighlightAction                   ();
    void                            handleSearchAction                      ();
    void                            handleDeleteAction                      (bool fromContentList = false);
#ifndef HACKERS_EDITION
    void                            handleDictioAction                      ();
#endif
    void                            handleWikiAction                        ();

    void                            saveNote                                ();
    void                            searchWordRequested                     (const QString& word, const QString &context);

    void                            handleDelimiterPressEvent               (QPoint const eventPoint);
    void                            handleDelimiterMoveEvent                (QPoint const eventPoint);
    void                            handleDelimiterRelesaseEvent            (QPoint const eventPoint);

protected:
    void                            closeEditNote                           ();
    void                            clearInitialPoint                       ();
    QString                         getNoteText                             () const;

    Viewer*                         m_parentViewer;
    QDocView*                       m_currentDoc;
    QString                         qs_docPath;

    BookLocation*                   m_currentMark;
    int                             m_currentHighlightIndex;
    QPoint                          m_initialPoint;
    QString                         m_memNote;
    bool                            m_modifyingMark;
    QRect                           m_tempHighlightRect;
    QPoint                          m_highlightHeights;
    QPoint                          m_staticHighlightPoint;
    bool                            m_movingRightDelimiter;
    bool                            m_firstDelimiterMoveEvent;
    int                             m_lastStartTrackHiliID;

    // Customized Ui objects
    ViewerEditNotePopup*            m_viewerEditNote;
    ViewerTextActionsMenu*          m_viewerTextActionsMenu;
    ViewerTextActionsPopup*         m_viewerTextActionPopup;
    ViewerNoteActionsPopup*         m_viewerNoteActionsPopup;
    ViewerDelimiter*                m_viewerDelimiters[2];
    bool                            m_fromContentList;

};

#endif // VIEWERMARKHANDLER_H



