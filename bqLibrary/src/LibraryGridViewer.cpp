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

#include "LibraryGridViewer.h"

#include "Library.h"
#include "LibraryGridViewerItem.h"
#include "LibraryActionsMenu.h"
#include "LibraryPageHandler.h"
#include "BookInfo.h"
#include "QBookApp.h"
#include "Model.h"
#include "Screen.h"

LibraryGridViewer::LibraryGridViewer(QWidget* parent) : GestureWidget(parent)
  , m_library(NULL)
{}

void LibraryGridViewer::init()
{
    int itemsPerPage = getItemsPerPage();
    LibraryGridViewerItem* books[itemsPerPage];
    getBookItems(books);

    m_actionsMenu = new LibraryActionsMenu(this);
    m_actionsMenu->hide();

    connect(m_actionsMenu,          SIGNAL(copyFile(const QString)),            this, SIGNAL(copyFile(const QString)));
    connect(m_actionsMenu,          SIGNAL(deleteFile(const QString)),          this, SIGNAL(removeFile(const QString)));

    for(int i = getItemsPerPageOffset(); i < itemsPerPage + getItemsPerPageOffset(); ++i)
    {
        books[i]->hide();
        connect(books[i], SIGNAL(itemClicked(const QString&)),      this, SIGNAL(itemClicked(const QString&)));
        connect(books[i], SIGNAL(itemLongPress(const QString&)),    this, SIGNAL(itemLongPress(const QString&)));
        connect(books[i], SIGNAL(fileLongPressed(const QString&)),  this, SLOT(fileLongPress(const QString&)));
    }

    connect(this, SIGNAL(tap()), this, SLOT(hideElements()));
    connect(this, SIGNAL(longPressStart()), this, SLOT(hideElements()));
    LibraryPageHandler* pageHandler = getPageHandler();
    connect(pageHandler,   SIGNAL(previousPageReq()),       this, SLOT(previousPage()));
    connect(pageHandler,   SIGNAL(nextPageReq()),           this, SLOT(nextPage()));
    connect(pageHandler,   SIGNAL(firstPage()),             this, SLOT(firstPage()));
    connect(pageHandler,   SIGNAL(lastPage()),              this, SLOT(lastPage()));
    connect(pageHandler,   SIGNAL(goToLibraryToPage(int)),  this, SLOT(goLibraryPage(int)));
    pageHandler->hide();
}

void LibraryGridViewer::done()
{
    delete m_actionsMenu;
    m_actionsMenu = NULL;
}

void LibraryGridViewer::setLibrary( Library* library )
{
    m_library = library;
}

void LibraryGridViewer::start(int startPage)
{
    qDebug() << Q_FUNC_INFO;

    // Setup the pager
    int search_size = m_library->m_searchBooks.size() + m_library->m_files.size() + m_library->m_dirs.size();
    if(m_library->getFilterMode() == Library::ELFM_COLLECTIONS)
        setupPager(m_collectionsList.size(), startPage);
    else if(m_library->getFilterMode() < Library::ELFM_LIBRARY_MODE || m_library->getFilterMode() == Library::ELFM_COLLECTION)
    {
        setupPager(m_library->m_books.size());
    }
    else if(m_library->getFilterMode() == Library::ELFM_SEARCH)
    {
        setupPager(search_size);
    }
    else
    {
        setupPager(m_library->m_files.size());
    }

    paint();
    resume();
    show();
}

void LibraryGridViewer::resume()
{
    qDebug() << Q_FUNC_INFO;
}

void LibraryGridViewer::paint()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();

    int mode;
    if(m_library->getFilterMode() == Library::ELFM_SEARCH && !m_library->isSearchView())
        mode = m_library->getLastMode();
    else
        mode = m_library->getFilterMode();
    if(mode == Library::ELFM_COLLECTIONS)
        paintCollections();
    else if(mode < Library::ELFM_LIBRARY_MODE || mode == Library::ELFM_COLLECTION)
    {
        paintBooks();
    }
    else
    {
        paintFiles();
    }

    getPageHandler()->setPage(m_library->m_page+1);
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void LibraryGridViewer::pause()
{
    qDebug() << Q_FUNC_INFO;
}

void LibraryGridViewer::stop()
{
    qDebug() << Q_FUNC_INFO;

    pause();
    hide();
}

void LibraryGridViewer::fileLongPress(const QString &path)
{
    qDebug() << Q_FUNC_INFO << path;
    QFileInfo fi(path);
    if(m_library->getFilterMode() == Library::ELFM_COLLECTIONS)
        return;

    int i;
    int itemsPerPage = getItemsPerPage();
    m_actionsMenu->setFile(path);
    LibraryGridViewerItem* books[itemsPerPage];
    getBookItems(books);
    for(i = getItemsPerPageOffset(); i < itemsPerPage + getItemsPerPageOffset(); ++i)
    {
        if(books[i]->getPath() == path) {
            Screen::getInstance()->queueUpdates();
            int x, y;
            if(m_library->getViewMode() == Library::ELVM_LINE){
                x = books[i]->x() + ((books[i]->width() - m_actionsMenu->width())>>1);
                y = books[i]->y() + ((m_actionsMenu->height() - books[i]->height())>>1);
            }
            else{
                x = books[i]->parentWidget()->mapToParent(QPoint(0,0)).x();
                if((i%3)!=0)
                    x += ((books[i]->width() - m_actionsMenu->width())>>1);
                y = books[i]->parentWidget()->mapToParent(QPoint(0,0)).y() + books[i]->parentWidget()->height() - m_actionsMenu->height();
            }
            m_actionsMenu->move(x, y);
            m_actionsMenu->show();
            Screen::getInstance()->flushUpdates();
            break;
        }
    }
}

void LibraryGridViewer::paintBooks()
{
    qDebug() << Q_FUNC_INFO;

    if (m_library->m_page < 0)
        m_library->m_page = 0;

    int itemsPerPage = getItemsPerPage();
    LibraryGridViewerItem* books[itemsPerPage];
    getBookItems(books);

    int librarySize;
    if(m_library->isSearchView())
        librarySize = m_library->m_searchBooks.size();
    else
        librarySize = m_library->m_books.size();
    const int pageOffset = m_library->m_page * itemsPerPage;
    const int size = itemsPerPage - 1 + getItemsPerPageOffset();
    for(int i = getItemsPerPageOffset(); i <= size; ++i)
    {
        LibraryGridViewerItem* item = books[i];
        int pos = pageOffset + i;
        if(pos < librarySize)
        {
            const BookInfo* book;
            if(m_library->isSearchView())
                book = m_library->m_searchBooks.at(pos);
            else
                book = m_library->m_books.at(pos);
            item->setBook(book, m_library->getFilterMode());
            if(m_library->m_selectedBooks.contains(book->path))
                item->select();
            else
                item->unselect();
            item->show();
        }
        else
        {
            item->hide();
        }
    }
}

void LibraryGridViewer::paintFiles()
{
    qDebug() << Q_FUNC_INFO;

    int itemsPerPage = getItemsPerPage();
    LibraryGridViewerItem* books[itemsPerPage];
    getBookItems(books);

    int librarySize = m_library->m_searchBooks.size() + m_library->m_files.size() + m_library->m_dirs.size();;
    const int pageOffset = m_library->m_page * itemsPerPage;
    const int size = itemsPerPage - 1;

    // Special case, to paint the back arrow:
    int itemIdx = 0;
    int fileIdx = 0;

    if(!m_library->isCurrentPathEmpty())
    {
        if(m_library->m_page == 0)
        {
            LibraryGridViewerItem* item = books[itemIdx];
            item->setArrow(m_library->getCurrentPathParent());
            item->show();
            ++itemIdx;
        }
        else
            fileIdx = -1;// For a proper behaviour of the pos variable inside the loop
    }

    int dir_size = m_library->m_dirs.size();
    int searchBooks_size = m_library->m_searchBooks.size();
    for(; itemIdx <= size; ++itemIdx, ++fileIdx)
    {
        LibraryGridViewerItem* item = books[itemIdx];
        int pos = pageOffset + fileIdx;
        if(pos < librarySize)
        {
            if(pos < dir_size)
                item->setFile(m_library->m_dirs.at(pos));
            else if(pos < searchBooks_size + dir_size) {
                int bookPos = pos - dir_size;
                item->setBook(m_library->m_searchBooks.at(bookPos), m_library->getFilterMode());
            }
            else {
                int filePos = pos - searchBooks_size - dir_size;
                const QFileInfo* fi = m_library->m_files.at(filePos);
                Model* model = QBookApp::instance()->getModel();
                const BookInfo* bookInfo = model->getBookInfo(fi->absoluteFilePath());
                if(bookInfo)
                    item->setBook(bookInfo, m_library->getFilterMode());
                else
                    item->setFile(fi);
            }
            item->show();
        }
        else
        {
            item->hide();
        }
    }
}

void LibraryGridViewer::paintCollections()
{
    qDebug() << "--->" << Q_FUNC_INFO;

    int itemsPerPage = getItemsPerPage();
    LibraryGridViewerItem* books[itemsPerPage];
    getBookItems(books);

    const int collectionsSize = m_collectionsList.size();
    const int pageOffset = m_library->m_page * itemsPerPage;
    const int size = itemsPerPage - 1 + getItemsPerPageOffset();

    for(int i=0; i <= size; ++i){
        LibraryGridViewerItem* item = books[i];
        int pos = pageOffset + i;
        if(pos < collectionsSize)
        {
            int books = QBookApp::instance()->getModel()->getBooksInCollectionCount(m_collectionsList.at(pos));
            item->setCollection(m_collectionsList.at(pos),books);
        }else
            item->setCollection("",0);
        item->show();
    }
}

bool LibraryGridViewer::clearActionsMenu()
{
    if(m_actionsMenu->isVisible()) {
        m_actionsMenu->hide();
        return true;
    }
    else {
        m_actionsMenu->hide();
        return false;
    }
}

void LibraryGridViewer::firstPage()
{
    qDebug() << Q_FUNC_INFO;
    clearActionsMenu();
    if(m_library->anyPopUpShown()) {
        m_library->hideAllElements();
        LibraryPageHandler* pageHandler = getPageHandler();
        pageHandler->setPage(m_library->m_page+1);
        return;
    }

    pause();

    m_library->m_page = 0;
    paint();

    resume();
}

void LibraryGridViewer::lastPage()
{
    qDebug() << Q_FUNC_INFO;
    clearActionsMenu();
    if(m_library->anyPopUpShown()) {
        m_library->hideAllElements();
        LibraryPageHandler* pageHandler = getPageHandler();
        pageHandler->setPage(m_library->m_page+1);
        return;
    }

    pause();

    m_library->m_page = m_library->m_totalPages - 1;
    paint();

    resume();
}

void LibraryGridViewer::nextPage()
{
    if(!isVisible()) return;
    qDebug() << Q_FUNC_INFO;
    clearActionsMenu();
    if(m_library->anyPopUpShown()) {
        m_library->hideAllElements();
        LibraryPageHandler* pageHandler = getPageHandler();
        pageHandler->setPage(m_library->m_page+1);
        return;
    }

    pause();

    m_library->m_page++;
    paint();

    resume();
}

void LibraryGridViewer::previousPage()
{
    if(!isVisible()) return;
    qDebug() << Q_FUNC_INFO;
    clearActionsMenu();
    if(m_library->anyPopUpShown()) {
        m_library->hideAllElements();
        getPageHandler()->setPage(m_library->m_page-1);
        return;
    }

    pause();

    m_library->m_page--;
    paint();

    resume();
}

void LibraryGridViewer::goLibraryPage(int page)
{
    qDebug() << Q_FUNC_INFO << "page: " << page;
    clearActionsMenu();

    if(m_library->anyPopUpShown())
    {
        m_library->hideAllElements();
        LibraryPageHandler* pageHandler = getPageHandler();
        pageHandler->setPage(page);
        return;
    }

    pause();

    m_library->m_page = page;
    paint();

    resume();
}

void LibraryGridViewer::setupPager( int librarySize, int startPage )
{
    qDebug() << Q_FUNC_INFO << "startPage: " << startPage;
    LibraryPageHandler* pageHandler = getPageHandler();
    m_library->m_page = startPage;
    if(!librarySize)
    {
        pageHandler->setup(1, 1, true);
        pageHandler->hidePages();
    }
    else
    {
        if(!m_library->isCurrentPathEmpty())
            m_library->m_totalPages = ((librarySize-getItemsPerPageOffset()) / getItemsPerPage()) + 1;
        else
            m_library->m_totalPages = ((librarySize-1-getItemsPerPageOffset()) / getItemsPerPage()) + 1;
        pageHandler->setup(m_library->m_totalPages, m_library->m_page+1, true);
        pageHandler->show();
    }
}

QString LibraryGridViewer::getDateStyled( const QDateTime& date, bool isIcon)
{
    qDebug() << Q_FUNC_INFO;

    const QString lang = QBook::settings().value("setting/language", QVariant("es")).toString();
    QString hourSufix = " h";
    if (lang == "en")
    {
        hourSufix = "";
    }

    if(date.isNull()) return "";

    if(date.toString("MM/yy").operator ==(QDateTime::currentDateTime().toString("MM/yy"))){
        int today = QDateTime::currentDateTime().toString("d").toInt();
        int date_book = date.toString("d").toInt();
        if((today - date_book) == 1)
        {
            if(isIcon)
            {
                return tr("Ayer") + date.toString(", hh:mm") + hourSufix;
            }
            else
            {
                return tr("Ayer a las") + date.toString(" hh:mm") + hourSufix;
            }
        }
        if(today == date_book)
        {
            if(isIcon)
            {
                return tr("Hoy") + date.toString(", hh:mm") + hourSufix;
            }
            else
            {
                return tr("Hoy a las") + date.toString(" hh:mm") + hourSufix;
            }
        }
    }

    if(isIcon)
    {
        return date.toString(tr("dd/MM, hh:mm")) + hourSufix;
    }
    else
    {
        return date.toString(tr("dd/MM/yy, ")) + tr("a las") + date.toString(" hh:mm") + hourSufix;
    }
}

void LibraryGridViewer::hideElements()
{
    clearActionsMenu();
    m_library->hideAllElements();
}

void LibraryGridViewer::setCollections(QStringList collections)
{
    qDebug() << Q_FUNC_INFO;
    m_collectionsList = collections;
}
