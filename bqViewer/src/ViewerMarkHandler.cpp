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

#include "ViewerMarkHandler.h"

#include "QBookApp.h"
#include "Viewer.h"
#include "QDocView.h"
#include "BookInfo.h"
#include "BookLocation.h"

#include "Dictionary.h"
#include "Screen.h"
#include "Keyboard.h"

#include "ViewerEditNotePopup.h"
#include "ViewerTextActionsMenu.h"
#include "ViewerTextActionsPopup.h"
#include "ViewerNoteActionsPopup.h"
#include "ViewerContentsPopup.h"
#include "ViewerDelimiter.h"

#include <QDebug>
#include <QTextEdit>
#include <QElapsedTimer>
#include <QtCore/qmath.h>

#define NUMDICCONTEXTWORDS 4

ViewerMarkHandler::ViewerMarkHandler(Viewer* parent) : QObject(parent)
  , m_parentViewer(parent)
  , m_currentDoc(NULL)
  , m_currentMark(NULL)
  , m_viewerEditNote(NULL)
  , m_viewerTextActionsMenu(NULL)
  , m_fromContentList(false)
  , m_modifyingMark(false)
  , m_highlightHeights(QPoint(0, 0))
{
    qDebug() << Q_FUNC_INFO;

    m_viewerTextActionsMenu = new ViewerTextActionsMenu(m_parentViewer);
    connect(m_viewerTextActionsMenu, SIGNAL(notesActionClicked()),      this, SLOT(handleNoteAction()));
    connect(m_viewerTextActionsMenu, SIGNAL(highlightActionClicked()),  this, SLOT(handleHighlightAction()));
    connect(m_viewerTextActionsMenu, SIGNAL(searchActionClicked()),     this, SLOT(handleSearchAction()));
#ifndef HACKERS_EDITION
    connect(m_viewerTextActionsMenu, SIGNAL(deleteActionClicked()),     this, SLOT(handleDeleteAction()));
    connect(m_viewerTextActionsMenu, SIGNAL(dictioActionClicked()),     this, SLOT(handleDictioAction()));
#endif
    m_viewerTextActionsMenu->hide();

    m_viewerTextActionPopup = new ViewerTextActionsPopup(m_parentViewer);
    connect(m_viewerTextActionPopup, SIGNAL(hideMe()),                      this, SLOT(cancelMarkOp()));
    connect(m_viewerTextActionPopup, SIGNAL(createNoteClicked()),           this, SLOT(handleNoteAction()));
    connect(m_viewerTextActionPopup, SIGNAL(highlightedClicked()),          this, SLOT(handleHighlightAction()));
    connect(m_viewerTextActionPopup, SIGNAL(searchClicked()),               this, SLOT(handleSearchAction()));
#ifndef HACKERS_EDITION
    connect(m_viewerTextActionPopup, SIGNAL(completeDefinitionClicked()),   this, SLOT(handleDictioAction()));
#endif
    connect(m_viewerTextActionPopup, SIGNAL(wordToSearch(const QString&, const QString&)),  this, SLOT(searchWordRequested(const QString&, const QString& )));
    m_viewerTextActionPopup->hide();

    m_viewerEditNote = new ViewerEditNotePopup(m_parentViewer);
    connect(m_viewerEditNote,        SIGNAL(saveNote()),                this, SLOT(saveNote()));
    connect(m_viewerEditNote,        SIGNAL(hideMe()),                  this, SLOT(cancelMarkOp()));
    m_viewerEditNote->hide();

    m_viewerNoteActionsPopup = new ViewerNoteActionsPopup(m_parentViewer);
    connect(m_viewerNoteActionsPopup, SIGNAL(searchActionClicked()),    this, SLOT(handleSearchAction()));
    connect(m_viewerNoteActionsPopup, SIGNAL(deleteActionClicked()),    this, SLOT(handleDeleteAction()));
    connect(m_viewerNoteActionsPopup, SIGNAL(editActionClicked()),      this, SLOT(handleNoteAction()));
    connect(m_viewerNoteActionsPopup, SIGNAL(dictioActionClicked()),    this, SLOT(handleDictioAction()));
    connect(m_viewerNoteActionsPopup, SIGNAL(hideMe()),                 m_parentViewer, SLOT(hideAllElements()));
    m_viewerNoteActionsPopup->hide();

    m_viewerDelimiters[0] = new ViewerDelimiter(m_parentViewer);
    connect(m_viewerDelimiters[0], SIGNAL(pressEvent(QPoint)),   this, SLOT(handleDelimiterPressEvent(QPoint)));
    connect(m_viewerDelimiters[0], SIGNAL(moveEvent(QPoint)),    this, SLOT(handleDelimiterMoveEvent(QPoint)));
    connect(m_viewerDelimiters[0], SIGNAL(releaseEvent(QPoint)), this, SLOT(handleDelimiterRelesaseEvent(QPoint)));
    m_viewerDelimiters[0]->hide();

    m_viewerDelimiters[1] = new ViewerDelimiter(m_parentViewer);
    connect(m_viewerDelimiters[1], SIGNAL(pressEvent(QPoint)),   this, SLOT(handleDelimiterPressEvent(QPoint)));
    connect(m_viewerDelimiters[1], SIGNAL(moveEvent(QPoint)),    this, SLOT(handleDelimiterMoveEvent(QPoint)));
    connect(m_viewerDelimiters[1], SIGNAL(releaseEvent(QPoint)), this, SLOT(handleDelimiterRelesaseEvent(QPoint)));
    m_viewerDelimiters[1]->hide();

    connect(m_parentViewer, SIGNAL(upperMarginUpdate(int)), m_viewerDelimiters[0], SLOT(viewUpperMarginUpdate(int)));
}

ViewerMarkHandler::~ViewerMarkHandler()
{
    qDebug() << Q_FUNC_INFO;

    delete m_viewerTextActionsMenu;
    m_viewerTextActionsMenu = NULL;

    delete m_viewerEditNote;
    m_viewerEditNote = NULL;

    delete m_viewerNoteActionsPopup;
    m_viewerNoteActionsPopup = NULL;

    delete m_viewerDelimiters[0];
    m_viewerDelimiters[0] = NULL;

    delete m_viewerDelimiters[1];
    m_viewerDelimiters[1] = NULL;

}

void ViewerMarkHandler::setCurrentDoc(QDocView* doc)
{
    qDebug() << Q_FUNC_INFO;
    m_currentDoc = doc;
    applyMarks();
}

void ViewerMarkHandler::setCurrentMark(BookLocation* location)
{
    qDebug() << Q_FUNC_INFO;
    m_currentMark = location;
}

void ViewerMarkHandler::addTempHighlight(const QString& ref)
{
    qDebug() << Q_FUNC_INFO << ref;
    QStringList locations;
    m_parentViewer->getCurrentBookInfo()->getLocationsAsString(locations);
    locations << ref;
    m_currentDoc->setHighlightList(locations);
}

void ViewerMarkHandler::applyMarks()
{
    qDebug() << Q_FUNC_INFO;
    QStringList locations;
    m_parentViewer->getCurrentBookInfo()->getLocationsAsString(locations);
    m_currentDoc->setHighlightList(locations);
}

void ViewerMarkHandler::reloadMarks()
{
    qDebug() << Q_FUNC_INFO;
    m_currentDoc->clearHighlightList();
    applyMarks();
    handleMarksPerPage();
}

void ViewerMarkHandler::handleMarksPerPage()
{
    qDebug() << Q_FUNC_INFO << m_currentDoc;

    QElapsedTimer timer;
    timer.start();

    qDebug() << Q_FUNC_INFO << "Start at " << timer.elapsed() << " milliseconds";

    double startPos, endPos;
    int bookmarksNum = 0; // BOOKMARKS
    int highlightsNum = 0; // HIGHLIGHTS
    int notesNum = 0; // NOTES
    bool markBeforeNextPage = false;

    if(!m_currentDoc || !m_parentViewer) return;

    bool isCR3Format = Viewer::isUsingCR3((Viewer::SupportedExt)m_parentViewer->getCurrentDocExt());

    m_currentDoc->getBookmarkRange(&startPos, &endPos);

    const QHash<QString, BookLocation*>& locations = m_parentViewer->getCurrentBookInfo()->getLocations();
    QHash<QString, BookLocation*>::const_iterator it = locations.constBegin();
    QHash<QString, BookLocation*>::const_iterator itEnd = locations.constEnd();
    while(it != itEnd)
    {
        if(it.value()->type == BookLocation::BOOKMARK && it.value()->operation != BookLocation::DELETE)
        {
            if(m_parentViewer->isUsingCR3((Viewer::SupportedExt)m_parentViewer->getCurrentDocExt()))
            {
                if(m_currentDoc->getPageFromMark(it.value()->bookmark) == m_currentDoc->pageNumberForScreen())
                    ++bookmarksNum;
            }
            else
            {
                if(it.value()->pos <= 0)
                    it.value()->pos = m_currentDoc->getPosFromBookmark(it.value()->bookmark);

                if((startPos <= it.value()->pos || fabs(startPos - it.value()->pos) <= 0.000001) && it.value()->pos < endPos) // Mark in the range
                   ++bookmarksNum;
            }
        }

        if(it.value()->type == BookLocation::NOTE && it.value()->operation != BookLocation::DELETE)
        {
            if(it.value()->pos <= 0)
                it.value()->pos = m_currentDoc->getPosFromHighlight(it.value()->bookmark);

            if(isCR3Format) markBeforeNextPage = it.value()->pos < endPos;
            else markBeforeNextPage = it.value()->pos <= endPos;

            if((startPos <= it.value()->pos || fabs(startPos - it.value()->pos) <= 0.000001) && markBeforeNextPage) // Mark in the range
                ++notesNum;
        }

        if(it.value()->type == BookLocation::HIGHLIGHT && it.value()->operation != BookLocation::DELETE)
        {
            if(it.value()->pos <= 0)
                it.value()->pos = m_currentDoc->getPosFromHighlight(it.value()->bookmark);

            if(isCR3Format) markBeforeNextPage = it.value()->pos < endPos;
            else markBeforeNextPage = it.value()->pos <= endPos;

            if((startPos <= it.value()->pos || fabs(startPos - it.value()->pos) <= 0.000001) && markBeforeNextPage) // Mark in the range
                ++highlightsNum;
        }
            ++it;
    }

    qDebug() << Q_FUNC_INFO << "Finish at " << timer.elapsed() << " milliseconds";

    // Bookmarks
    if (bookmarksNum>0) // Bookmark found
        emit setBookmark(true);
    else
        emit setBookmark(false);

    emit numPageMarks(notesNum,highlightsNum);
}

bool ViewerMarkHandler::isPointingText(const QPoint& startPoint) const
{
    QString pointedWord = m_currentDoc->wordAt(startPoint.x(),startPoint.y());
    qDebug() << Q_FUNC_INFO << pointedWord;

    if(pointedWord == "")
        return false;
    else
        return true;
}

int ViewerMarkHandler::isHighlighted(const QPoint& pressedPoint) const
{
    qDebug() << Q_FUNC_INFO;

    return m_currentDoc->highlightAt(pressedPoint.x(),pressedPoint.y());
}

int ViewerMarkHandler::startHighlight(const QPoint& initialPoint)
{
    qDebug() << Q_FUNC_INFO;

    m_initialPoint = initialPoint;

    int hiliIndex = m_currentDoc->trackHighlight(QDocView::HIGHLIGHT_BEGIN, initialPoint.x(), initialPoint.y());

    // Plus 10 in initialPoint.x() to avoid segmentation fault when creating a note just by longpressing and releasing
    // without movement to select something
    //TODO: review this mess
//    int hiliIndex = m_currentDoc->trackHighlight(QDocView::HIGHLIGHT_TRACK, initialPoint.x()+10, initialPoint.y()); TODO: ¿esto por qué?

    qDebug() << "hiliIndex" << hiliIndex;

//    QStringList listW = m_currentDoc->wordAt(m_initialPoint.x(), m_initialPoint.y(), 3);
//    qs_roundingTextLeft = listW[1];

    return hiliIndex;
}

int ViewerMarkHandler::trackHighlight(const QPoint& movingPoint)
{
    qDebug() << Q_FUNC_INFO;

    return m_currentDoc->trackHighlight(QDocView::HIGHLIGHT_TRACK, movingPoint.x(), movingPoint.y());
}

int ViewerMarkHandler::finishHighlight(const QPoint& endPoint)
{
    qDebug() << Q_FUNC_INFO;

    int hiliId = m_currentDoc->trackHighlight(QDocView::HIGHLIGHT_END, endPoint.x(), endPoint.y());
    qDebug() << "END" << hiliId;

    if(hiliId < 0)
    {
        qDebug() << Q_FUNC_INFO << "Highlight Out of bounds. Return -1";
        return -1;
    }

    m_currentDoc->trackHighlight(QDocView::HIGHLIGHT_COMMIT, endPoint.x(), endPoint.y());
    qDebug() << "COMMIT" << hiliId;

    return hiliId;
}

void ViewerMarkHandler::saveNote()
{
    qDebug() << Q_FUNC_INFO << "m_currentHighlightIndex" << m_currentHighlightIndex;

    Screen::getInstance()->queueUpdates();

    QString noteText(getNoteText()); // Read entered note
    if(noteText.size() == 0) {
        if(m_fromContentList)
            handleChangeNoteToHighlight();
        else
            handleHighlightAction();
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
        return;
    }
    if(!m_currentMark) // New Note
    {
        QDocView::Location* loc = m_currentDoc->highlightLocation(m_currentHighlightIndex);

        if (!loc) return;

        m_parentViewer->setCurrentChapterInfo();
        BookLocation* location = new BookLocation(BookLocation::NOTE, BookLocation::ADD, loc->preview, loc->ref, loc->pos, loc->page, noteText, QDateTime::currentDateTime().toMSecsSinceEpoch(), m_parentViewer->getChapterTitle());
        m_parentViewer->getCurrentBookInfo()->addLocation(location->bookmark, location);

        delete loc;
    }
    else // Edit old note
    {
        m_parentViewer->getCurrentBookInfo()->editNote(m_currentMark->bookmark, noteText, QDateTime::currentDateTime().toMSecsSinceEpoch());
    }


    // Update number of marks
    reloadMarks();
    m_parentViewer->endHiliMode();
    closeEditNote();
    clearInitialPoint();

    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void ViewerMarkHandler::cancelMarkOp()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();

    if((!m_currentMark || m_currentMark->operation == BookLocation::DELETE) && m_currentHighlightIndex >= 0)
    {
        m_currentDoc->removeHighlight(m_currentHighlightIndex);
        m_currentHighlightIndex = -1;
    }

    reloadMarks();
    m_parentViewer->endHiliMode();
    closeEditNote();
    clearInitialPoint();

    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void ViewerMarkHandler::searchWordRequested( const QString& word, const QString& context)
{
    qDebug() << Q_FUNC_INFO << " Word to search: " << word;

#ifndef HACKERS_EDITION
    if(!QBookApp::instance()->isActivated() && !QBookApp::instance()->isLinked())
        return;

    if(!word.isEmpty())
    {
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        m_parentViewer->hideAllElements();

        QStringList wordList;
        wordList << word << context << "";

        emit dictioSearchReq(wordList);

        if(!m_currentMark)
        {
            qDebug() << Q_FUNC_INFO << "Remove highlight in dictionary calling to id " << m_currentHighlightIndex;
            m_currentDoc->removeHighlight(m_currentHighlightIndex);
        }
    }
#endif
    clearInitialPoint();// TODO: Revisar el reseteo al acabar todas las operaciones
}

void ViewerMarkHandler::handleBookmark()
{
    qDebug() << Q_FUNC_INFO;

    if (!m_currentDoc) return;

    double start, end;
    m_currentDoc->getBookmarkRange(&start, &end);

    QDocView::Location *loc = m_currentDoc->bookmark();

    if (!loc) return;

    const QHash<QString, BookLocation*>& locations = m_parentViewer->getCurrentBookInfo()->getLocations();
    QHash<QString, BookLocation*>::const_iterator it = locations.constBegin();
    QHash<QString, BookLocation*>::const_iterator itEnd = locations.constEnd();
    bool found = false;
    double markPos = 0;

    while(it != itEnd)
    {
        if(it.value()->type == BookLocation::BOOKMARK)
        {
            markPos = it.value()->pos;
            if(markPos <= 0)
            {
                if(m_parentViewer->getCurrentBookInfo()->locationsPosCache.contains(it.value()->bookmark) && m_parentViewer->getCurrentBookInfo()->locationsPosCache.value(it.key()) > 0)
                {
                    markPos = m_parentViewer->getCurrentBookInfo()->locationsPosCache.value(it.key());
                    it.value()->pos = markPos;
                }else{
                    markPos = m_currentDoc->getPosFromBookmark(it.key());
                    it.value()->pos = markPos;
                }
            }

            if ((it.value()->operation != BookLocation::DELETE) && (start < markPos || fabs(start - markPos) <= 0.000001) && markPos < end) {
                qDebug() << " MARKED";
                it.value()->lastUpdated = QDateTime::currentDateTime().toMSecsSinceEpoch();
                m_parentViewer->getCurrentBookInfo()->setMarktoRemove(it.value());
                emit setBookmark(false);
                found = true;
                break;
            }
        }
        ++it;
    }
    if(!found)
    {
        qDebug() << "UNMARKED";
        m_parentViewer->setCurrentChapterInfo();
        BookLocation* location = new BookLocation(BookLocation::BOOKMARK, BookLocation::ADD, loc->preview, loc->ref, loc->pos, loc->page, QDateTime::currentDateTime().toMSecsSinceEpoch(), m_parentViewer->getChapterTitle());
        m_parentViewer->getCurrentBookInfo()->setMarktoAdd(location);
        emit setBookmark(true);
    }

    delete loc;
}

void ViewerMarkHandler::closeEditNote()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    if(!m_fromContentList)
        m_parentViewer->hideAllElements();
    else {
        m_viewerEditNote->hide();
        m_parentViewer->repaintContentList();
    }

    m_fromContentList = false;
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void ViewerMarkHandler::clearInitialPoint()
{
    m_initialPoint.setX(0);
    m_initialPoint.setY(0);
}

QString ViewerMarkHandler::getNoteText() const
{
    qDebug() << Q_FUNC_INFO;
    return m_viewerEditNote->getNoteText();
}

void ViewerMarkHandler::hideTextDelimiters()
{
    if(m_viewerDelimiters[0] && m_viewerDelimiters[0]->isVisible())
        m_viewerDelimiters[0]->hide();

    if(m_viewerDelimiters[1] && m_viewerDelimiters[1]->isVisible())
        m_viewerDelimiters[1]->hide();
}

void ViewerMarkHandler::showTextActions( HighlightPopupType action, int hiliIndex, const QPoint& endPoint, const QRect& bbox, const QString& word, const bool isPdf)
{
    qDebug() << Q_FUNC_INFO;

    m_highlightHeights = m_currentDoc->getHighlightHeight(hiliIndex);

    if(m_highlightHeights.x() == 0 || m_highlightHeights.y() == 0)
            m_highlightHeights = m_currentDoc->getTempHighlightHeight();

    if(!isPdf)
    {
        m_viewerDelimiters[0]->showIn(bbox, m_highlightHeights.x(), true);
        m_viewerDelimiters[1]->showIn(bbox, m_highlightHeights.y(), false);
    }

    switch (action)
    {
        case INIT_SINGLE_WORD_POPUP:
        if(isPdf)
            showTextActionsPopup(hiliIndex, bbox.topLeft(), bbox, word); // To ensure the correct popup placement.
        else
            showTextActionsPopup(hiliIndex, endPoint, bbox, word);

        break;

        case DEFAULT_POPUP:
            showTextActionsMenu(hiliIndex, endPoint, bbox);
        break;
    }
}

void ViewerMarkHandler::hideTextActions()
{
    qDebug() << Q_FUNC_INFO;

    if(m_viewerTextActionsMenu && m_viewerTextActionsMenu->isVisible())
        m_viewerTextActionsMenu->hide();

    if(m_viewerTextActionPopup && m_viewerTextActionPopup->isVisible())
        m_viewerTextActionPopup->hide();

    if(m_viewerNoteActionsPopup && m_viewerNoteActionsPopup->isVisible())
        m_viewerNoteActionsPopup->hide();

    if(m_viewerEditNote && m_viewerEditNote->isVisible())
        m_viewerEditNote->hide();
}

void ViewerMarkHandler::showTextActionsMenu( int hiliIndex, const QPoint& endPoint, const QRect& bbox )
{
    qDebug() << Q_FUNC_INFO << hiliIndex << endPoint << bbox;

    if (m_initialPoint.isNull())
    {
        // We are showing the ContextMenu without startHighlight+finishhighlight
        // so we are opening a previoud highligth. We need m_initialPoint set for Dictionary search
        m_initialPoint = endPoint;
    }

    m_currentHighlightIndex = hiliIndex;
    qDebug() << Q_FUNC_INFO << "highlight id " << m_currentHighlightIndex;

    QDocView::Location* loc = m_currentDoc->highlightLocation(m_currentHighlightIndex);
    m_currentMark = m_parentViewer->getCurrentBookInfo()->getLocation(loc->ref);

    if(m_modifyingMark && m_currentMark)
    {
        m_modifyingMark = false;

        if( !m_memNote.isEmpty())
        {
            m_currentMark->type = BookLocation::NOTE;
            m_currentMark->note = m_memNote;
            m_memNote.clear();
            m_modifyingMark = false;
        }
    }

    bool oneWord = loc->preview.split(QRegExp("\\s")).count() == 1;
    if( /*!m_modifyingMark &&*/
       (!m_currentMark || m_currentMark->operation == BookLocation::DELETE) )
    {
        if(oneWord)
            showTextActionsPopup(m_currentHighlightIndex, endPoint, bbox, loc->preview);
        else
        {
            m_viewerTextActionsMenu->setup(
                        true,
                        oneWord,
                        endPoint,
                        bbox,
                        m_viewerDelimiters[0]->height());
            m_parentViewer->showElement(m_viewerTextActionsMenu);
        }
    }
    else // Modifing mark
    {
        qDebug() << Q_FUNC_INFO << "m_currentMark" << m_currentMark <<  "m_currentDoc" << m_currentDoc << "hiliIndex" << hiliIndex;
        if(m_currentMark->type == BookLocation::NOTE)
        {
            m_viewerNoteActionsPopup->setup(
                        m_currentMark->note,
                        oneWord,
                        endPoint,
                        bbox,
                        m_viewerDelimiters[0]->height());
            m_parentViewer->showElement(m_viewerNoteActionsPopup);
        }
        else
        {
            m_viewerTextActionsMenu->setup(
                        false,
                        oneWord,
                        endPoint,
                        bbox,
                        m_viewerDelimiters[0]->height());
            m_parentViewer->showElement(m_viewerTextActionsMenu);
        }
    }

    delete loc;
}

void ViewerMarkHandler::showTextActionsPopup(int hiliIndex, const QPoint& endPoint, const QRect& bbox, const QString& word)
{
    qDebug() << Q_FUNC_INFO << "hiliIndex: " << hiliIndex << ", endPoint: " << endPoint;

    m_currentHighlightIndex = hiliIndex;
    qDebug() << Q_FUNC_INFO << "highlight id " << m_currentHighlightIndex;

    m_currentMark = NULL;// TODO: ¿no es necesaria la mark?

    QStringList wordAndContext = m_currentDoc->wordAt(endPoint.x(), endPoint.y(), NUMDICCONTEXTWORDS);
    qDebug() << "selectedWord=" << word << "context=" << wordAndContext[1] << " and " << wordAndContext [2];

    QString context = QString(wordAndContext.at(1)) + QString("<w>") + QString(wordAndContext.at(2));
#ifndef HACKERS_EDITION
    QString searchResult = Dictionary::instance()->translate(word, context);

    m_viewerTextActionPopup->setup(searchResult, endPoint, bbox, m_viewerDelimiters[0]->height());
    m_parentViewer->setInitialDefinitionSearch(word, context);
#else
    m_viewerTextActionPopup->setup("", endPoint, bbox, m_viewerDelimiters[0]->height());
#endif

    if (m_parentViewer)
        m_parentViewer->showElement(m_viewerTextActionPopup);
}

void ViewerMarkHandler::handleNoteAction(bool fromContentList)
{
    qDebug() << Q_FUNC_INFO;

    m_fromContentList = fromContentList;

    if(m_currentHighlightIndex >= m_currentDoc->highlightCount() && !m_fromContentList) // Safety check for quick page turning
    {
        qWarning() << Q_FUNC_INFO << "ERROR: FAULTY HIGHLIGHT, INDEX =" << m_currentHighlightIndex;
        return;
    }
    m_viewerTextActionsMenu->hide();
    m_viewerNoteActionsPopup->hide();
    m_viewerTextActionPopup->hide();

    if (!m_currentMark || m_currentMark->operation == BookLocation::DELETE || m_currentMark->type != BookLocation::NOTE) // Creating new Note
    {
        m_viewerEditNote->setUpCreateNote();
    }
    else // Editing old note
    {
        m_viewerEditNote->setUpEditNote(m_currentMark->note);
    }
}

void ViewerMarkHandler::handleChangeNoteToHighlight()
{
    m_currentMark->type = BookLocation::HIGHLIGHT;
    m_parentViewer->getCurrentBookInfo()->addLocation(m_currentMark->bookmark, m_currentMark);
    m_parentViewer->endHiliMode();

    // Update number of marks
    reloadMarks();

    m_currentMark = NULL;

    closeEditNote();
    clearInitialPoint();
}


void ViewerMarkHandler::handleHighlightAction()
{
    qDebug() << Q_FUNC_INFO;

    QDocView::Location* loc = m_currentDoc->highlightLocation(m_currentHighlightIndex);

    if (!loc) return;

    m_parentViewer->setCurrentChapterInfo();
    BookLocation* location = new BookLocation(BookLocation::HIGHLIGHT, BookLocation::ADD, loc->preview, loc->ref, loc->pos, loc->page, QDateTime::currentDateTime().toMSecsSinceEpoch(), m_parentViewer->getChapterTitle());
    qDebug() << Q_FUNC_INFO << "location->lastUpdated" << location->lastUpdated;

    m_parentViewer->getCurrentBookInfo()->addLocation(location->bookmark, location);
    m_parentViewer->endHiliMode();
    delete loc;

    // Update number of marks
    reloadMarks();

    m_currentMark = NULL;

    closeEditNote();// TODO: Correcto llamar a esta función?
    clearInitialPoint();
//    m_parentViewer->hideAllElements();
}

void ViewerMarkHandler::handleSearchAction()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();
    m_viewerTextActionsMenu->hide();
    m_viewerNoteActionsPopup->hide();
    m_viewerTextActionPopup->hide();

    QDocView::Location* loc = m_currentDoc->highlightLocation(m_currentHighlightIndex);
    if(loc)
    {
        if(!m_currentMark)
            m_currentDoc->removeHighlight(m_currentHighlightIndex);

        emit bookSearchReq(loc->preview);
        delete loc;
    }else
        Screen::getInstance()->flushUpdates();
}

void ViewerMarkHandler::handleDeleteAction(bool fromContentList)
{
    qDebug() << Q_FUNC_INFO;

    m_fromContentList = fromContentList;

    // Remove from model
    if(m_currentMark) // Old mark
    {
        if(m_currentMark->operation == BookLocation::ADD)
        {
            qDebug() << Q_FUNC_INFO << "deleting mark from model: " << m_currentMark->bookmark;
            m_parentViewer->getCurrentBookInfo()->removeLocation(m_currentMark->bookmark);
        }
        else
        {
            qDebug() << Q_FUNC_INFO << "set mark to delete to send it to server: " << m_currentMark->bookmark;
            m_parentViewer->getCurrentBookInfo()->setMarktoRemove(m_currentMark);
            handleMarksPerPage();
        }
    }

    // Remove from renderer
    if (m_currentHighlightIndex > 0) // Valid highlight index available
        m_currentDoc->removeHighlight(m_currentHighlightIndex);

    reloadMarks();
    m_currentMark = NULL;
    closeEditNote();
    clearInitialPoint();
}

#ifndef HACKERS_EDITION
void ViewerMarkHandler::handleDictioAction()
{
    qDebug() << Q_FUNC_INFO;    

    QDocView::Location* loc = m_currentDoc->highlightLocation(m_currentHighlightIndex);
    if(!loc)
        return;

    QStringList wordAndContext = m_currentDoc->wordAt(m_initialPoint.x(), m_initialPoint.y(), NUMDICCONTEXTWORDS);

    if(wordAndContext.size() != 3) return;

    wordAndContext[0] = loc->preview;
    m_parentViewer->resetInitialDefinitionSearch();

    qDebug() << "selectedWord=" << wordAndContext[0] << "context=" << wordAndContext[1] << " and " << wordAndContext [2];

    clearInitialPoint();

    emit dictioSearchReq(wordAndContext);

    // TODO: Solve SEGMETATION FAULT HERE the second time you do it
    if(!m_currentMark)
    {
        qDebug() << Q_FUNC_INFO << "Remove highlight in dictionary calling to id " << m_currentHighlightIndex;
        m_currentDoc->removeHighlight(m_currentHighlightIndex);
    }

}
#endif

void ViewerMarkHandler::handleDelimiterPressEvent(QPoint const eventPoint)
{
    qDebug() << Q_FUNC_INFO;

    m_modifyingMark = false;

    QPoint dynamicHighlightPoint = eventPoint;

    if((m_staticHighlightPoint = m_viewerDelimiters[0]->getHighlightPoint()) == eventPoint)
    {
        m_staticHighlightPoint = m_viewerDelimiters[1]->getHighlightPoint()/* - QPoint(0, 0)*/;
        //dynamicHighlightPoint += QPoint(0, 0);
        m_movingRightDelimiter = false;
    }
    else
    {
        //m_staticHighlightPoint += QPoint(0 , 0);
        //dynamicHighlightPoint -= QPoint(0, 0);
        m_movingRightDelimiter = true;
    }


    int i_dynHiliId = isHighlighted(dynamicHighlightPoint); // >= 0 if exists, It should be ever well.
    int i_statHiliId = isHighlighted(m_staticHighlightPoint);



    if(m_movingRightDelimiter)
    {
        while((i_dynHiliId = isHighlighted(dynamicHighlightPoint)) < 0) dynamicHighlightPoint -= QPoint(1, 0);
        while((i_statHiliId = isHighlighted(m_staticHighlightPoint)) < 0) m_staticHighlightPoint += QPoint(1, 0);
    }
    else
    {
        while((i_dynHiliId = isHighlighted(dynamicHighlightPoint)) < 0) dynamicHighlightPoint += QPoint(1, 0);
        while((i_statHiliId = isHighlighted(m_staticHighlightPoint)) < 0) m_staticHighlightPoint -= QPoint(1, 0);
    }


    if(i_dynHiliId < 0 || i_statHiliId < 0 || i_dynHiliId != i_statHiliId) return;

    int i_hiliId = i_dynHiliId;

    /*if((i_hiliId = i_dynHiliId) < 0)
    {
        if((i_hiliId = i_statHiliId) < 0) return;
    }*/


    m_modifyingMark = true;

    m_tempHighlightRect = m_currentDoc->getHighlightBBox(i_hiliId);

    if(m_currentMark)
    {
        m_memNote = m_currentMark->note;
        m_parentViewer->getCurrentBookInfo()->setMarktoRemove(m_currentMark);
        handleMarksPerPage();
    }

    m_currentDoc->removeHighlight(i_hiliId);
    m_currentDoc->setBlockPaintEvents(true);

    Screen::getInstance()->queueUpdates();
    Screen::getInstance()->setMode(Screen::MODE_QUICK, false, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);

    m_lastStartTrackHiliID = startHighlight(m_staticHighlightPoint);
    trackHighlight(dynamicHighlightPoint);

    // Hide popups
    hideTextActions();

    m_modifyingMark = true;
    m_firstDelimiterMoveEvent = true;
}

void ViewerMarkHandler::handleDelimiterMoveEvent(QPoint const dynamicHighlightPoint)
{
    qDebug() << Q_FUNC_INFO;

    if(!m_modifyingMark) return;

    m_currentDoc->setBlockPaintEvents(false);

    if(m_firstDelimiterMoveEvent) trackHighlight(dynamicHighlightPoint);

    // After track the point, move delimiters.
    QRect rect = m_currentDoc->getTempHighlightRect();

    // To solve the automatic selection of symbols.
    if( m_movingRightDelimiter && m_firstDelimiterMoveEvent &&
       (rect.left() != m_tempHighlightRect.left() || m_lastStartTrackHiliID == NOTHING_TRACK_ERROR))
    {
        QString word = m_currentDoc->wordAt(m_staticHighlightPoint.x(), m_staticHighlightPoint.y());

        do
        {
            m_staticHighlightPoint.rx() += 2;
        }
        while(word == m_currentDoc->wordAt(m_staticHighlightPoint.x(), m_staticHighlightPoint.y()) &&
              m_staticHighlightPoint.x() < Screen::getInstance()->screenWidth());

        int i_hiliId = finishHighlight(dynamicHighlightPoint);
        m_currentDoc->removeHighlight(i_hiliId);

        Screen::getInstance()->setMode(Screen::MODE_FAST, false, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        startHighlight(m_staticHighlightPoint);
        trackHighlight(dynamicHighlightPoint);

        rect = m_currentDoc->getTempHighlightRect();

    }
    else
    {
        trackHighlight(dynamicHighlightPoint);
    }

    if(m_firstDelimiterMoveEvent) Screen::getInstance()->flushUpdates();

    if (rect != m_tempHighlightRect) // Highlight change.
    {
        Screen::getInstance()->setMode(Screen::MODE_QUICK, false, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        m_highlightHeights = m_currentDoc->getTempHighlightHeight();
        m_viewerDelimiters[0]->showIn(rect, m_highlightHeights.x(), true);
        m_viewerDelimiters[1]->showIn(rect, m_highlightHeights.y(), false);
        m_tempHighlightRect = rect;
    }

    m_firstDelimiterMoveEvent = false;
}

void ViewerMarkHandler::handleDelimiterRelesaseEvent(QPoint const dynamicHighlightPoint)
{
    qDebug() << Q_FUNC_INFO;

    if(!m_modifyingMark) return;

    if(!m_firstDelimiterMoveEvent) Screen::getInstance()->queueUpdates();

    int i_hiliId = finishHighlight(dynamicHighlightPoint);

    if(m_modifyingMark && m_currentMark) handleHighlightAction(); // Erase old mark.

    if(i_hiliId >= 0)
    {
        QDocView::Location* loc = m_currentDoc->highlightLocation(i_hiliId);

        if( loc && !loc->preview.isEmpty())
        {

            QRect bbox = m_currentDoc->getHighlightBBox(i_hiliId);

            Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);

            m_viewerDelimiters[0]->showIn(bbox, m_highlightHeights.x(), true);
            m_viewerDelimiters[1]->showIn(bbox, m_highlightHeights.y(), false);

            m_currentDoc->setBlockPaintEvents(false);

            showTextActions(DEFAULT_POPUP, i_hiliId, dynamicHighlightPoint, bbox);

        }
        else
        {
            m_currentDoc->removeHighlight(i_hiliId);
        }
    }
    else
    {
        hideTextDelimiters();
        reloadMarks();
    }

    Screen::getInstance()->flushUpdates();

    m_firstDelimiterMoveEvent = false;
    m_modifyingMark = false;
    m_lastStartTrackHiliID = 0;

}
