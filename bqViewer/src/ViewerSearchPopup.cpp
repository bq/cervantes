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

#include "ViewerSearchPopup.h"

#include "QBookApp.h"
#include "Viewer.h"
#include "ViewerMarkHandler.h"
#include "Screen.h"
#include "Keyboard.h"
#include "PowerManager.h"
#include "ViewerSearchResultItem.h"
#include "ViewerSearchContextMenu.h"
#include "ViewerSearchContextMenuLandscape.h"
#include "bqUtils.h"

#include <QPainter>
#include <QMutexLocker>
#include <QTimer>
#include <QThread>
#include <QFuture>

#define SEARCH_GOTO_FREQ 500
#define SEARCH_PREVIEW_MAX_CHARS 65

ViewerSearchPopup::ViewerSearchPopup( Viewer* viewer ) :
    ViewerMenuPopUp(viewer)
  , m_navigationMenu(NULL)
  , m_navigationMenuLandscape(NULL)
  , i_currentShownResultIndex(1)
  , m_keyboard(NULL)
  , m_userTyped(false)
  , m_isSearchActive(false)
  , m_alreadyPainted(false)
  , m_finishSearch(false)
  , b_pendingGoto(false)
  , m_currentSearch(NULL)
  , m_page(0)
  , m_totalPages(0)
  ,m_timer_gotoSearch(new QTimer(this))
{
    qDebug() << Q_FUNC_INFO;

    setupUi(this);

    m_initialSearchText = searchLineEdit->text();

    connect(clearSearchBtn, SIGNAL(clicked()),              this, SLOT(handleClearEditLineClicked()));
    connect(closeBtn,       SIGNAL(clicked()),              this, SLOT(handleCloseSearchClicked()));
    connect(searchLineEdit, SIGNAL(clicked()),              this, SLOT(handleSearchLineEditClicked()));

    connect(this,           SIGNAL(newPageReady()),         this, SLOT(updateCounters()));
    connect(this,           SIGNAL(searchFinished()),       this, SLOT(updateCounters()));

    connect(VerticalPagerPopup, SIGNAL(previousPageReq()),  this, SLOT(previousPage()));
    connect(VerticalPagerPopup, SIGNAL(nextPageReq()),      this, SLOT(nextPage()));
    connect(VerticalPagerPopup, SIGNAL(firstPage()),        this, SLOT(firstPage()));
    connect(VerticalPagerPopup, SIGNAL(lastPage()),         this, SLOT(lastPage()));
    VerticalPagerPopup->hide();

    m_uiSearchItems[0] = SearchResultItem1;
    m_uiSearchItems[1] = SearchResultItem2;
    m_uiSearchItems[2] = SearchResultItem3;
    m_uiSearchItems[3] = SearchResultItem4;
    m_uiSearchItems[4] = SearchResultItem5;

    for(int i = 0; i < RESULTS_PER_PAGE; ++i)
    {
        connect(m_uiSearchItems[i], SIGNAL(goToUrl(const QString&)), this, SLOT(handleResultPressed(const QString&)));
    }

    // Navigation queue
    m_timer_gotoSearch->setInterval(SEARCH_GOTO_FREQ);
    connect(m_timer_gotoSearch, SIGNAL(timeout()), this, SLOT(navigateToPending()));

    m_navigationMenu = new ViewerSearchContextMenu(m_parentViewer);
    m_navigationMenu->hide();
    m_navigationMenuLandscape = new ViewerSearchContextMenuLandscape(m_parentViewer);
    m_navigationMenuLandscape->hide();

    connect(m_navigationMenu,           SIGNAL(close()),            this, SLOT(clearSearch()));
    connect(m_navigationMenu,           SIGNAL(previousResult()),   this, SLOT(navigateToPrevious()));
    connect(m_navigationMenu,           SIGNAL(nextResult()),       this, SLOT(navigateToNext()));
    connect(m_navigationMenu,           SIGNAL(backToList()),       this, SLOT(handleBackToList()));
    connect(m_navigationMenuLandscape,  SIGNAL(close()),            this, SLOT(clearSearch()));
    connect(m_navigationMenuLandscape,  SIGNAL(previousResult()),   this, SLOT(navigateToPrevious()));
    connect(m_navigationMenuLandscape,  SIGNAL(nextResult()),       this, SLOT(navigateToNext()));
    connect(m_navigationMenuLandscape,  SIGNAL(backToList()),       this, SLOT(handleBackToList()));

    updateCounters();
    paintUISearchItems();
}

ViewerSearchPopup::~ViewerSearchPopup()
{
    m_keyboard = NULL;

    clearSearch();
    // clearSearchResults without updating!
    QList<QDocView::Location*>::iterator it = m_searchResults.begin();
    QList<QDocView::Location*>::iterator itEnd = m_searchResults.end();
    for(; it != itEnd; ++it)
    {
        delete *it;
    }
    m_searchResults.clear();

    m_page = 0;
    m_totalPages = 0;
    //
}

void ViewerSearchPopup::setup()
{
    qDebug() << Q_FUNC_INFO;

    handleClearEditLineClicked();
}

void ViewerSearchPopup::start()
{
    qDebug() << Q_FUNC_INFO;

    if( m_parentViewer->isMenuShown() )
    {
        arrowCont->show();
        showKeyboardSearch();
    }
    else
        arrowCont->hide();
}

void ViewerSearchPopup::stop()
{
    qDebug() << Q_FUNC_INFO;

    if(!b_goToSearch)
        clearSearch();

    // Hide keyboard
    hideKeyboardSearch();
}

void ViewerSearchPopup::searchRequest( const QString& text )
{
    qDebug() << Q_FUNC_INFO;

    searchLineEdit->setText(text);
    m_userTyped = true;// Make it true as if the user has typed it.
    performSearch(text);
}

void ViewerSearchPopup::handleClearEditLineClicked()
{
    qDebug() << Q_FUNC_INFO;

    stopSearch();

    clearSearch();

    if(m_keyboard)
    {
        m_keyboard->clearText();
    }

    m_userTyped = false;
    searchLineEdit->setText(m_initialSearchText);
}

void ViewerSearchPopup::handleCloseSearchClicked()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->lockScreen();
    stopSearch();

    emit hideMe();
    Screen::getInstance()->releaseScreen();
}

void ViewerSearchPopup::handleSearchLineEditClicked()
{
    qDebug() << Q_FUNC_INFO;

    stopSearch();

    clearSearch();

    if(!m_keyboard)
    {
        showKeyboardSearch();
    }

    if(!m_userTyped)
    {
        searchLineEdit->clear();
    }
}

void ViewerSearchPopup::keyboardPressedChar( const QString& input )
{
    if(!m_userTyped)
    {
//        QApplication::postEvent(searchForm->searchLineEdit, new QMouseEvent( QEvent::MouseButtonPress, searchForm->searchLineEdit->pos(), Qt::LeftButton, Qt::NoButton, Qt::NoModifier));
//        QApplication::postEvent(searchForm->searchLineEdit, new QMouseEvent( QEvent::MouseButtonRelease, searchForm->searchLineEdit->pos(), Qt::LeftButton, Qt::NoButton, Qt::NoModifier));

        searchLineEdit->clear();
        searchLineEdit->setText(input);

        m_userTyped = true;
    }
}

void ViewerSearchPopup::handleKeyboardAction()
{
    qDebug() << Q_FUNC_INFO;

    if(!searchLineEdit->text().trimmed().isEmpty())
        performSearch(searchLineEdit->text());
}

void ViewerSearchPopup::showKeyboardSearch()
{
    qDebug() << Q_FUNC_INFO;

    m_keyboard = QBookApp::instance()->showKeyboard(tr("Buscar"));

    m_keyboard->handleMyQLineEdit(searchLineEdit);
    connect(m_keyboard, SIGNAL(actionRequested()), this, SLOT(handleKeyboardAction()), Qt::UniqueConnection);
    connect(m_keyboard, SIGNAL(newLinePressed()), this, SLOT(handleKeyboardAction()), Qt::UniqueConnection);
    connect(m_keyboard, SIGNAL(pressedChar(const QString &)), this, SLOT(keyboardPressedChar(const QString &)), Qt::UniqueConnection);
}

void ViewerSearchPopup::hideKeyboardSearch()
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->hideKeyboard();
    m_keyboard = NULL;
}

void ViewerSearchPopup::paintEvent( QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ViewerSearchPopup::performSearch( const QString& search )
{
    qDebug() << Q_FUNC_INFO;

    stopSearch();

    clearSearch();
    clearSearchResults();
    hideKeyboardSearch();

    // Start new search
    QDocView* docView = m_parentViewer->docView();
    docView->setBlockPaintEvents(true);
    qDebug() << Q_FUNC_INFO << "Disabled paintevents";

    processSearchCont->show();

    // Start the computation.
    QMutexLocker mutexLocker(&m_searchMutex);

    m_isSearchActive = true;
    m_finishSearch = false;
    m_alreadyPainted = false;

    // Removes spaces, \t, \n, etc.
    m_searchTerm = search.simplified();

    QFuture<void> future = QtConcurrent::run(this, &ViewerSearchPopup::performSearchConcurrent);
    m_futureWatcher.setFuture(future);
}

void ViewerSearchPopup::performSearchConcurrent()
{
    qDebug() << Q_FUNC_INFO;

    QMutexLocker mutexLocker(&m_searchMutex);

    qDebug() << Q_FUNC_INFO << "Searching";

    QThread::currentThread()->setPriority(QThread::IdlePriority);

    PowerManagerLock* lock = PowerManager::getNewLock(this);
    lock->activate();

    if(!m_searchTerm.isEmpty())
    {
        QDocView* docView = m_parentViewer->docView();
        m_currentSearch = docView->search(m_searchTerm);
    }

    if (m_currentSearch)
    {
        int i = m_searchResults.size();
        while (m_isSearchActive)
        {
            if(m_currentSearch->hasNext())
            {
                i++;
                QDocView::Location* content = m_currentSearch->next();
                trimPreview(content->preview);
                m_searchResults.append(content);
                resultsNumLbl->setText(tr("%1 resultados").arg(i));

                if( i % RESULTS_PER_PAGE == 0)
                    emit newPageReady();
            }
            else
                break;
        }
    }

    if(m_isSearchActive)
    {
        m_isSearchActive = false;
        m_finishSearch = true;

        emit searchFinished();

        delete m_currentSearch;
        m_currentSearch = NULL;
    }

    processSearchCont->hide();

    lock->release();

    qDebug() << Q_FUNC_INFO << "End search";
}

void ViewerSearchPopup::stopSearch()
{
    qDebug() << Q_FUNC_INFO;
    if(m_isSearchActive)
    {
        m_parentViewer->docView()->abortProcessing();
        m_isSearchActive = false;

        m_futureWatcher.waitForFinished();
        m_parentViewer->docView()->clearAbortProcessing();
    }
}

void ViewerSearchPopup::clearSearch()
{
    qDebug() << Q_FUNC_INFO;

    b_goToSearch = false;

    stopSearch();

    if((m_navigationMenu && m_navigationMenu->isVisible()) ||
       (m_navigationMenuLandscape && m_navigationMenuLandscape->isVisible()))
    {
        Screen::getInstance()->queueUpdates();
        m_parentViewer->hideAllElements();
        m_parentViewer->markHandler()->reloadMarks();
        Screen::getInstance()->flushUpdates();
    }

    delete m_currentSearch;
    m_currentSearch = NULL;

    processSearchCont->hide();
    clearSearchResults();
}

void ViewerSearchPopup::clearSearchResults()
{
    qDebug() << Q_FUNC_INFO;

    QList<QDocView::Location*>::iterator it = m_searchResults.begin();
    QList<QDocView::Location*>::iterator itEnd = m_searchResults.end();
    for(; it != itEnd; ++it)
    {
        delete *it;
    }
    m_searchResults.clear();

    m_page = 0;
    m_totalPages = 0;

    updateCounters();
    paintUISearchItems();
}

void ViewerSearchPopup::updateCounters()
{
    qDebug() << Q_FUNC_INFO;

    if(isVisible())
        Screen::getInstance()->queueUpdates();

    processSearchCont->hide();

    int numResults = m_searchResults.size();
    resultsNumLbl->setText(tr("%1 resultados").arg(numResults));
    setupPagination();

    if(!m_isSearchActive || (!m_alreadyPainted && m_searchResults.size() >= RESULTS_PER_PAGE) )
    {
        m_alreadyPainted = true;
        paintUISearchItems();
    }

    if (m_navigationMenu->isVisible()) m_navigationMenu->setTotalResults(numResults);
    else if (m_navigationMenuLandscape->isVisible()) m_navigationMenuLandscape->setTotalResults(numResults);

    if(isVisible())
    {
        Screen::getInstance()->setMode(Screen::MODE_QUICK, true, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
    }
}

void ViewerSearchPopup::setupPagination()
{
    qDebug() << Q_FUNC_INFO;

    int totalResults =  m_searchResults.size();

    m_totalPages = int(totalResults/RESULTS_PER_PAGE);

    // Avoid showing imcomplete page when ongoing search
    if(!m_isSearchActive && totalResults%RESULTS_PER_PAGE != 0)
        m_totalPages++;

    VerticalPagerPopup->show();
    VerticalPagerPopup->setup(m_totalPages, m_page+1, true);
}

void ViewerSearchPopup::paintUISearchItems()
{
    qDebug() << Q_FUNC_INFO;

    const int searchSize = m_searchResults.size();
    const int pageOffset = m_page * RESULTS_PER_PAGE;
    const int size = RESULTS_PER_PAGE - 1;
    for(int i = 0; i <= size; ++i)
    {
        ViewerSearchResultItem* item = m_uiSearchItems[i];
        int pos = pageOffset + i;
        if(pos < searchSize)
        {
            QDocView::Location* loc = m_searchResults.at(pos);
            item->setLocation(loc);
            item->show();
        }
        else
        {
            item->hide();
        }
    }
}

void ViewerSearchPopup::previousPage()
{
    qDebug() << Q_FUNC_INFO;

    m_page--;
    VerticalPagerPopup->setPage();
    paintUISearchItems();
}

void ViewerSearchPopup::nextPage()
{
    qDebug() << Q_FUNC_INFO;

    m_page++;
    VerticalPagerPopup->setPage();
    paintUISearchItems();
}

void ViewerSearchPopup::firstPage()
{
    qDebug() << Q_FUNC_INFO;

    m_page = 0;
    VerticalPagerPopup->setPage();
    paintUISearchItems();
}

void ViewerSearchPopup::lastPage()
{
    qDebug() << Q_FUNC_INFO;

    m_page = m_totalPages - 1;
    VerticalPagerPopup->setPage();
    paintUISearchItems();
}

void ViewerSearchPopup::handleResultPressed( const QString& url )
{
    qDebug() << Q_FUNC_INFO << "Url: " << url;

    Screen::getInstance()->lockScreen();
    b_goToSearch = true;

    m_parentViewer->hideAllElements();

    m_navigationMenu->setTotalResults(m_searchResults.size());
    m_navigationMenuLandscape->setTotalResults(m_searchResults.size());

    // Get pressed result index
    const int searchSize = m_searchResults.size();
    const int pageOffset = m_page * RESULTS_PER_PAGE;
    i_currentShownResultIndex = pageOffset;
    for(int i = pageOffset;
        i < (searchSize < RESULTS_PER_PAGE + pageOffset ? searchSize - pageOffset: RESULTS_PER_PAGE + pageOffset); i++){
        if(url == m_searchResults[i]->ref){
            i_currentShownResultIndex = i;
            break;
        }

    }

    m_navigationMenu->setCurrentResultIndex(i_currentShownResultIndex + 1);
    m_navigationMenuLandscape->setCurrentResultIndex(i_currentShownResultIndex + 1);

    m_parentViewer->gotoSearchWordPage(url);

    if (m_parentViewer->docView()->isHorizontal())
        m_parentViewer->showElement(m_navigationMenuLandscape);
    else
        m_parentViewer->showElement(m_navigationMenu);

    Screen::getInstance()->releaseScreen();
}

void ViewerSearchPopup::trimPreview( QString& totalStr )
{
    qDebug() << Q_FUNC_INFO << totalStr;

    static int preview_max_chars = SEARCH_PREVIEW_MAX_CHARS - 20;// Length of "<b><i><u>%1</u></i></b>" and "..."*2

    int searchTermSize = m_searchTerm.size();
    qDebug() << Q_FUNC_INFO << searchTermSize;

    if(searchTermSize < preview_max_chars)
    {
        int charsLeft = preview_max_chars - searchTermSize;
        QString strToCompare = bqUtils::simplify(totalStr);
        int pos = strToCompare.indexOf(bqUtils::simplify(m_searchTerm));
        QString searchTerm = totalStr.mid(pos, searchTermSize);
        totalStr = totalStr.mid(pos-(charsLeft>>1), preview_max_chars);
        totalStr = totalStr.replace(searchTerm, QString("<b><i><u>%1</u></i></b>").arg(searchTerm), Qt::CaseInsensitive);
    }
    else
    {
        totalStr = m_searchTerm.mid(0, preview_max_chars);
    }

    totalStr.prepend("...");
    totalStr.append("...");
    qDebug() << totalStr;
}

void ViewerSearchPopup::navigateToPrevious()
{
    qDebug() << "--->" << Q_FUNC_INFO << i_currentShownResultIndex;

    if(i_currentShownResultIndex <= 0)
        return;

    i_currentShownResultIndex--;
    navigateToCurrent();
}

void ViewerSearchPopup::navigateToNext()
{
    qDebug() << "--->" << Q_FUNC_INFO << i_currentShownResultIndex;

    if(i_currentShownResultIndex >= m_searchResults.size() - 1 )
        return;

    i_currentShownResultIndex++;
    navigateToCurrent();
}

void  ViewerSearchPopup::navigateToCurrent()
{
    qDebug() << "--->" << Q_FUNC_INFO;

    // Prevent jumps after canceling search
    if(!m_navigationMenu->isVisible() && !m_navigationMenuLandscape->isVisible()) return;

    m_navigationMenu->setCurrentResultIndex(i_currentShownResultIndex + 1);
    m_navigationMenuLandscape->setCurrentResultIndex(i_currentShownResultIndex + 1);

    if(!m_timer_gotoSearch->isActive()) {
        b_pendingGoto = true;
        m_timer_gotoSearch->start();
    }
}

void ViewerSearchPopup::navigateToPending()
{
    qDebug() << "--->" << Q_FUNC_INFO;
    m_timer_gotoSearch->stop();
    if(b_pendingGoto && i_currentShownResultIndex <= m_searchResults.size() - 1)
    {
        b_pendingGoto = false;
        Screen::getInstance()->queueUpdates();
        m_parentViewer->gotoSearchWordPage(m_searchResults[i_currentShownResultIndex]->ref);
        Screen::getInstance()->flushUpdates();
    }
}

void ViewerSearchPopup::handleBackToList()
{
    qDebug() << Q_FUNC_INFO;
    m_page = i_currentShownResultIndex/RESULTS_PER_PAGE;
    Screen::getInstance()->queueUpdates();
    m_parentViewer->hideAllElements();
    m_parentViewer->markHandler()->reloadMarks();
    paintUISearchItems();
    setupPagination();
    m_parentViewer->showElement(this);
    Screen::getInstance()->flushUpdates();
}
