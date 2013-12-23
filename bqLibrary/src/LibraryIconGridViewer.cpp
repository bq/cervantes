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

#include "LibraryIconGridViewer.h"

#include "Storage.h"
#include "Library.h"
#include "QBookApp.h"
#include "QBook.h"
#include "Model.h"
#include "EpubMetaDataExtractor.h"
#include "Fb2MetaDataExtractor.h"
#include "MobiMetaDataExtractor.h"
#include "QDocView.h"
#include "PowerManagerLock.h"
#include "PowerManager.h"
#include "Viewer.h"

#ifndef DISABLE_ADOBE_SDK
#include "AdobeDRM.h"
#endif

#include <QFile>
#include <QPainter>
#include <QDebug>
#include <QImageReader>
#include <QFile>

const int s_itemsPerPage = 6;
#define QBOOK_THUMBNAIL_CREATE_TIME 50

class QBookThumbnailEvent: public QEvent
{

public:
    enum { EventId = QEvent::User };
    QBookThumbnailEvent(const QString& _bookInfoPath, const QString &coverPath_, LibraryIconGridViewerItem* itemToUpdate_, int gridPage_)
        : QEvent(static_cast<QEvent::Type>(EventId)),
        bookInfoPath(_bookInfoPath),
        coverPath(coverPath_),
        itemToUpdate(itemToUpdate_),
        gridPage(gridPage_){}

    virtual ~QBookThumbnailEvent(){}
    QString                     bookInfoPath;
    QString                     coverPath;
    LibraryIconGridViewerItem*  itemToUpdate;
    int                         gridPage;
};

LibraryIconGridViewer::LibraryIconGridViewer(QWidget* parent) :
    LibraryGridViewer(parent)
  , m_isThumbnailGenerationStopped(true)
  , m_powerLock(NULL)
{
    setupUi(this);
}

LibraryIconGridViewer::~LibraryIconGridViewer()
{
    if(m_powerLock)
        delete m_powerLock;
    m_powerLock = NULL;
}

void LibraryIconGridViewer::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

LibraryPageHandler* LibraryIconGridViewer::getPageHandler()
{
    return VerticalPagerHandler;
}

int LibraryIconGridViewer::getItemsPerPage()
{
    return s_itemsPerPage;
}

void LibraryIconGridViewer::getBookItems( LibraryGridViewerItem* books[] )
{
    books[0] = book1;
    books[1] = book2;
    books[2] = book3;
    books[3] = book4;
    books[4] = book5;
    books[5] = book6;
}

void LibraryIconGridViewer::resume()
{
    qDebug() << Q_FUNC_INFO;

    LibraryGridViewer::resume();

    if(!m_powerLock)
        m_powerLock = PowerManager::getNewLock(this);

    // Run thumbnails
    resumeThumbnailGeneration();
}

void LibraryIconGridViewer::pause()
{
    qDebug() << Q_FUNC_INFO;

    // Stop thumbnails
    pauseThumbnailGeneration();

    waitForThumbnailRunner();

    if(m_powerLock)
        m_powerLock->release();

    LibraryGridViewer::pause();
}

void LibraryIconGridViewer::runGetThumbnail()
{
    qDebug() << Q_FUNC_INFO << "Begin... " << this;

    if(!m_isThumbnailGenerationStopped)
    {
        int itemsPerPage = getItemsPerPage();
        LibraryGridViewerItem* books[itemsPerPage];
        getBookItems(books);

        for(int i = 0; i < itemsPerPage; ++i)
        {
            const QString& path = books[i]->getPath();
            Model* model = QBookApp::instance()->getModel();
            const BookInfo* book = model->getBookInfo(path);
            if(!book)
                continue;
#ifndef DISABLE_ADOBE_SDK
            if(!AdobeDRM::getInstance()->isLinked() && book->isDRMFile)
                continue;
#endif

            QFileInfo thumbnailFile(book->thumbnail);

            bool coverToDownscale = !thumbnailFile.completeBaseName().endsWith(THUMBNAIL_SUFIX)
                                    && !book->thumbnail.endsWith(NO_COVER_RESOURCE)
                                    && !book->path.contains(Storage::getInstance()->getPrivatePartition()->getMountPoint())
                                    && !book->corrupted;

            if(book->thumbnail.isEmpty() || coverToDownscale)
            {
                qDebug()<< Q_FUNC_INFO << "Launching runCoverPage" << this;

                if(!m_isThumbnailGenerationStopped)
                {
                    m_powerLock->activate(); //Block sleep
                    m_futureThumbnail = QtConcurrent::run(this, &LibraryIconGridViewer::runCoverPage, this, book->path, (LibraryIconGridViewerItem*)books[i], m_library->m_page);
                }
                break;
            }
        }
    }
    qDebug()<< Q_FUNC_INFO << "End... " << this;
}

void LibraryIconGridViewer::runCoverPage( QObject* receiver, const QString& bookInfoPath, LibraryIconGridViewerItem* itemToUpdate, int gridPage )
{
    qDebug()<< Q_FUNC_INFO << "Memory: " << this << bookInfoPath;

    QBookApp::instance()->getStatusBar()->setSpinner(true);

    QFileInfo fi(bookInfoPath);

    // Check dir
    QDir dir(fi.absolutePath());
    if(!dir.exists( ".thumbnail/"))
        dir.mkdir( ".thumbnail/");

    QString coverPath = fi.absolutePath() + "/" + ".thumbnail" + "/" + fi.fileName() + THUMBNAIL_SUFIX;
    coverPath = coverPath.replace("\'", "");  //Books with apostrophe the title cant load the thumbnail already generated.

    /* Try first with cover before trying to render the first page */
    bool gotCover = MetaDataExtractor::extractCover(bookInfoPath, coverPath);
    if (!gotCover)
    {
        if(!m_isThumbnailGenerationStopped)
        {
            coverPath = QDocView::coverPage(bookInfoPath, coverPath);
        }
        else
        {
            QBookApp::instance()->getStatusBar()->setSpinner(false);
            return;
        }

    }

    QImageReader image(coverPath);
    QFile imageFile(coverPath);
    if(image.canRead())
    {

        if(!m_isThumbnailGenerationStopped)
        {
            Library::fromCover2Thumbnail(coverPath);
            QImageReader modifiedImage(coverPath);
            coverPath += "." + modifiedImage.format();
            imageFile.rename(coverPath);
        }
        else
        {
            QBookApp::instance()->getStatusBar()->setSpinner(false);
            return;
        }
    }
    else
    {
        qDebug() << Q_FUNC_INFO << "Cover image format not supported";
        imageFile.remove();
    }

    QBookApp::instance()->getStatusBar()->setSpinner(false);
    QApplication::postEvent(receiver, new QBookThumbnailEvent(bookInfoPath, coverPath, itemToUpdate, gridPage));
}

bool LibraryIconGridViewer::event(QEvent* event)
{
    qDebug()<< Q_FUNC_INFO << event;

    if(event->type() == static_cast<QEvent::Type>(QBookThumbnailEvent::EventId))
    {
        event->accept();

        QBookThumbnailEvent *thumbnailEvent = static_cast<QBookThumbnailEvent*>(event);

        if (m_library->m_page == thumbnailEvent->gridPage && !m_isThumbnailGenerationStopped)
        {
            const BookInfo* modelBookInfo = QBookApp::instance()->getModel()->getBookInfo(thumbnailEvent->bookInfoPath);
            if(modelBookInfo)
            {
                BookInfo* bookInfo = new BookInfo(*modelBookInfo);
                QFile thumb_file(thumbnailEvent->coverPath);
                if(thumb_file.exists())
                {
                    if(!bookInfo->thumbnail.isEmpty() && (bookInfo->thumbnail != thumbnailEvent->coverPath)){
                        QFile old_thumb_file(bookInfo->thumbnail);
                        if(old_thumb_file.exists())
                            old_thumb_file.remove();
                    }
                    bookInfo->thumbnail = thumbnailEvent->coverPath;
                    qDebug() << Q_FUNC_INFO <<"Setting cover..." << bookInfo->thumbnail;
                }
                else if(!bookInfo->corrupted) // New thumbnail generated
                {
                    bookInfo->thumbnail = NO_COVER_RESOURCE;
                    qDebug() << Q_FUNC_INFO << "Cover not found ..." << bookInfo->thumbnail;
                }

                // WORKAROUND We shouldn't generate thumbs here....
                QBookApp::instance()->getModel()->updateBook(bookInfo);

                thumbnailEvent->itemToUpdate->setBook(bookInfo, m_library->getFilterMode());

                delete bookInfo;
            }
        }

        // Unblock sleep
        m_powerLock->release();


        QTimer::singleShot(QBOOK_THUMBNAIL_CREATE_TIME, this, SLOT(runGetThumbnail()));
        return true;
    }

    bool value = LibraryGridViewer::event(event);

    qDebug()<< Q_FUNC_INFO << "End";
    return value;

}

void LibraryIconGridViewer::pauseThumbnailGeneration()
{
    qDebug() << Q_FUNC_INFO;

    m_isThumbnailGenerationStopped = true;
}

void LibraryIconGridViewer::resumeThumbnailGeneration()
{
    qDebug() << Q_FUNC_INFO;

    if(m_isThumbnailGenerationStopped && m_library->isVisible())
    {
        m_isThumbnailGenerationStopped = false;
        QTimer::singleShot(QBOOK_THUMBNAIL_CREATE_TIME, this, SLOT(runGetThumbnail()));
    }
}

void LibraryIconGridViewer::waitForThumbnailRunner()
{
    m_futureThumbnail.waitForFinished();
}
