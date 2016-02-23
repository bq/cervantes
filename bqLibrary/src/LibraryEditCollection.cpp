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

#include "LibraryEditCollection.h"

#include "Library.h"
#include "Model.h"
#include "bqUtils.h"
#include "BookInfo.h"
#include "GestureWidget.h"
#include "SelectionDialog.h"
#include "QBookApp.h"
#include <QPainter>
#include <QPoint>
#include <QDebug>
#include <QKeyEvent>
#include <QStringList>

#define ITEMS_PER_PAGE 14
#define EDIT_STRING_MAX_LENGTH 36

LibraryEditCollection::LibraryEditCollection(QWidget *parent) : GestureWidget(parent) , b_fromBookSummary(false), b_collectionSaved(false)
{
        setupUi(this);

        m_listBooks = new QHash<QString, bool>();

        items.append(book1);
        items.append(book2);
        items.append(book3);
        items.append(book4);
        items.append(book5);
        items.append(book6);
        items.append(book7);
        items.append(book8);
        items.append(book9);
        items.append(book10);
        items.append(book11);
        items.append(book12);
        items.append(book13);
        items.append(book14);

        for(int i= 0; i < items.size(); i++){
            connect(items.at(i), SIGNAL(changeStatus()), this, SLOT(setPageChanged()));
        }

        m_initialCollectionName = "";
        m_initialText = collectionNameLineEdit->text();
        saveCollectionBnt->setEnabled(false);

        connect(collectionNameLineEdit,  SIGNAL(clicked()), this, SLOT(handleCollectionNameLine()));
        connect(closeSearchBtn,          SIGNAL(clicked()), this, SLOT(handleClose()));
        connect(selectAllBtn,            SIGNAL(clicked()), this, SLOT(selectAll()));
        connect(unSelectAllBtn,          SIGNAL(clicked()), this, SLOT(unSelectAll()));
        connect(saveCollectionBnt,       SIGNAL(clicked()), this, SLOT(saveCollection()));
        connect(clearInputBtn,           SIGNAL(clicked()), this, SLOT(clearTextEdit()));

        connect(VerticalPager,   SIGNAL(previousPageReq()), this, SLOT(previousPage()));
        connect(VerticalPager,   SIGNAL(nextPageReq()),     this, SLOT(nextPage()));
        connect(VerticalPager,   SIGNAL(firstPage()),       this, SLOT(firstPage()));
        connect(VerticalPager,   SIGNAL(lastPage()),        this, SLOT(lastPage()));
}

LibraryEditCollection::~LibraryEditCollection()
{
    qDebug() << Q_FUNC_INFO;
    m_listBooks->clear();
    m_books.clear();
}

void LibraryEditCollection::paintEvent (QPaintEvent *)
{
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void LibraryEditCollection::setup (const QString& collection, const BookInfo* book)
{
    qDebug() << Q_FUNC_INFO;
    m_listBooks->clear();
    m_books.clear();
    m_collection = collection;
    b_collectionSaved = false;

    if(m_collection != "")
    {
        m_newCollection = false;
        saveCollectionBnt->setEnabled(true);
        m_initialCollectionName = collection;
        collectionNameLineEdit->setText(m_collection);
        collectionActionTitle->setText(tr("Editar %1").arg(bqUtils::truncateStringToLength(m_collection,EDIT_STRING_MAX_LENGTH)));
    }
    else
    {
        m_newCollection = true;
        saveCollectionBnt->setEnabled(false);
        collectionActionTitle->setText(tr("Crear Collecion"));
        collectionNameLineEdit->setText(m_initialText);
    }
    QBookApp::instance()->getModel()->getAllBooks(m_books);
    qSort(m_books.begin(), m_books.end(), titleLessThan);
    if(!book)
    {
        m_page = 0;
        b_fromBookSummary =false;
    }
    else
    {
        b_fromBookSummary =true;
        int pos = m_books.indexOf(book);
        m_page = pos / ITEMS_PER_PAGE;
    }
    if(m_books.size() % ITEMS_PER_PAGE == 0)
        m_totalPages = m_books.size() / ITEMS_PER_PAGE;
    else
        m_totalPages = m_books.size() / ITEMS_PER_PAGE + 1;
    paint();
    VerticalPager->setup(m_totalPages, m_page+1, true);
}

void LibraryEditCollection::paint()
{
    qDebug() << Q_FUNC_INFO;

    if(isSaveInHash())
    {
        paintFromHash();
        return;
    }

    int librarySize = m_books.size();

    const int pageOffset = m_page * ITEMS_PER_PAGE;

    for(int i = 0; i < ITEMS_PER_PAGE; ++i)
    {
        LibraryBookToCollectionItem* item = items[i];
        int pos = pageOffset + i;
        if(pos < librarySize)
        {
            const BookInfo* book = m_books.at(pos);
            item->setBook(book);
            QHash<QString, double> bookCollections = book->getCollectionsList();
            bool inCollection = false;
            if(bookCollections.find(m_collection) != bookCollections.end())
                inCollection = true;
            if(inCollection)
                item->setChecked(true);
            else
                item->setChecked(false);
            item->show();
        }
        else
        {
            item->hideItem();
        }
    }
}

void LibraryEditCollection::handleCollectionNameLine()
{
    qDebug() << Q_FUNC_INFO;
    if(collectionNameLineEdit->text() == m_initialText)
        collectionNameLineEdit->clear();

    // Show keyboard
    m_keyboard = QBookApp::instance()->showKeyboard(tr("Ocultar"));
    m_keyboard->handleMyQLineEdit(collectionNameLineEdit);
    connect(m_keyboard, SIGNAL(actionRequested()), this, SLOT(hideKeyboard()), Qt::UniqueConnection);
    //
}

void LibraryEditCollection::hideKeyboard()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->hideKeyboard();
    checkCollectionName();
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void LibraryEditCollection::saveCollection()
{
    qDebug() << Q_FUNC_INFO;

    m_collection = collectionNameLineEdit->text();

    if(m_collection == m_initialText || m_collection.trimmed().isEmpty())
        return;

    if(checkCollectionExist() && m_collection != m_initialCollectionName)
    {
        SelectionDialog* overrideCollection = new SelectionDialog(this, tr("¿Deseas sobreescribir la coleccion del mismo nombre? No se mantendran los libros guardados en ella."));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        overrideCollection->exec();
        int result = overrideCollection->result();
        delete overrideCollection;
        if(result == QDialog::Rejected)
            return;
    }
    b_collectionSaved = true;
    if(!m_newCollection && m_collection != m_initialCollectionName)
        changeOldCollection();
    saveItemsInfo();

    QHash<QString, bool>::const_iterator it = m_listBooks->constBegin();
    QHash<QString, bool>::const_iterator itEnd = m_listBooks->constEnd();

    for(; it != itEnd; ++it)
    {
        const BookInfo* book = QBookApp::instance()->getModel()->getBookInfo(it.key());
        if(!book)
            continue;
        BookInfo *bookInfo = new BookInfo(*book);
        if(it.value())
        {
            double index = QBookApp::instance()->getModel()->getBooksInCollectionCount(m_collection) + 1;
            bookInfo->addCollection(m_collection, index);
        }
        else
            bookInfo->removeCollection(m_collection);
        QBookApp::instance()->getModel()->updateBook(bookInfo);
    }
    QBookApp::instance()->getModel()->addCollection(m_collection);
    m_listBooks->clear();
    m_books.clear();
    emit hideMe();
}

void LibraryEditCollection::changeOldCollection()
{
    qDebug() << Q_FUNC_INFO;
    QList<const BookInfo *> books;
    QBookApp::instance()->getModel()->getBooksInCollection(books, m_initialCollectionName);
    QList<const BookInfo*>::const_iterator it = books.constBegin();
    QList<const BookInfo*>::const_iterator itEnd = books.constEnd();
    for(; it != itEnd; it++)
    {
        BookInfo *bookInfo = new BookInfo(*(*it));
        bookInfo->removeCollection(m_initialCollectionName);
        double index = QBookApp::instance()->getModel()->getBooksInCollectionCount(m_collection) + 1;
        bookInfo->addCollection(m_collection, index);
        QBookApp::instance()->getModel()->updateBook(bookInfo);
    }
    QBookApp::instance()->getModel()->removeCollection(m_initialCollectionName);
    books.clear();
}

void LibraryEditCollection::firstPage()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();

    saveItemsInfo();

    m_page = 0;
    m_pageChanged = false;
    VerticalPager->setPage(m_page+1);
    paint();

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void LibraryEditCollection::lastPage()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();

    saveItemsInfo();

    m_page = m_totalPages - 1;
    m_pageChanged = false;
    VerticalPager->setPage(m_page+1);
    paint();

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void LibraryEditCollection::nextPage()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();

    saveItemsInfo();

    m_page++;
    m_pageChanged = false;
    VerticalPager->setPage(m_page+1);
    paint();

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void LibraryEditCollection::previousPage()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();

    saveItemsInfo();

    m_page--;
    m_pageChanged = false;
    VerticalPager->setPage(m_page+1);
    paint();

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void LibraryEditCollection::saveItemsInfo()
{
    qDebug() << Q_FUNC_INFO;

    if(!isSaveInHash())
    {

        int librarySize = m_books.size();

        const int pageOffset = m_page * ITEMS_PER_PAGE;

        for(int i = 0; i < ITEMS_PER_PAGE; ++i)
        {
            LibraryBookToCollectionItem* item = items[i];
            int pos = pageOffset + i;
            if(pos < librarySize)
            {
                const BookInfo* book = m_books.at(pos);
                m_listBooks->insert(book->path, item->isChecked());
            }
        }
    }
    else if(m_pageChanged)
    {
        int librarySize = m_books.size();

        const int pageOffset = m_page * ITEMS_PER_PAGE;

        for(int i = 0; i < ITEMS_PER_PAGE; ++i)
        {
            LibraryBookToCollectionItem* item = items[i];
            int pos = pageOffset + i;
            if(pos < librarySize)
            {
                const BookInfo* book = m_books.at(pos);
                m_listBooks->insert(book->path, item->isChecked());
            }
        }
    }
}

void LibraryEditCollection::selectAll()
{
    qDebug() << Q_FUNC_INFO;
    m_pageChanged = true;

    Screen::getInstance()->queueUpdates();
    for(int i = 0; i < ITEMS_PER_PAGE; ++i)
    {
        LibraryBookToCollectionItem* item = items[i];
        if(item->isVisible())
            item->setChecked(true);
    }

    if(m_keyboard && m_keyboard->isVisible())
        m_keyboard->hide();
    checkCollectionName();

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void LibraryEditCollection::unSelectAll()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();
    m_pageChanged = true;

    for(int i = 0; i < ITEMS_PER_PAGE; ++i)
    {
        LibraryBookToCollectionItem* item = items[i];
        if(item->isVisible())
            item->setChecked(false);
    }

    if(m_keyboard && m_keyboard->isVisible())
        m_keyboard->hide();
    checkCollectionName();
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void LibraryEditCollection::clearTextEdit()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();

    collectionNameLineEdit->clear();
    checkCollectionName();
    Screen::getInstance()->flushUpdates();

}

bool LibraryEditCollection::isSaveInHash()
{
    qDebug() << Q_FUNC_INFO;
    bool isInHash = false;
    const int pageOffset = m_page * ITEMS_PER_PAGE;
    int librarySize = m_books.size();
    const BookInfo* firstBook = 0;
    if(pageOffset < librarySize)
        firstBook = m_books.at(pageOffset);

    QHash<QString, bool>::const_iterator it = m_listBooks->constBegin();
    QHash<QString, bool>::const_iterator itEnd = m_listBooks->constEnd();

    for(; it != itEnd; ++it)
    {
        const BookInfo* book = QBookApp::instance()->getModel()->getBookInfo(it.key());
        if(firstBook && book->path == firstBook->path)
        {
            isInHash = true;
            break;
        }
    }
    return isInHash;
}


void LibraryEditCollection::paintFromHash()
{
    qDebug() << Q_FUNC_INFO;

    int librarySize = m_books.size();

    const int pageOffset = m_page * ITEMS_PER_PAGE;

    for(int i = 0; i < ITEMS_PER_PAGE; ++i)
    {
        LibraryBookToCollectionItem* item = items[i];
        int pos = pageOffset + i;
        if(pos < librarySize)
        {
            const BookInfo* book = m_books.at(pos);
            item->setBook(book);
            bool inCollection = false;
            QHash<QString, bool>::const_iterator it = m_listBooks->constBegin();
            QHash<QString, bool>::const_iterator itEnd = m_listBooks->constEnd();

            for(; it != itEnd; ++it)
            {
                const BookInfo* bookInHash = QBookApp::instance()->getModel()->getBookInfo(it.key());
                if(book->path == bookInHash->path && it.value())
                {
                    inCollection = true;
                    break;
                }
            }
            if(inCollection)
                item->setChecked(true);
            else
                item->setChecked(false);
            item->show();
        }
        else
        {
            item->hideItem();
        }
    }
}

void LibraryEditCollection::setPageChanged()
{
    qDebug() << Q_FUNC_INFO;
    m_pageChanged = true;
    checkCollectionName();
    if(m_keyboard && m_keyboard->isVisible())
    {
        Screen::getInstance()->queueUpdates();
        m_keyboard->hide();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
    }
}

bool LibraryEditCollection::checkCollectionExist()
{
    qDebug() << Q_FUNC_INFO;

    QStringList collections = QBookApp::instance()->getModel()->getCollections();
    for(int i = 0; i < collections.size(); i++)
    {
        if(m_collection == collections.at(i))
            return true;
    }
    return false;
}

void LibraryEditCollection::handleClose()
{
    qDebug() << Q_FUNC_INFO;
    if(m_keyboard && m_keyboard->isVisible())
        m_keyboard->hide();
    m_listBooks->clear();
    m_books.clear();
    saveCollectionBnt->setEnabled(false);
    emit hideMe();
}

void LibraryEditCollection::checkCollectionName()
{
    qDebug() << Q_FUNC_INFO;
    if(collectionNameLineEdit->text().trimmed().isEmpty() || collectionNameLineEdit->text() == m_initialText)
        saveCollectionBnt->setEnabled(false);
    else if(!saveCollectionBnt->isEnabled())
        saveCollectionBnt->setEnabled(true);
}

void LibraryEditCollection::checkBookToAdd(const BookInfo* bookInfo)
{
    for(int i = 0; i < ITEMS_PER_PAGE; ++i)
    {
        LibraryBookToCollectionItem* item = items[i];
        if(item->getBook() == bookInfo)
        {
            item->setChecked(true);
            break;
        }
    }
}
