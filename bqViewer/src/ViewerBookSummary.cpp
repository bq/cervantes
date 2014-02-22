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


#include "ViewerBookSummary.h"

#include "QDocView.h"
#include "QBookApp.h"
#include "bqUtils.h"
#include "BookInfo.h"
#include "PopUp.h"
#include "ViewerBookListActions.h"
#include "ViewerCollectionLayer.h"
#include "QBook.h"
#include "Screen.h"
#include "Model.h"
#include "Storage.h"
#include "Viewer.h"
#include "SelectionDialog.h"
#include "InfoDialog.h"
#include "PowerManager.h"
#include "Library.h"

#include <QWidget>
#include <QDebug>
#include <QScrollBar>
#include <QPainter>

#define BOOKSUMMARY_TITLE_MAX_LENGTH 60
#define BOOKSUMMARY_TITLE_MAX_LENGTH_SAMPLE 30
#define BOOKSUMMARY_AUTHOR_MAX_LENGTH 30
#define ICON_VIEWER_STRING_MAX_LENGTH 10
#define PERCENT_STEP_VALUE 0.75
#define COLLECTION_MAX_LENGTH 12

#define USERGUIDEPATH "/app/share/userGuides/"

#define FONT_SIZE_HD "font-size:24px;"
#define LITTLE_FONT_SIZE_HD "font-size:22px;"
#define FONT_SIZE_SD "font-size:18px;"
#define LITTLE_FONT_SIZE_SD "font-size:15px;"

ViewerBookSummary::ViewerBookSummary(Viewer* viewer) :
    ViewerMenuPopUp(viewer), m_powerLock(NULL)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);

    m_viewerListActions = new ViewerBookListActions(this);
    m_viewerListActions->hide();
    connect(m_viewerListActions,    SIGNAL(readStateChanged(int)),   this, SLOT(changeReadState(int)));

    m_collectionLayer = new ViewerCollectionLayer(this);
    m_collectionLayer->hide();
    connect(m_collectionLayer, SIGNAL(addCollection(QString)),   this, SLOT(addBookToCollection(QString)));
    connect(m_collectionLayer, SIGNAL(removeCollection(QString)),   this, SLOT(removeBookToCollection(QString)));
    connect(m_collectionLayer, SIGNAL(createCollection()), this, SLOT(createNewCollection()));

    connect(closeBtn,               SIGNAL(clicked()),          this,           SIGNAL(hideMe()));

    connect(copyToSDBtn,            SIGNAL(clicked()),          this,           SLOT(copyBook()));
    connect(exportNotesBtn,         SIGNAL(clicked()),          this,           SLOT(exportNotes()));
    connect(removeBtn,              SIGNAL(clicked()),          this,           SLOT(deleteBook()));
    connect(archiveBtn,             SIGNAL(clicked()),          this,           SLOT(storeBook()));
    connect(moreActionsBtn,         SIGNAL(clicked()),          this,           SLOT(moreActionsClicked()));
    connect(addToCollectionBtn,     SIGNAL(clicked()),          this,           SLOT(addToCollectionClicked()));
    connect(buyBookBtn,             SIGNAL(clicked()),          this,           SLOT(buyBook()));

    connect(VerticalPager,          SIGNAL(previousPageReq()),  this,           SLOT(previousPage()));
    connect(VerticalPager,          SIGNAL(nextPageReq()),      this,           SLOT(nextPage()));

    summaryTextBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vbar = summaryTextBrowser->verticalScrollBar();

    m_powerLock = PowerManager::getNewLock(this);
}

ViewerBookSummary::~ViewerBookSummary()
{
    qDebug() << Q_FUNC_INFO;

    delete m_viewerListActions;
    m_viewerListActions = NULL;

    delete m_collectionLayer;
    m_collectionLayer = NULL;
}

void ViewerBookSummary::setup()
{
}

void ViewerBookSummary::start()
{
    setBook(m_parentViewer->getCurrentBookInfo());
}

void ViewerBookSummary::stop()
{
    m_viewerListActions->hide();
    m_collectionLayer->hide();
}

void ViewerBookSummary::setBook(const BookInfo* book)
{
    qDebug() << Q_FUNC_INFO;

    m_book = book;
    buyBookBtn->hide();
    bookPriceLbl->hide();

    QList<BookLocation::Type> types;
    types.append(BookLocation::NOTE);
    types.append(BookLocation::HIGHLIGHT);
    int countMarks = book->getLocationList(types)->size();

    // Cover
    if (book->thumbnail == NO_COVER_RESOURCE || book->thumbnail.isEmpty())
    {
        // Title
        titleLbl->setText(bqUtils::truncateStringToLength(book->title, ICON_VIEWER_STRING_MAX_LENGTH));
        titleLbl->show();

        // Author
        if(book->author == "---")
            authorLbl->setText(bqUtils::truncateStringToLength(tr("Autor Desconocido"), ICON_VIEWER_STRING_MAX_LENGTH));
        else
            authorLbl->setText(bqUtils::truncateStringToLength(book->author, ICON_VIEWER_STRING_MAX_LENGTH));
        authorLbl->show();

        // Cover
        coverLbl->setPixmap(NULL);
        QString imageCover = QBookApp::instance()->getImageResource(book->path);
        coverLbl->setStyleSheet("image:url(" + imageCover + ")");
    }
    else
    {
        titleLbl->hide();
        authorLbl->hide();
        QPixmap pixmap(book->thumbnail);
        coverLbl->setPixmap(pixmap.scaled(coverLbl->size(),Qt::KeepAspectRatioByExpanding));
    }

    // Title
    if(m_book->m_type != BookInfo::BOOKINFO_TYPE_DEMO)
        bookTitleLbl->setText(bqUtils::truncateStringToLength(m_book->title, BOOKSUMMARY_TITLE_MAX_LENGTH));
    else
        bookTitleLbl->setText(bqUtils::truncateStringToLength(m_book->title, BOOKSUMMARY_TITLE_MAX_LENGTH_SAMPLE));

    // Author
    if(m_book->author == "--")
        bookAuthorLbl->setText(tr("Autor Desconocido"));
    else
        bookAuthorLbl->setText(bqUtils::truncateStringToLength(m_book->author, BOOKSUMMARY_AUTHOR_MAX_LENGTH));

    bookAuthorLbl->setWordWrap(true);
    bookTitleLbl->setWordWrap(true);

    int pages = m_book->pageCount;
    numberTotalPagesLbl->setText(QString::number(pages));

    synopsisLbl->setText(tr("Sinopsis"));
    summaryTextBrowser->setText(m_book->synopsis);
    if(!m_book->synopsis.isEmpty())
        synopsisLbl->show();
    else
        synopsisLbl->hide();

    //Define a single step as the 75% of the normal height in the widget.
    vbar->setSingleStep(summaryTextBrowser->height()*PERCENT_STEP_VALUE);

    m_viewerListActions->setButtonsState(m_book->readingStatus);
    setActionsBtnText(m_book->readingStatus);
    QStringList bookCollectionList = m_book->getCollectionsList();
    m_collectionLayer->setup(bookCollectionList);
    setCollectionLayerBtnText(bookCollectionList);

    //Format
    formatLbl->setText(m_book->format);

    if(m_book->m_type != BookInfo::BOOKINFO_TYPE_DEMO && !m_book->m_archived && countMarks > 0)
        exportNotesBtn->show();
    else
        exportNotesBtn->hide();

    if(m_book->path.contains(USERGUIDEPATH))
    {
        copyToSDBtn->hide();
        removeBtn->hide();
        moreActionsBtn->hide();
        archiveBtn->hide();
    }

    else if(!m_book->path.contains(Storage::getInstance()->getPrivatePartition()->getMountPoint()) )
    {
        copyToSDBtn->show();
        copyToSDBtn->setText(tr("Copy file"));
        removeBtn->show();
        archiveBtn->hide();
    }
    else
    {
        copyToSDBtn->hide();

        if(m_book->m_type == BookInfo::BOOKINFO_TYPE_DEMO)
        {
            buyBookBtn->show();
            QString euro = QString::fromUtf8("\u20AC");
            QString storePrice = book->storePrice + euro;
            if(!storePrice.isEmpty())
            {
                bookPriceLbl->setText(storePrice);
                buyBookBtn->show();
                priceCont->show();
                removeBtn->show();
                archiveBtn->hide();
            }
        }
        else
        {
            removeBtn->hide();
            archiveBtn->show();

            if(m_book->m_archived)
                archiveBtn->setText(QApplication::translate("ViewerBookSummary", "Unarchive", 0, QApplication::UnicodeUTF8));
            else
                archiveBtn->setText(QApplication::translate("ViewerBookSummary", "Archive", 0, QApplication::UnicodeUTF8));
        }
    }

    summaryTextBrowser->verticalScrollBar()->hide();
    setupPagination();
}

void ViewerBookSummary::setupPagination ()
{
    qDebug() << Q_FUNC_INFO;

    if (vbar)
    {
        int totalPages;
        if(vbar->maximum() % vbar->singleStep() == 0)
            totalPages = vbar->maximum() / vbar->singleStep() + 1;
        else
            totalPages = vbar->maximum() / vbar->singleStep() + 2;
        VerticalPager->setup(totalPages, 1, false, true);
    }
    VerticalPager->hideLabels();
}

void ViewerBookSummary::previousPage ()
{
    qDebug() << Q_FUNC_INFO;

    if (vbar)
        vbar->triggerAction(QAbstractSlider::SliderSingleStepSub);
}

void ViewerBookSummary::nextPage ()
{
    qDebug() << Q_FUNC_INFO;

    if (vbar)
         vbar->triggerAction(QAbstractSlider::SliderSingleStepAdd);
}

void ViewerBookSummary::buyBook()
{
#ifndef HACKERS_EDITION
    qDebug() << Q_FUNC_INFO;
    QString shopBookUrl = QBook::settings().value("serviceURLs/book").toString() + m_book->isbn;
    QBookApp::instance()->openStoreWithUrl(shopBookUrl);
#endif
}

void ViewerBookSummary::moreActionsClicked()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();
    if(!m_viewerListActions->isVisible())
    {
        QPoint pos(moreActionsBtn->mapToGlobal(QPoint(0,0)));
        pos.setY(pos.y() + moreActionsBtn->height());
        m_viewerListActions->move(mapFromGlobal(pos));
        m_viewerListActions->resize(moreActionsBtn->width(), m_viewerListActions->height());
        m_viewerListActions->show();

        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
    }
    else
    {
        m_viewerListActions->hide();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    }
    Screen::getInstance()->flushUpdates();

}

void ViewerBookSummary::addToCollectionClicked()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();
    if(!m_collectionLayer->isVisible())
    {
        QPoint pos(addToCollectionBtn->mapToGlobal(QPoint(0,0)));
        pos.setY(pos.y() + addToCollectionBtn->height());
        m_collectionLayer->move(mapFromGlobal(pos));
        m_collectionLayer->resize(addToCollectionBtn->width(), m_collectionLayer->height());
        m_collectionLayer->paint();
        m_collectionLayer->show();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
    }
    else
    {
        m_collectionLayer->hide();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    }
    Screen::getInstance()->flushUpdates();
}

void ViewerBookSummary::storeBook()
{
    qDebug() << Q_FUNC_INFO;

    QString archiveText = tr("You are about to archive the book") + " ";
    archiveText += tr("from") + " ";

    QString shop = QBook::settings().value("shopName", "Tienda").toString();
    archiveText += shop + " eBooks. \n"; archiveText += tr("This action will remove the book from device. ");
    archiveText += tr("The book will be always available for download.");

    SelectionDialog* archiveDialog = new SelectionDialog(this, archiveText, tr("Archive"));
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
    archiveDialog->exec();

    if(archiveDialog->result())
    {
        Screen::getInstance()->queueUpdates();
        QBookApp::instance()->goToHome();
        QBookApp::instance()->getModel()->archiveBook(QBookApp::instance()->getModel()->getBookInfo(m_book->path));

        InfoDialog* dialog = new InfoDialog(this,tr("Book correctly stored."));
        dialog->hideSpinner();
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
        dialog->showForSpecifiedTime();
        delete dialog;
    }
}

void ViewerBookSummary::deleteBook()
{
    qDebug() << Q_FUNC_INFO;

    QString deleteText = tr("You are about to remove the book ");
    deleteText += tr("from ");

    if(m_book->path.contains(Storage::getInstance()->getPrivatePartition()->getMountPoint()))
    {
        QString shop = QBook::settings().value("shopName", "Tienda").toString();
        deleteText += shop + " eBooks. \n";
    }
    else if(m_book->path.contains(Storage::getInstance()->getPublicPartition()->getMountPoint()))
        deleteText += tr("internal memory. \n");
    else if(Storage::getInstance()->getRemovablePartition() && m_book->path.contains(Storage::getInstance()->getRemovablePartition()->getMountPoint()))
        deleteText += tr("SD card. \n");

    deleteText += tr("This action will remove the book from device. ");

    if(m_book->path.contains(Storage::getInstance()->getPrivatePartition()->getMountPoint())) {
        if(m_book->m_type==BookInfo::BOOKINFO_TYPE_DEMO)
            deleteText += tr("The sample book will not be available for download.");
        else
            deleteText += tr("The book will be always available for download.");
    }

    SelectionDialog* deleteDialog = new SelectionDialog(this, deleteText, tr("Remove"));
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
    deleteDialog->exec();

    if(deleteDialog->result())
    {
        m_viewerListActions->hide();
        InfoDialog* dialog = new InfoDialog(this,tr("Book correctly removed."));
        dialog->hideSpinner();
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_PARTIALSCREEN_UPDATE,Q_FUNC_INFO);
        dialog->showForSpecifiedTime();
        delete dialog;
        QBookApp::instance()->getModel()->removeBook(m_book);
        QtConcurrent::run(QBookApp::instance(), &QBookApp::syncModel);
    }

    delete deleteDialog;
}

void ViewerBookSummary::copyBook()
{
    QFileInfo fi = QFileInfo(m_book->path);
    QString destination;

    QString copyText = tr("You are about to copy the book ");

   if(m_book->path.contains(Storage::getInstance()->getPublicPartition()->getMountPoint()) && Storage::getInstance()->getRemovablePartition()) {
        copyText += tr("from internal memory to SD.");
        destination = Storage::getInstance()->getRemovablePartition()->getMountPoint() + "/" + fi.fileName();
   }
    else if(Storage::getInstance()->getRemovablePartition() && m_book->path.contains(Storage::getInstance()->getRemovablePartition()->getMountPoint())) {
        copyText += tr("from SD card to internal memory.");
        destination = Storage::getInstance()->getPublicPartition()->getMountPoint() + "/" + fi.fileName();
   }

   else if(m_book->path.contains(Storage::getInstance()->getPublicPartition()->getMountPoint()) && !Storage::getInstance()->getRemovablePartition()){
       InfoDialog* errorDialog = new InfoDialog(this,tr("SD card is not mounted."));
       errorDialog->hideSpinner();
       errorDialog->showForSpecifiedTime();
       delete errorDialog;
       return;
   }

   copyText += tr("\nDo you want to continue?");
   SelectionDialog* copyDialog = new SelectionDialog(this, copyText, tr("Copy"));
   Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
   copyDialog->exec();

   if(copyDialog->result())
   {
       QBookApp::instance()->getStatusBar()->setBusy(true);
       m_powerLock->activate();
       QFile file(destination);
       m_viewerListActions->hide();
       Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
       if(QFile::copy(m_book->path, destination))
       {
           QBookApp::instance()->getStatusBar()->setBusy(false);
           InfoDialog* dialog = new InfoDialog(this,tr("File correctly copied."));
           dialog->hideSpinner();
           dialog->showForSpecifiedTime();
           QBookApp::instance()->getModel()->loadDefaultInfo(destination);
           QBookApp::instance()->syncModel();
           delete dialog;
       }
       else {
           QBookApp::instance()->getStatusBar()->setBusy(false);
           InfoDialog* dialog;
           if(!file.exists())
               dialog = new InfoDialog(this,tr("Copy has failed. Please check your SD card."));
           else
               dialog = new InfoDialog(this,tr("Copy failure, the file already exists."));

           dialog->hideSpinner();
           dialog->showForSpecifiedTime();
           delete dialog;
       }
       m_powerLock->release();
   }
   delete copyDialog;
}

void ViewerBookSummary::exportNotes()
{
    qDebug() << Q_FUNC_INFO;

    QString infoText = tr("You are about to export notes of this book ");
    QFileInfo fi = QFileInfo(m_book->path);
    QString destination;

    if(m_book->path.contains(Storage::getInstance()->getPrivatePartition()->getMountPoint())){
         infoText += tr("from your library to internal memory.");
         destination = Storage::getInstance()->getPublicPartition()->getMountPoint() + "/" + fi.fileName();
    }
    else if(m_book->path.contains(Storage::getInstance()->getPublicPartition()->getMountPoint())){
        infoText += tr("from internal memory to internal memory.");
        destination = Storage::getInstance()->getPublicPartition()->getMountPoint() + "/" + fi.fileName();
    }
    else if(Storage::getInstance()->getRemovablePartition() && m_book->path.contains(Storage::getInstance()->getRemovablePartition()->getMountPoint())) {
         infoText += tr("from SD card to SD card.");
         destination = Storage::getInstance()->getRemovablePartition()->getMountPoint() + "/" + fi.fileName();
    }

    infoText += tr("\nDo you want to continue?");

    SelectionDialog* exportDialog = new SelectionDialog(this, infoText, tr("Export"));
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
    exportDialog->exec();

    if(exportDialog->result())
    {
        QBookApp::instance()->getStatusBar()->setBusy(true);
        Screen::getInstance()->queueUpdates();
        m_parentViewer->calculateLocationsChapter();
        m_viewerListActions->hide();
        if(QBookApp::instance()->getModel()->writeNotesToFile(m_book, "html", destination))
        {
            QBookApp::instance()->getStatusBar()->setBusy(false);
            InfoDialog* dialog = new InfoDialog(this,tr("Notes exported correctly."));
            dialog->hideSpinner();
            Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
            Screen::getInstance()->flushUpdates();
            dialog->showForSpecifiedTime();
            delete dialog;
        }else{
            QBookApp::instance()->getStatusBar()->setBusy(false);
            Screen::getInstance()->flushUpdates();
        }
    }

    delete exportDialog;
}

void ViewerBookSummary::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ViewerBookSummary::setActionsBtnText(BookInfo::readStateEnum state)
{
    switch(state)
    {
    case BookInfo::READING_BOOK:
        moreActionsBtn->setText(tr("Leyendo"));
        break;
    case BookInfo::READ_BOOK:
        moreActionsBtn->setText(tr("Leido"));
        break;
    case BookInfo::NO_READ_BOOK: default:
        moreActionsBtn->setText(tr("No leido"));
        break;
    }
}

void ViewerBookSummary::changeReadState(int state)
{
    qDebug() << Q_FUNC_INFO;
    BookInfo::readStateEnum stateEnum = (BookInfo::readStateEnum)state;

    QString readText = tr("You are about to change the book state ");
    readText += tr("from ");

    if(m_book->path.contains(Storage::getInstance()->getPrivatePartition()->getMountPoint()))
    {
        QString shop = QBook::settings().value("shopName", "Tienda").toString();
        readText += shop + " eBooks. \n";
    }
    else if(m_book->path.contains(Storage::getInstance()->getPublicPartition()->getMountPoint()))
        readText += tr("internal memory. \n");
    else if(Storage::getInstance()->getRemovablePartition() && m_book->path.contains(Storage::getInstance()->getRemovablePartition()->getMountPoint()))
        readText += tr("SD card. \n");

    switch(stateEnum)
    {
    case BookInfo::NO_READ_BOOK:
        readText += tr("This book will be marked as no read. ");
    break;
    case BookInfo::READING_BOOK:
        readText += tr("This book will be marked as now reading. ");
    break;
    case BookInfo::READ_BOOK:
        readText += tr("This book will be marked as read. ");
    break;
    }

    SelectionDialog* readDialog = new SelectionDialog(this, readText, tr("Change"));
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
    readDialog->exec();

    if(!readDialog->result())
        return;

    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->goToHome();
    QBookApp::instance()->getModel()->changeReadState(QBookApp::instance()->getModel()->getBookInfo(m_book->path), stateEnum);

    InfoDialog* dialog = new InfoDialog(this,tr("Book correctly changed."));
    dialog->hideSpinner();
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
    dialog->showForSpecifiedTime();
}

void ViewerBookSummary::setCollectionLayerBtnText(QStringList collectionList)
{
    qDebug() << Q_FUNC_INFO;
    if(collectionList.empty())
        addToCollectionBtn->setText(tr("Colecciones"));
    else if (collectionList.size() == 1)
    {
        if(QBook::getInstance()->getResolution() == QBook::RES758x1024)
        {
            if(collectionList[0].size() >= 10)
                addToCollectionBtn->setStyleSheet(LITTLE_FONT_SIZE_HD);
            else
                addToCollectionBtn->setStyleSheet(FONT_SIZE_HD);
        }
        else
        {
            if(collectionList[0].size() >= 10)
                addToCollectionBtn->setStyleSheet(LITTLE_FONT_SIZE_SD);
            else
                addToCollectionBtn->setStyleSheet(FONT_SIZE_SD);
        }
        addToCollectionBtn->setText(bqUtils::truncateStringToLength(tr("%1").arg(collectionList[0]), COLLECTION_MAX_LENGTH));

    }
    else
    {
        if(QBook::getInstance()->getResolution() == QBook::RES758x1024)
        {
            if(collectionList.size() >= 10)
                addToCollectionBtn->setStyleSheet(LITTLE_FONT_SIZE_HD);
            else
                addToCollectionBtn->setStyleSheet(FONT_SIZE_HD);
        }
        addToCollectionBtn->setText(tr("En %1 colecciones").arg(collectionList.size()));
    }
}

void ViewerBookSummary::addBookToCollection(QString collectionName)
{
    qDebug() << Q_FUNC_INFO;
    BookInfo* book = new BookInfo(*m_book);
    book->addCollection(collectionName);
    QStringList bookCollectionList = book->getCollectionsList();
    setCollectionLayerBtnText(bookCollectionList);
    QBookApp::instance()->getModel()->updateBook(book);
    m_book = QBookApp::instance()->getModel()->getBookInfo(book->path);
    delete book;
}

void ViewerBookSummary::removeBookToCollection(QString collectionName)
{
    qDebug() << Q_FUNC_INFO;
    BookInfo* book = new BookInfo(*m_book);
    book->removeCollection(collectionName);
    QStringList bookCollectionList = book->getCollectionsList();
    setCollectionLayerBtnText(bookCollectionList);
    QBookApp::instance()->getModel()->updateBook(book);
    m_book = QBookApp::instance()->getModel()->getBookInfo(book->path);
    delete book;
}

void ViewerBookSummary::createNewCollection()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    m_parentViewer->hideAllElements();
    const BookInfo* book = QBookApp::instance()->getModel()->getBookInfo(m_book->path);
    qDebug() << Q_FUNC_INFO << book->title << book->path;
    emit addNewCollection(book);
    Screen::getInstance()->flushUpdates();
}
