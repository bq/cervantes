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

#include "Search.h"
#include "SearchItem.h"

#include "Keyboard.h"
#include "Model.h"
#include "Library.h"
#include "InfoDialog.h"

#include "QBookApp.h"
#include "BookInfo.h"
#include "Storage.h"
#include "bqUtils.h"

#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include <QFileInfo>

#ifndef HACKERS_EDITION
#include "bqDeviceServices.h"
#endif

#define LAST_ITEM 8
#define BTN_ITEM 2

Search::Search (QWidget* parent) : QBookForm(parent)
  , m_powerLock(NULL)
  , keyboard(NULL)
  , remoteSearchResult(NULL)
  , m_userTyped(false)
  , m_abortSearch(false)
{
        setupUi(this);

        QFile fileSpecific(":/res/search_styles.qss");
        QFile fileCommons(":/res/search_styles_generic.qss");
        fileSpecific.open(QFile::ReadOnly);
        fileCommons.open(QFile::ReadOnly);

        QString styles = QLatin1String(fileSpecific.readAll() + fileCommons.readAll());
        setStyleSheet(styles);

        page = 0;
        i_searchSize = 0;

        connect(clearSearchBtn, SIGNAL(clicked()),this,SLOT(clearSearchLine()));
        connect(searchLineEdit, SIGNAL(clicked()),this, SLOT(handleClicked()));

        connect(verticalPagerHandler,SIGNAL(previousPageReq()),this,SLOT(back()));
        connect(verticalPagerHandler,SIGNAL(nextPageReq()),this,SLOT(forward()));
        connect(verticalPagerHandler,SIGNAL(firstPage()),this,SLOT(first()));
        connect(verticalPagerHandler,SIGNAL(lastPage()),this,SLOT(last()));

        shopOnlineBtn->hide();
        deviceBooksBtn->hide();
        connect(shopOnlineBtn,SIGNAL(clicked()),this,SLOT(openServiceSearch()));

        books.append(SearchResultItem1);
        books.append(SearchResultItem2);
        books.append(SearchResultItem3);
        books.append(SearchResultItem4);
        books.append(SearchResultItem5);
        books.append(SearchResultItem6);
        books.append(SearchResultItem7);
        books.append(SearchResultItem8);
        books.append(SearchResultItem9);


        shopBooks.append(SearchResultItemShop1);
        shopBooks.append(SearchResultItemShop2);
        shopBooks.append(SearchResultItemShop3);
        shopBooks.append(SearchResultItemShop4);
        shopBooks.append(SearchResultItemShop5);
        shopBooks.append(SearchResultItemShop6);
        shopBooks.append(SearchResultItemShop7);
        shopBooks.append(SearchResultItemShop8);
        shopBooks.append(SearchResultItemShop9);


        connect(SearchResultItem1, SIGNAL(openPath(const QString&)), this, SLOT(openPath(const QString&)));
        connect(SearchResultItem2, SIGNAL(openPath(const QString&)), this, SLOT(openPath(const QString&)));
        connect(SearchResultItem3, SIGNAL(openPath(const QString&)), this, SLOT(openPath(const QString&)));
        connect(SearchResultItem4, SIGNAL(openPath(const QString&)), this, SLOT(openPath(const QString&)));
        connect(SearchResultItem5, SIGNAL(openPath(const QString&)), this, SLOT(openPath(const QString&)));
        connect(SearchResultItem6, SIGNAL(openPath(const QString&)), this, SLOT(openPath(const QString&)));
        connect(SearchResultItem7, SIGNAL(openPath(const QString&)), this, SLOT(openPath(const QString&)));
        connect(SearchResultItem8, SIGNAL(openPath(const QString&)), this, SLOT(openPath(const QString&)));
        connect(SearchResultItem9, SIGNAL(openPath(const QString&)), this, SLOT(openPath(const QString&)));

#ifndef HACKERS_EDITION
        connect(SearchResultItemShop1, SIGNAL(openStore(const BookInfo*)), this, SLOT(openStore(const BookInfo*)));
        connect(SearchResultItemShop2, SIGNAL(openStore(const BookInfo*)), this, SLOT(openStore(const BookInfo*)));
        connect(SearchResultItemShop3, SIGNAL(openStore(const BookInfo*)), this, SLOT(openStore(const BookInfo*)));
        connect(SearchResultItemShop4, SIGNAL(openStore(const BookInfo*)), this, SLOT(openStore(const BookInfo*)));
        connect(SearchResultItemShop5, SIGNAL(openStore(const BookInfo*)), this, SLOT(openStore(const BookInfo*)));
        connect(SearchResultItemShop6, SIGNAL(openStore(const BookInfo*)), this, SLOT(openStore(const BookInfo*)));
        connect(SearchResultItemShop7, SIGNAL(openStore(const BookInfo*)), this, SLOT(openStore(const BookInfo*)));
        connect(SearchResultItemShop8, SIGNAL(openStore(const BookInfo*)), this, SLOT(openStore(const BookInfo*)));
        connect(SearchResultItemShop9, SIGNAL(openStore(const BookInfo*)), this, SLOT(openStore(const BookInfo*)));
#endif


        connect(&m_futureWatcher, SIGNAL(finished()), this, SLOT(handleSearchFinished()));

        model = QBookApp::instance()->getModel();

        m_powerLock = PowerManager::getNewLock(this);
        init();
}

Search::~Search()
{
        qDebug() << Q_FUNC_INFO;

        deleteData();

        delete remoteSearchResult;
        remoteSearchResult = NULL;
        future.waitForFinished();
}

void Search::activateForm()
{
        qDebug() << Q_FUNC_INFO;

        QBookApp::instance()->getStatusBar()->setMenuTitle(tr("Search"));

        if(i_searchSize == 0)
            handleKeyboard();

        else
        {
            removeResults();
            show();
        }

        connect(model, SIGNAL(modelChanged(QString,int)), this, SLOT(removeResults()));
        connect(Storage::getInstance(), SIGNAL(partitionUmounted(StoragePartition*)), this, SLOT(removeResults()));
}

void Search::deactivateForm()
{
        qDebug() << Q_FUNC_INFO;

        m_abortSearch = true;
        m_userTyped = false;

        future.waitForFinished();

        disconnect(model, SIGNAL(modelChanged(QString,int)), this, SLOT(removeResults()));
        disconnect(Storage::getInstance(), SIGNAL(partitionUmounted(StoragePartition*)), this, SLOT(removeResults()));
}

void Search::init()
{
        qDebug() << Q_FUNC_INFO;

        SearchItem* item;
        foreach(item , books)
            item->hide();

        SearchItemShop* shopItem;
        foreach(shopItem , shopBooks)
            shopItem->hide();

        resultNumberCont->hide();
        shopOnlineBtn->hide();
        verticalPagerHandler->hide();
}

void Search::keyReleaseEvent(QKeyEvent *event)
{
        qDebug() << Q_FUNC_INFO;

        switch (event->key()) {
        case QBook::QKEY_BACK:
            event->accept();

            Screen::getInstance()->queueUpdates();
            if(keyboard && keyboard->isVisible()){
                QBookApp::instance()->hideKeyboard();
                future.waitForFinished();
                Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_WAITFORCOMPLETION,Q_FUNC_INFO);
                Screen::getInstance()->flushUpdates();
                break;
            }

            Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
            Screen::getInstance()->flushUpdates();
            QBookApp::instance()->goToHome();
            break;
        default:
             QBookForm::keyReleaseEvent(event);
             break;
        }
}

void Search::clearSearchLine ()
{
        qDebug() << Q_FUNC_INFO;

        searchLineEdit->clear();

        m_userTyped = true;

}

void Search::deleteData()
{
        qDebug() << Q_FUNC_INFO;

        i_searchSize = 0;

        m_books.clear();

        QList<const QFileInfo*>::iterator it = m_files.begin();
        QList<const QFileInfo*>::iterator itEnd = m_files.end();
        for(; it != itEnd; ++it)
        {
            delete *it;
        }
        m_files.clear();
        it = m_dirs.begin();
        itEnd = m_dirs.end();
        for(; it != itEnd; ++it)
        {
            delete *it;
        }
        m_dirs.clear();
        m_images.clear();
}

void Search::handleClose ()
{
        qDebug() << Q_FUNC_INFO;

        keyboard = QBookApp::instance()->hideKeyboard();
        future.waitForFinished();
        QBookApp::instance()->goToLibrary();
}

void Search::handleSearch ()
{
        qDebug() << Q_FUNC_INFO;
        if(!m_userTyped || searchLineEdit->text().trimmed().isEmpty())
            return;

        QBookApp::instance()->getStatusBar()->setSpinner(true);
        Screen::getInstance()->queueUpdates();
        QBookApp::instance()->hideKeyboard();
        deleteData();
        delete remoteSearchResult;
        remoteSearchResult = NULL;
        page = 0;
        m_powerLock->activate();

        // Start the computation.
        QMutexLocker mutexLocker(&m_searchMutex);

        m_abortSearch = false;

        future = QtConcurrent::run(this, &Search::searchResults);
        m_futureWatcher.setFuture(future);

        // Display the dialog and start the event loop.
        qDebug() << Q_FUNC_INFO << "End";

        Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_PARTIALSCREEN_UPDATE,Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();

}

void Search::searchResults()
{
        qDebug() << Q_FUNC_INFO;

        QMutexLocker mutexLocker(&m_searchMutex);

        qs_lastSearch = bqUtils::simplify(searchLineEdit->text());
        qDebug() << "Search String:" << qs_lastSearch;
        searchInLibrary(qs_lastSearch);
        qDebug() << "Search result size :" << m_books.size() + m_files.size() + m_dirs.size();
#ifndef HACKERS_EDITION
        if(!m_abortSearch
                && ConnectionManager::getInstance()->isConnected()
                && QBookApp::instance()->isLinked())
        {
            // Get remote results
            delete remoteSearchResult;
            remoteSearchResult = NULL;
            remoteSearchResult = searchInService(qs_lastSearch);
        }
#endif

        qDebug() << Q_FUNC_INFO << "End";
}

void Search::searchInLibrary( const QString& searchStr)
{
        qDebug() << Q_FUNC_INFO << "Path: " << searchStr;

        searchInPath(Storage::getInstance()->getPrivatePartition()->getMountPoint(), searchStr);
        searchInPath(Storage::getInstance()->getPublicPartition()->getMountPoint(), searchStr);
        if(Storage::getInstance()->getRemovablePartition())
            searchInPath(Storage::getInstance()->getRemovablePartition()->getMountPoint(), searchStr);

}

bool ImageLessThan(const QString &s1, const QString &s2)
{
    QFileInfo f1(s1);
    QFileInfo f2(s2);
    return f1.completeBaseName().toLower() < f2.completeBaseName().toLower();
}

void Search::searchInPath(const QString& path, const QString& searchStr)
{
        qDebug() << Q_FUNC_INFO << "Path: " << path << "Search String" << searchStr;

        QDir dir(path);
        if(path!=Storage::getInstance()->getPrivatePartition()->getMountPoint()) {
            dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files | QDir::Readable);
            dir.setSorting(QDir::DirsFirst | QDir::Name);
        }
        else {
            dir.setFilter(QDir::Files | QDir::Readable);
            dir.setSorting(QDir::Name);
        }

        QStringList list = dir.entryList();

        Model* model = QBookApp::instance()->getModel();

        int size = list.size();
        QList<const BookInfo*>  books_title;
        QList<const BookInfo*>  books_author;
        QFileInfo fi;
        for (int i = 0; i < size; ++i)
        {
            QString filePath(dir.filePath(list.at(i)));
            fi.setFile(filePath);
            if(fi.isDir())
            {
                searchInPath(filePath, searchStr);
            }

            const BookInfo* bookInfo = model->getBookInfo(filePath);
            if(bqUtils::simplify(filePath).contains(searchStr, Qt::CaseInsensitive) && fi.isDir()) {
                qDebug() << Q_FUNC_INFO << "Adding filePath: " << filePath;
                QFileInfo* dir = new QFileInfo(fi);
                m_dirs.append(dir);
            }
            else if(bookInfo) {
                if(bqUtils::simplify(bookInfo->title).contains(searchStr, Qt::CaseInsensitive))
                {
                    qDebug() << Q_FUNC_INFO << "Adding filePath: " << filePath;
                    books_title.append(bookInfo);
                }
                else if(bqUtils::simplify(bookInfo->author).contains(searchStr, Qt::CaseInsensitive))
                {
                    qDebug() << Q_FUNC_INFO << "Adding filePath: " << filePath;
                    books_author.append(bookInfo);
                }
            }
            else if(bqUtils::simplify(filePath).contains(searchStr, Qt::CaseInsensitive) &&
                    !filePath.contains(Storage::getInstance()->getPrivatePartition()->getMountPoint()))
            {
                qDebug() << Q_FUNC_INFO << "Adding filePath: " << filePath;
                QFileInfo* file = new QFileInfo(fi);
                m_files.append(file);
                if(QBookApp::instance()->isImage(filePath))
                {
                    m_images.append(filePath);
                }
            }
        }
        qSort(m_dirs.begin(), m_dirs.end(), fileNameLessThan);
        qSort(books_title.begin(), books_title.end(), titleLessThan);
        qSort(books_author.begin(), books_author.end(), authorLessThan);
        m_books.append(books_title);
        m_books.append(books_author);
        qSort(m_files.begin(), m_files.end(), fileNameLessThan);
        qSort(m_images.begin(), m_images.end(), ImageLessThan);
}


#ifndef HACKERS_EDITION
bqDeviceServicesClientBooksList* Search::searchInService( const QString& searchStr)
{
        bqDeviceServices* services = QBookApp::instance()->getDeviceServices();
        // NOTE (epaz) itemsPerPage>>1 equals to itemsPerPage/2
        return services->getServiceBooksList(bqDeviceServicesClient::CATEGORY_ALL, books.size()>>1, searchStr.toLower());
}
#endif

void Search::show ()
{
        qDebug() << Q_FUNC_INFO;

        bool shopResults = false;
        resultsNumberLbl->hide();
        resultsLbl->hide();
        verticalPagerHandler->show();
        verticalPagerHandler->setFocus();

        int pos;
        int pageItems = 0;

        int firstPos;
        int lastPos;
        int size;
        if(page == 0) {
            size = books.size() - 1;
            firstPos = pageItems + 1;
            lastPos = firstPos + books.size() - 2;
            deviceBooksBtn->show();
            books.at(LAST_ITEM)->hide();
        }
        else  {
            pageItems = (page*books.size()) - 1;
            size = books.size();
            firstPos = pageItems + 1;
            lastPos = firstPos + books.size() - 1;
            deviceBooksBtn->hide();
        }

        i_searchSize = m_files.size() + m_books.size() + m_dirs.size();
        if(lastPos > i_searchSize)
            lastPos = i_searchSize;
        if(i_searchSize == 0) {
            showResultLbl->setText(tr("Mostrando (0) resultados"));
            showResultLbl->show();
            resultNumberCont->show();
        }
        else if(firstPos > i_searchSize && i_searchSize > 0) {
            showResultLbl->hide();
            resultNumberCont->hide();
        }
        else {
            showResultLbl->setText(tr("Mostrando %1 - %2 de %3 resultados").arg(firstPos).arg(lastPos).arg(i_searchSize));
            showResultLbl->show();
            resultNumberCont->show();
        }

        int book_size = m_books.size();
        int dir_size = m_dirs.size();

        for(int i=0; i<size; ++i){
            pos = pageItems + i;
            if(pos < i_searchSize)
            {
                SearchItem* item = books.at(i);
                if(pos < dir_size)
                    item->setFile(m_dirs.at(pos));
                else if(pos < book_size + dir_size) {
                    int bookPos = pos - dir_size;
                    item->setBook(m_books.at(bookPos));
                }
                else {
                    int filePos = pos - book_size - dir_size;
                    item->setFile(m_files.at(filePos));
                }
                item->paint();
                item->show();
                shopBooks.at(i)->setBook(0);
                shopBooks.at(i)->hide();
                shopResults = false;

            }else{
                books.at(i)->setBook(0);
                books.at(i)->hide();
                if(remoteSearchResult)
                    shopResults = true;
            }
        }
#ifndef HACKERS_EDITION
        if(remoteSearchResult && remoteSearchResult->booksList.size() != 0)
            shopOnlineBtn->setVisible(shopResults);
        else
#endif
            shopOnlineBtn->setVisible(false);


        int relativePage = page - (i_searchSize / books.size());

        //We use BTN_ITEM to add the buttons shopOnlineBtn, deviceBooksBtn like items.
        qDebug() << Q_FUNC_INFO << (i_searchSize + BTN_ITEM) % books.size();
        for(int i=0; i < (relativePage == 0 ? shopBooks.size() - ((i_searchSize + BTN_ITEM) % books.size()) : shopBooks.size()); ++i)
        {
            int relativePos;
            if(relativePage == 0) {
                relativePos = i;
                if(((i_searchSize + BTN_ITEM) % books.size()) == 0)
                    shopResults = false;
            }
            else {
                if(((i_searchSize + BTN_ITEM) % books.size()) == 0)
                    relativePos = i;
                else
                    relativePos = (relativePage*shopBooks.size()) + i - ((i_searchSize + BTN_ITEM) % books.size());
            }

#ifndef HACKERS_EDITION
            if(relativePos < (remoteSearchResult ? remoteSearchResult->booksList.size() : 0) && shopResults == true){
                SearchItemShop* item = shopBooks.at(i);
                item->setBook(remoteSearchResult->booksList.at(relativePos));
                item->paint();
                item->show();
            } else{
#endif
                shopBooks.at(i)->setBook(0);
                shopBooks.at(i)->hide();
#ifndef HACKERS_EDITION
            }
#endif
        }
}

void Search::handleKeyboard ()
{
        qDebug() << Q_FUNC_INFO;

        keyboard = QBookApp::instance()->showKeyboard(tr("Search"));
        keyboard->handleMyQLineEdit(searchLineEdit);
        connect(keyboard, SIGNAL(actionRequested()), this, SLOT(handleSearch()), Qt::UniqueConnection);
        connect(keyboard, SIGNAL(newLinePressed()),  this, SLOT(handleSearch()), Qt::UniqueConnection);
        connect(keyboard, SIGNAL(pressedChar(const QString &)), this, SLOT(keyboardPressedChar(const QString &)), Qt::UniqueConnection);

}

void Search::forward ()
{
        qDebug() << Q_FUNC_INFO;

        ++page;
        Screen::getInstance()->queueUpdates();
        show();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();

}

void Search::back()
{
        qDebug() << Q_FUNC_INFO;

        --page;
        Screen::getInstance()->queueUpdates();
        show();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();

}

void Search::first ()
{
        qDebug() << Q_FUNC_INFO;

        page = 0;
        Screen::getInstance()->queueUpdates();
        show();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();

}

void Search::last()
{
        qDebug() << Q_FUNC_INFO;

        page = getTotalPages()-1;
        Screen::getInstance()->queueUpdates();
        show();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
}

int  Search::getTotalPages ()
{
        qDebug() << Q_FUNC_INFO;
        i_searchSize = m_files.size() + m_books.size() + m_dirs.size();

        if(!i_searchSize && ! remoteSearchResult) {
            shopOnlineBtn->hide();
            return 1;
        }

#ifndef HACKERS_EDITION
        if(i_searchSize == 0 && remoteSearchResult->booksList.size() == 0) {
            shopOnlineBtn->hide();
            return 1;
        }
#endif

        if (!remoteSearchResult) {
            if ((i_searchSize + 1 ) % books.size() == 0)
                return (i_searchSize + 1) / books.size();
            else
                return (i_searchSize + 1) / books.size() + 1;
        }
#ifndef HACKERS_EDITION
        return ((i_searchSize + remoteSearchResult->booksList.size() + 1) / books.size()) + 1;
#else
        return ((i_searchSize +1) / books.size()) + 1;
#endif
}

void Search::openPath(const QString& path)
{
        qDebug() << Q_FUNC_INFO;
        const BookInfo* book = QBookApp::instance()->getModel()->getBookInfo(path);
        if(book)
            QBookApp::instance()->openContent(book);
        else if(QBookApp::instance()->isImage(path))
        {
            QBookApp::instance()->getLibrary()->setImageList(m_images);
            QBookApp::instance()->openContent(new BookInfo(path));
            QBookApp::instance()->goToLibrary();
        }
        else
        {
            QFileInfo fi(path);
            if(fi.isDir())
            {
                QBookApp::instance()->getLibrary()->openSearchPath(path, true);
                QBookApp::instance()->goToLibrary();
            }
            else
            {
                InfoDialog* dialog = new InfoDialog(this,tr("Document format not supported"));
                dialog->hideSpinner();
                dialog->showForSpecifiedTime();
                delete dialog;
            }
        }
}

#ifndef HACKERS_EDITION
void Search::openStore(const BookInfo* book)
{
        qDebug() << Q_FUNC_INFO;
        QString bookUrl = QBook::settings().value("serviceURLs/book","").toString();
        bookUrl.append(book->isbn);
        qDebug() << "--->" << Q_FUNC_INFO << "url" << bookUrl;
        QBookApp::instance()->openStoreWithUrl(bookUrl);

}

void Search::openServiceSearch()
{
        qDebug() << Q_FUNC_INFO;

        if(!searchLineEdit->text().isEmpty())
        {
            QUrl searchUrl(QBook::settings().value("serviceURLs/search","").toString());
            searchUrl.addQueryItem("input",searchLineEdit->text());
            QBookApp::instance()->openStoreWithUrl(searchUrl.toString());
        }
}
#endif

void Search::keyboardPressedChar(const QString& input)
{
        qDebug() << Q_FUNC_INFO;

        if(!m_userTyped)
        {
            searchLineEdit->clear();
            searchLineEdit->setText(input);

            m_userTyped = true;

            disconnect(keyboard, SIGNAL(pressedChar(const QString &)), this, SLOT(keyboardPressedChar(const QString &)));
        }
}

void Search::handleClicked()
{
        qDebug() << Q_FUNC_INFO;

        handleKeyboard();

        if(!m_userTyped)
            searchLineEdit->clear();
}

void Search::handleSearchFinished()
{
        qDebug() << Q_FUNC_INFO;

        verticalPagerHandler->setup(getTotalPages());
        m_powerLock->release();
        show();
        QBookApp::instance()->getStatusBar()->setSpinner(false);
}

/** Refreshes local results when the model changes (SD extracted) */
void Search::removeResults()
{
        if(m_files.size() + m_books.size() + m_dirs.size() <= 0) // No results, no need to refresh
            return;

        deleteData();
        page = 0;
        qDebug() << Q_FUNC_INFO << "Searching" << qs_lastSearch;
        m_abortSearch = false;
        searchInLibrary(qs_lastSearch);
        m_abortSearch = true;
        qDebug() << Q_FUNC_INFO << "Found" << m_files.size() + m_books.size() + m_dirs.size();
        verticalPagerHandler->setup(getTotalPages());
        if(isVisible())
            show();
}

void Search::paintEvent (QPaintEvent *)
{
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
