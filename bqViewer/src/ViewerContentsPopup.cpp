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

#include "ViewerContentsPopup.h"

#include "QDocView.h"
#include "Screen.h"
#include "QBookApp.h"
#include "Model.h"
#include "BookInfo.h"
#include "Viewer.h"
#include "ViewerContentsList.h"
#include "ViewerAnnotationsList.h"

#include <QWidget>
#include <QDebug>
#include <QKeyEvent>
#include <QEventLoop>
#include <QTimer>
#include <QPainter>
#include <QButtonGroup>

#define CHECKED "background-color:#DADADA;"
#define ANNOTATIONS_UNCHECKED "border-right-style:solid; text-align:left; border-right-color:#C7C7C7; background-color:#FFFFFF; background:url(':/res/notes_tabs.png') no-repeat center left;"
#define CONTENTS_UNCHECKED "text-align:left; background:url(':/res/contents_tabs.png') no-repeat center left; background-color:#FFFFFF;"

ViewerContentsPopup::ViewerContentsPopup(Viewer* viewer) :
    ViewerMenuPopUp(viewer)
  , m_book(NULL)
  , m_currentView(NULL)
  , m_page(0)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);

    connect(closeBtn,       SIGNAL(clicked()), this, SIGNAL(hideMe()));
    connect(contentsTabBtn, SIGNAL(clicked()), this, SLOT(viewIndex()));
    connect(notesTabBtn,    SIGNAL(clicked()), this, SLOT(viewMarks()));
    connect(VerticalPagerPopup,  SIGNAL(nextPageReq()), this, SLOT(showNextPage()));
    connect(VerticalPagerPopup,  SIGNAL(previousPageReq()), this, SLOT(showPreviousPage()));

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->addButton(contentsTabBtn);
    m_buttonGroup->addButton(notesTabBtn);

    m_chaptersWidget = new ViewerContentsList(Content);
    connect(m_chaptersWidget,  SIGNAL(navigateToChapter(const QString&)), this, SLOT(navigateToChapter(const QString&)));
    m_chaptersWidget->hide();

    m_marksWidget = new ViewerAnnotationsList(Content);
    connect(m_marksWidget,  SIGNAL(navigateToMark(const QString&)), this, SLOT(navigateToMark(const QString&)));
    connect(m_marksWidget,  SIGNAL(editNote(BookLocation*)), this, SLOT(editNote(BookLocation*)));
    connect(m_marksWidget,  SIGNAL(deleteNote(BookLocation*)), this, SLOT(deleteNote(BookLocation*)));
    connect(m_marksWidget,  SIGNAL(resetPage()), this, SLOT(resetPage()));
    m_marksWidget->hide();

    m_currentView = m_marksWidget;
    viewIndex();
}

ViewerContentsPopup::~ViewerContentsPopup()
{
    qDebug() << Q_FUNC_INFO;

    delete m_chaptersWidget;
    m_chaptersWidget = NULL;

    delete m_marksWidget;
    m_marksWidget = NULL;

    delete m_buttonGroup;
    m_buttonGroup = NULL;
}

void ViewerContentsPopup::setup()
{
    BookInfo* book = m_parentViewer->getCurrentBookInfo();
    qDebug() << Q_FUNC_INFO << m_bookType << book->m_type;
    bool bSameBook = (m_bookPath == book->path);
    bool demoToPurchase = false;
    if(bSameBook)
        demoToPurchase = m_book && m_bookType == BookInfo::BOOKINFO_TYPE_DEMO && book->m_type == BookInfo::BOOKINFO_TYPE_PURCHASE;


    m_page = 0;
    m_book = book;
    m_bookPath = m_book->path;
    m_bookType = m_book->m_type;

    if(!bSameBook || demoToPurchase)
        initContentList();// TODO: Intentar no hacer esto en la apertura del libro. Penaliza mucho en tiempo.

    setCurrentView();
}

void ViewerContentsPopup::initContentList()
{
    qDebug() << Q_FUNC_INFO;

    m_chaptersWidget->setTableOfContent(m_parentViewer->docView()->tableOfContent());
}

void ViewerContentsPopup::clearContentList()
{
    qDebug() << Q_FUNC_INFO;

    m_chaptersWidget->clearTableOfContent();
}

void ViewerContentsPopup::setCurrentView()
{
    if(m_currentView == m_marksWidget)
        m_currentView->hide();
    m_marksWidget->setCurrentView();
    m_currentView = m_chaptersWidget;
    notesTabArrowLbl->hide();
    notesTabBtn->setStyleSheet(ANNOTATIONS_UNCHECKED);
    contentsTabArrowLbl->show();
    contentsTabBtn->setStyleSheet(CHECKED);
}

void ViewerContentsPopup::start()
{
    qDebug() << Q_FUNC_INFO;

    setCurrentChapter();
    calculateMarksPage();

    m_chaptersWidget->setLocationList(m_book->getLocationList());
    m_chaptersWidget->setBook(m_book);
    m_marksWidget->setBook(m_book);

    if(m_currentView == m_chaptersWidget)
    {
        int currentPage = m_chaptersWidget->paintFirst();
        VerticalPagerPopup->setup(m_currentView->getTotalPages(), currentPage, false);
        m_page = currentPage - 1;
    }
    else
    {
        paint();
        VerticalPagerPopup->setup(m_currentView->getTotalPages(), m_page+1, false);
    }
    m_currentView->show();
}

void ViewerContentsPopup::stop()
{}

void ViewerContentsPopup::setCurrentChapter()
{
    QString currentChapter = m_parentViewer->getCurrentChapter();
    int pageStart = m_parentViewer->getPageStart();
    m_chaptersWidget->m_currentChapter = currentChapter;
    m_chaptersWidget->m_pageStart = pageStart;
}

const QList<QDocView::Location*>& ViewerContentsPopup::getContentList() const
{
    return m_chaptersWidget->getContentList();
}

void ViewerContentsPopup::calculatePageForChapters()
{
    qDebug() << Q_FUNC_INFO;

    if (getContentList().isEmpty()) return;

    QList<QDocView::Location*>::const_iterator it = getContentList().constBegin();
    QList<QDocView::Location*>::const_iterator itEnd = getContentList().constEnd();

    while( it != itEnd )
    {
        (*it)->page = m_parentViewer->docView()->getPageFromMark((*it)->ref);
        ++it;
    }
}

void ViewerContentsPopup::showNextPage()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->lockScreen();
    Screen::getInstance()->queueUpdates();
    m_page++;
    paint();
    VerticalPagerPopup->setPage();
    Screen::getInstance()->setMode(Screen::MODE_QUICK, true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
    Screen::getInstance()->releaseScreen();
}

void ViewerContentsPopup::showPreviousPage()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->lockScreen();
    Screen::getInstance()->queueUpdates();
    m_page--;
    paint();
    VerticalPagerPopup->setPage();
    Screen::getInstance()->setMode(Screen::MODE_QUICK,true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
    Screen::getInstance()->releaseScreen();
}

void ViewerContentsPopup::navigateToChapter( const QString& ref )
{
    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(true);
    Screen::getInstance()->flushUpdates();
    qDebug() << Q_FUNC_INFO  << m_book << " ref: " << ref;
    emit goToMark(ref);
    QBookApp::instance()->getStatusBar()->setSpinner(false);
}

void ViewerContentsPopup::navigateToMark( const QString& reference )
{
    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(true);
    Screen::getInstance()->flushUpdates();
    qDebug() << Q_FUNC_INFO << reference;
    emit goToMark(reference);
    QBookApp::instance()->getStatusBar()->setSpinner(false);
}

void ViewerContentsPopup::deleteNote(BookLocation* location)
{
    m_parentViewer->setMark(location);
    emit deleteNote(true);
}

void ViewerContentsPopup::editNote(BookLocation* location)
{
    qDebug() << Q_FUNC_INFO;
    m_parentViewer->setMark(location);
    emit editNote(true);
}

void ViewerContentsPopup::repaint(){
    m_marksWidget->setBook(m_parentViewer->getCurrentBookInfo());
    Screen::getInstance()->queueUpdates();
    if(m_page > m_currentView->getTotalPages()-1)
        m_page--;
    if(m_page < 0)
        m_page = 0;
    VerticalPagerPopup->setup(m_currentView->getTotalPages(), m_page+1, false);
    paint();
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void ViewerContentsPopup::resetPage()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->lockScreen();
    Screen::getInstance()->queueUpdates();
    m_page = 0;
    VerticalPagerPopup->setup(m_currentView->getTotalPages(), 1, false);
    paint();
    Screen::getInstance()->setMode(Screen::MODE_QUICK, true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
    Screen::getInstance()->releaseScreen();
}

void ViewerContentsPopup::paint()
{
    qDebug() << Q_FUNC_INFO;

    m_currentView->paint(m_page);
}

void ViewerContentsPopup::viewIndex()
{
    qDebug() << Q_FUNC_INFO;

    if(m_currentView == m_chaptersWidget)
        return;

    Screen::getInstance()->lockScreen();

    notesTabArrowLbl->hide();
    notesTabBtn->setStyleSheet(ANNOTATIONS_UNCHECKED);
    contentsTabArrowLbl->show();
    contentsTabBtn->setStyleSheet(CHECKED);
    m_marksWidget->hideActionsPopup();

    m_currentView->hide();

    m_currentView = m_chaptersWidget;
    int currentPage = m_chaptersWidget->paintFirst();
    VerticalPagerPopup->setup(m_currentView->getTotalPages(), currentPage, false);
    m_page = currentPage - 1;
    m_currentView->show();

    Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->releaseScreen();
}

void ViewerContentsPopup::viewMarks()
{
    qDebug() << Q_FUNC_INFO;

    if(m_currentView == m_marksWidget)
        return;

    qDebug()<< Q_FUNC_INFO;
    Screen::getInstance()->lockScreen();

    contentsTabArrowLbl->hide();
    contentsTabBtn->setStyleSheet(CONTENTS_UNCHECKED);
    notesTabArrowLbl->show();
    notesTabBtn->setStyleSheet(CHECKED);

    m_currentView->hide();

    m_currentView = m_marksWidget;
    m_page = 0;
    VerticalPagerPopup->setup(m_currentView->getTotalPages(), 1, false);
    paint();
    m_currentView->show();

    Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->releaseScreen();
}

void ViewerContentsPopup::calculateMarksPage()
{
    qDebug() << Q_FUNC_INFO;

    QHash<QString, BookLocation*>::const_iterator it = m_book->getLocations().constBegin();
    QHash<QString, BookLocation*>::const_iterator itEnd = m_book->getLocations().constEnd();

    while( it != itEnd )
    {
        if(it.value()->page == 0 || (m_parentViewer->isUsingCR3((Viewer::SupportedExt)m_parentViewer->getCurrentDocExt())))
            it.value()->page = m_parentViewer->docView()->getPageFromMark(it.value()->bookmark);

        if(it.value()->pos == 0)
        {
            if(it.value()->type == BookLocation::BOOKMARK)
                it.value()->pos = m_parentViewer->docView()->getPosFromBookmark(it.value()->bookmark);
            else
                it.value()->pos = m_parentViewer->docView()->getPosFromHighlight(it.value()->bookmark);
        }

        ++it;
    }
}

void ViewerContentsPopup::paintEvent (QPaintEvent *)
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

