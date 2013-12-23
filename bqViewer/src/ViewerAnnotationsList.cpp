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


#include "ViewerAnnotationsList.h"

#include "Viewer.h"
#include "Screen.h"
#include "QBook.h"
#include "QBookApp.h"
#include "BookLocation.h"
#include "ViewerAnnotationActions.h"

#include <QButtonGroup>
#include <QPainter>
#include <QWidget>
#include <QDebug>
#include <QKeyEvent>

const int itemsPerPage = 4;


#define CHECKED "background-color:#EDEDED;"
#define UNCHECKED "border-style:solid;border-left:none;border-bottom:none;border-top:none;border-color:#808080;background-color:#FFFFFF;"

ViewerAnnotationsList::ViewerAnnotationsList(QWidget* parent) :
    ViewerContentsPopupWidget(parent)
  , m_locationsList(NULL)
{
    setupUi(this);
    qDebug() << Q_FUNC_INFO;

    m_marksList.append(Annotation1);
    m_marksList.append(Annotation2);
    m_marksList.append(Annotation3);
    m_marksList.append(Annotation4);

    m_actions = new ViewerAnnotationActions(this);
    m_actions->hide();

    connect(m_actions, SIGNAL(goToMark(BookLocation*)), this, SLOT(goToMark(BookLocation*)));
    connect(m_actions, SIGNAL(editMark(BookLocation*)), this, SLOT(editMark(BookLocation*)));
    connect(m_actions, SIGNAL(deleteMark(BookLocation*)), this, SLOT(deleteMark(BookLocation*)));

    for(int i = 0; i < itemsPerPage; i++) {
        connect(m_marksList.at(i), SIGNAL(handleTap(const QString&)), this, SLOT(handleTap(const QString&)));
        connect(m_marksList.at(i), SIGNAL(longPress(BookLocation*)), this, SLOT(handleLongPress(BookLocation*)));
    }

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->addButton(allAnnotationsBtn, BookLocation::ALL_IN_BOOK);
    m_buttonGroup->addButton(bookmarksAnnotationsBtn, BookLocation::BOOKMARK);
    m_buttonGroup->addButton(highlightedAnnotationsBtn, BookLocation::HIGHLIGHT);
    m_buttonGroup->addButton(notesAnnotationsBtn, BookLocation::NOTE);

    connect(m_buttonGroup, SIGNAL(buttonPressed(int)), this, SLOT(paintMarks(int)));

    m_currentView = BookLocation::ALL_IN_BOOK;
}

ViewerAnnotationsList::~ViewerAnnotationsList()
{
    qDebug() << Q_FUNC_INFO;

    delete m_locationsList;
    delete m_actions;

    delete m_buttonGroup;
    m_buttonGroup = NULL;
}

void ViewerAnnotationsList::setBook( const BookInfo* book )
{
    delete m_locationsList;
    m_locationsList = book->getLocationList();
    m_actions->hide();

    // Fill the types count
    for(int i = 0; i < BookLocation::TYPE_COUNT; ++i)
        m_locationsTypeCount[i] = 0;

    m_locationsPaintList.clear();
    QList<const BookLocation*>::const_iterator it = m_locationsList->constBegin();
    QList<const BookLocation*>::const_iterator itEnd = m_locationsList->constEnd();
    for(; it != itEnd; ++it)
    {
        m_locationsTypeCount[BookLocation::ALL_IN_BOOK] = m_locationsTypeCount[BookLocation::ALL_IN_BOOK] + 1;
        m_locationsTypeCount[(*it)->type] = m_locationsTypeCount[(*it)->type] + 1;

        if( m_currentView == BookLocation::ALL_IN_BOOK || (*it)->type == m_currentView )
            m_locationsPaintList.append(*it);
    }

    setupGUI(book->format);
}

void ViewerAnnotationsList::setupGUI(const QString& extension)
{
    if(extension == "pdf" || Viewer::isUsingCR3(extension))
        setBookmarksCheck();
    else
    {
        if(m_currentView != BookLocation::ALL_IN_BOOK)
            return;
        else
            setAllNotesCheck();
    }
}

void ViewerAnnotationsList::handleTap(const QString& ref)
{
    if(hideActionsPopup())
        return;
    emit navigateToMark(ref);
}

void ViewerAnnotationsList::handleLongPress(BookLocation * location)
{
    if(hideActionsPopup())
        return;

    switch(location->type){
    case BookLocation::BOOKMARK:
        m_actions->setBookmark(location);
        break;
    case BookLocation::HIGHLIGHT:
    case BookLocation::NOTE:
        m_actions->setNote(location);
        break;
    }
    for(int i = 0; i < itemsPerPage; i++) {
        if(m_marksList.at(i)->getLocation() == location){
            Screen::getInstance()->queueUpdates();
            int x = ((m_marksList.at(i)->width() - m_actions->getPopUpWidth()))>>1;
            int y = m_marksList.at(i)->mapToParent(QPoint(0,0)).y() + annotationActions->height() + ((m_marksList.at(i)->height() - m_actions->getPopUpHeight())>>1);
            m_actions->move(x, y);
            m_actions->show();
            Screen::getInstance()->flushUpdates();
            break;
        }
    }
}

void ViewerAnnotationsList::goToMark(BookLocation* location)
{
    hideActionsPopup();
    emit navigateToMark(location->bookmark);
}

void ViewerAnnotationsList::deleteMark(BookLocation* location)
{
    hideActionsPopup();
    emit deleteNote(location);
}

void ViewerAnnotationsList::editMark(BookLocation* location)
{
    hideActionsPopup();
    emit editNote(location);
}

void ViewerAnnotationsList::paintMarks( int type )
{
    qDebug() << Q_FUNC_INFO;

    if(hideActionsPopup())
        return;

    if(m_currentView == type)
        return;

    Screen::getInstance()->queueUpdates();
    m_buttonGroup->button(type)->setStyleSheet(CHECKED);
    m_buttonGroup->button((int)m_currentView)->setStyleSheet(UNCHECKED);

    m_currentView = (BookLocation::Type)type;

    for(int i = 0; i < itemsPerPage; ++i)
    {
        m_marksList.at(i)->hide();
    }

    m_locationsPaintList.clear();
    QList<const BookLocation*>::const_iterator it = m_locationsList->constBegin();
    QList<const BookLocation*>::const_iterator itEnd = m_locationsList->constEnd();
    for(; it != itEnd; ++it)
    {
        if( m_currentView == BookLocation::ALL_IN_BOOK || (*it)->type == m_currentView )
            m_locationsPaintList.append(*it);
    }

    emit resetPage();
    Screen::getInstance()->flushUpdates();
}

void ViewerAnnotationsList::paint( int currentPage)
{
    qDebug() << Q_FUNC_INFO;

    hideActionsPopup();

    int size = m_locationsPaintList.size();
    int offset = currentPage * itemsPerPage;
    int itemIdx = 0;
    int notItemIdx = 0;
    while(itemIdx < itemsPerPage)
    {
        qDebug() << Q_FUNC_INFO << offset << itemIdx << notItemIdx;
        int pos = offset + itemIdx + notItemIdx;
        if(pos < size)
        {
            const BookLocation* location = m_locationsPaintList.at(pos);
            if( m_currentView == BookLocation::ALL_IN_BOOK || location->type == m_currentView )
            {
                m_marksList.at(itemIdx)->setItem(location);
                m_marksList.at(itemIdx)->show();
                ++itemIdx;
            }
            else
                ++notItemIdx;
        }
        else
        {
            m_marksList.at(itemIdx)->hide();
            ++itemIdx;
        }
    }
}

void ViewerAnnotationsList::paintEvent (QPaintEvent *)
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

int ViewerAnnotationsList::getTotalPages() const
{
    int items = m_locationsTypeCount[m_currentView];

    if( items % itemsPerPage == 0)
        return items / itemsPerPage;
    else
        return (items / itemsPerPage) + 1;
}

bool ViewerAnnotationsList::hideActionsPopup()
{
    if(m_actions->isVisible()){
        Screen::getInstance()->queueUpdates();
        m_actions->hide();
        Screen::getInstance()->flushUpdates();
        return true;
    }
    else
        return false;
}

void ViewerAnnotationsList::setCurrentView()
{
    m_currentView = BookLocation::ALL_IN_BOOK;
    setAllNotesCheck();
}

void ViewerAnnotationsList::mouseReleaseEvent(QMouseEvent* event)
{
    qDebug() << Q_FUNC_INFO;
    if(hideActionsPopup())
        event->accept();
}

void ViewerAnnotationsList::setBookmarksCheck()
{
    qDebug() << Q_FUNC_INFO;
    bookmarksAnnotationsBtn->setStyleSheet(CHECKED);

    allAnnotationsBtn->setEnabled(false);
    highlightedAnnotationsBtn->setEnabled(false);
    notesAnnotationsBtn->setEnabled(false);

    allAnnotationsBtn->setStyleSheet(UNCHECKED);
    highlightedAnnotationsBtn->setStyleSheet(UNCHECKED);
    notesAnnotationsBtn->setStyleSheet(UNCHECKED);

    m_currentView = BookLocation::BOOKMARK;
}

void ViewerAnnotationsList::setAllNotesCheck()
{
    qDebug() << Q_FUNC_INFO;
    allAnnotationsBtn->setStyleSheet(CHECKED);

    allAnnotationsBtn->setEnabled(true);
    highlightedAnnotationsBtn->setEnabled(true);
    notesAnnotationsBtn->setEnabled(true);

    bookmarksAnnotationsBtn->setStyleSheet(UNCHECKED);
    highlightedAnnotationsBtn->setStyleSheet(UNCHECKED);
    notesAnnotationsBtn->setStyleSheet(UNCHECKED);
    m_currentView = BookLocation::ALL_IN_BOOK;
}
