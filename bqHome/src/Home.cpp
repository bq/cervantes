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

#include <QDateTime>
#include <QPainter>
#include <QSize>
#include "Home.h"
#include "QBook.h"
#include "QBookForm.h"
#include "BookInfo.h"
#include "QBookApp.h"
#include "Screen.h"
#include "HomeBooksListOptions.h"
#include "Model.h"
#include "DeviceInfo.h"
#include "Library.h"
#include "bqUtils.h"
#include "Storage.h"

#ifndef HACKERS_EDITION
#include "bqDeviceServices.h"
#endif

#define ICON_HOME_STRING_MAX_LENGTH 13

Home::Home(QWidget *parent) :  QBookForm(parent),
          m_category1(0)
        , m_category2(0)
        , m_category3(0)
        , memoryCategory1List(0)
        , memoryCategory2List(0)
        , memoryCategory3List(0)
        , b_isMostRecentFromSD(false)
      {
        setupUi(this);

        connect(libraryBtn,     SIGNAL(clicked()), this, SLOT(goToLibrary()));
        connect(helpBtn,        SIGNAL(clicked()), this, SLOT(goToHelp()));
        connect(searchBtn,      SIGNAL(clicked()), this, SLOT(goToSearch()));
        connect(settingsBtn,    SIGNAL(clicked()), this, SLOT(goToSettings()));
        connect(showAllBtn,     SIGNAL(clicked()), this, SLOT(viewAll()));
        connect(recommendedBtn, SIGNAL(clicked()), this, SLOT(showOptions()));

    #ifndef HACKERS_EDITION
        connect(storeBtn,       SIGNAL(clicked()), QBookApp::instance(),SLOT(goToShop()));
    #endif
        connect(readBookBtn, SIGNAL(clicked()), this, SLOT(openMostRecentBook()));

        QFile fileSpecific(":/res/home_styles.qss");
        QFile fileCommons(":/res/home_styles_generic.qss");
        fileSpecific.open(QFile::ReadOnly);
        fileCommons.open(QFile::ReadOnly);

        QString styles = QLatin1String(fileSpecific.readAll() + fileCommons.readAll());
        setStyleSheet(styles);

        if(QBook::getResolution() == QBook::RES600x800){
            defaultHelpBook = new BookInfo(QString(USERGUIDEPATH) + "/userGuideTouch_" + QBook::settings().value("setting/language", QVariant("es")).toString()+ ".epub");
        }else{ //We do it to preserve back compatibility
            defaultHelpBook = new BookInfo(QString(USERGUIDEPATH) + "/userGuide_" + QBook::settings().value("setting/language", QVariant("es")).toString()+ ".epub");

        }

        defaultHelpBook->title = tr("Guia de Ayuda");
        defaultHelpBook->thumbnail = ":/res/unknow_book.png";
        defaultHelpBook->fontSize = 1; //Optimal to show images

        homeBooksListOptions = new HomeBooksListOptions(this);
        connect(homeBooksListOptions, SIGNAL(viewSelected(Home::VIEW)), this, SLOT(viewSelected(Home::VIEW)));
        connect(homeBooksListOptions, SIGNAL(hideMe()), this, SLOT(showOptions()));
        connect(homeBooksList, SIGNAL(openBook(const BookInfo*)), this, SLOT(openBook(const BookInfo*)));

#ifndef HACKERS_EDITION
        memoryCategory1List = new bqDeviceServicesCategoryObject();
        memoryCategory2List = new bqDeviceServicesCategoryObject();
        memoryCategory3List = new bqDeviceServicesCategoryObject();
        m_category1 = new bqDeviceServicesCategoryObject();
        m_category2 = new bqDeviceServicesCategoryObject();
        m_category3 = new bqDeviceServicesCategoryObject();
#endif
        bookAccessLbl->hide();
        numberAccessLbl->hide();
        subscriptionTagLbl->hide();

        currentView = Home::RECENT;
}

Home::~Home() {
        qDebug() << Q_FUNC_INFO;
#ifndef HACKERS_EDITION
        delete m_category1;
        delete m_category2;
        delete m_category3;
#endif
        delete defaultHelpBook;
}

void Home::activateForm(){
        QBookApp::instance()->getStatusBar()->setMenuTitle(tr("Inicio"));
        setupHome();
        homeBooksListOptions->setVisible(false);
        QTimer::singleShot(100, QBookApp::instance(),SLOT(syncSubcriptionInfo()));
}

void Home::paintEvent (QPaintEvent *)
{
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void Home::deactivateForm(){
        qDebug() << Q_FUNC_INFO;
}

void Home::showEvent (QShowEvent * )
{
        qDebug() << Q_FUNC_INFO << "x:" << x() << "y:" << y();

}

void Home::setupHome(){
        qDebug() << Q_FUNC_INFO;

        recentBooks.clear();
        QBookApp::instance()->getModel()->getNowReadingBooks(recentBooks, "");
        qSort(recentBooks.begin(), recentBooks.end(), readingTimeAfterThan);

        if(recentBooks.size() == 0) {
            setMostRecentBook(defaultHelpBook);
        }
        else{
            setMostRecentBook(recentBooks.at(0));
            QBookApp::instance()->setScreensaverCover(recentBooks.at(0));
            QBookApp::instance()->getLibrary()->setReloadModel(true);
        }
        setupBooksList();
}

void Home::setupBooksList (){
        qDebug() << Q_FUNC_INFO;

        homeBooksList->show();
        recommendedBtn->show();
        showAllBtn->show();

        switch(currentView){
#ifndef HACKERS_EDITION
            case CATEGORY1:
                if(m_category1->booksList.size() > 0)
                {
                    recommendedBtn->setText(m_category1->name);
                    homeBooksList->setBooks(m_category1->booksList);
                }else
                {
                    currentView = Home::RECENT;
                    setupRecentBooks();
                }
                break;
            case CATEGORY2:
                if(m_category2->booksList.size() > 0)
                {
                    recommendedBtn->setText(m_category2->name);
                    homeBooksList->setBooks(m_category2->booksList);
                }else
                {
                    currentView = Home::RECENT;
                    setupRecentBooks();
                }
                break;
            case CATEGORY3:
                if(m_category3->booksList.size() > 0)
                {
                    recommendedBtn->setText(m_category3->name);
                    homeBooksList->setBooks(m_category3->booksList);
                }else
                {
                    currentView = Home::RECENT;
                    setupRecentBooks();
                }
                break;
#endif
            case RECENT:default:
                setupRecentBooks();
            break;
        }

#ifndef HACKERS_EDITION
        if(m_category1->booksList.size() > 0 || m_category2->booksList.size() > 0 || m_category3->booksList.size() > 0)
            recommendedBtn->show();
#endif

        homeBooksList->showList();
}

void Home::setupRecentBooks()
{
    qDebug() << Q_FUNC_INFO;
    recommendedBtn->setText(tr("Recientes"));
    if(recentBooks.size() >= 4)
    {
        homeBooksList->setBooks(recentBooks.mid(1, 3));
        homeBooksList->hide(0);
    }else if(recentBooks.size() == 3) {
        homeBooksList->setBooks(recentBooks.mid(1, 2));
        homeBooksList->hide(1);
    }
    else if(recentBooks.size() == 2) {
        homeBooksList->setBooks(recentBooks.mid(1, 1));
        homeBooksList->hide(2);
    }
    else if(recentBooks.size() <= 1) {
        homeBooksList->setBooks(QList<const BookInfo*>());
        homeBooksList->hide(3);
        recommendedBtn->hide();
        showAllBtn->hide();
    }
}

void Home::showOptions(){
        qDebug() << Q_FUNC_INFO;

        if(homeBooksListOptions->isVisible())
            homeBooksListOptions->setVisible(false);
        else{
#ifndef HACKERS_EDITION
            homeBooksListOptions->setupOptions(recentBooks.size() > 1, (m_category1 && m_category1->booksList.size()>0), m_category1->name, (m_category2 && m_category2->booksList.size()>0), m_category2->name, (m_category3 && m_category3->booksList.size()>0), m_category3->name, currentView);
#else
            homeBooksListOptions->setupOptions(recentBooks.size() > 1, false, "", false, "", false, "", currentView);
#endif
            homeBooksListOptions->setVisible(true);
        }
}

void Home::viewSelected(Home::VIEW _view){
        qDebug() << "--->" << Q_FUNC_INFO;
        Screen::getInstance()->queueUpdates();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        homeBooksListOptions->hide();
        currentView = _view;
        QBook::settings().setValue("setting/home/currentView", (int)currentView);
        setupBooksList();
        Screen::getInstance()->flushUpdates();
}

void Home::openBook(const BookInfo* book){
        qDebug() << Q_FUNC_INFO;

        if(book->path != NULL)
            QBookApp::instance()->openContent(book);
#ifndef HACKERS_EDITION
        else
        {
            QString privatePath = Storage::getInstance()->getPrivatePartition()->getMountPoint();
            const BookInfo* modelBook = QBookApp::instance()->getModel()->bookInPath(book->isbn, privatePath);
            if(modelBook && !modelBook->path.isEmpty())
            {
                qDebug() << Q_FUNC_INFO << "modelBook: " << modelBook->path << " archived: " << modelBook->m_archived;
                QBookApp::instance()->openContent(modelBook);
                return;
            }

            QString shopBookUrl = QBook::settings().value("serviceURLs/book").toString() + book->isbn;
            QDateTime expiredDate = QDateTime::fromString(QBook::settings().value("subscription/subscriptionEndDate").toString(), Qt::ISODate);
            QDateTime currentTime = QDateTime::currentDateTimeUtc();
            if(expiredDate.isValid() && expiredDate > currentTime)
                shopBookUrl = shopBookUrl + "&shopMode=subscription";

            QBookApp::instance()->openStoreWithUrl(shopBookUrl);
        }
#endif
}

void Home::goToLibrary() {
    qDebug() << Q_FUNC_INFO;
    QBookApp::instance()->goToLibrary();
}

void Home::viewAll(){
        qDebug() << Q_FUNC_INFO;
        if(currentView == RECENT)
            emit viewRecentBooks();
#ifndef HACKERS_EDITION
        else
        {
            QString shopUrl = QBook::settings().value("shopUrl").toString();
            int categoryId = getCategoryId();
            qDebug() << Q_FUNC_INFO << " categoryId:" << categoryId;
            shopUrl = shopUrl + "?categoryId=" + QString::number(categoryId);

            QDateTime expiredDate = QDateTime::fromString(QBook::settings().value("subscription/subscriptionEndDate").toString(), Qt::ISODate);
            QDateTime currentTime = QDateTime::currentDateTimeUtc();
            if(expiredDate.isValid() && expiredDate > currentTime)
                shopUrl = shopUrl + "&shopMode=subscription";

            QBookApp::instance()->openStoreWithUrl(shopUrl);
        }
#endif
}

void Home::goToHelp(){
        qDebug() << Q_FUNC_INFO << defaultHelpBook;
        QBookApp::instance()->openContent(defaultHelpBook);
}

void Home::goToSearch(){
        qDebug() << Q_FUNC_INFO;
        QBookApp::instance()->goToSearch();
}

void Home::goToSettings(){
        qDebug() << Q_FUNC_INFO;
        QBookApp::instance()->goToSettings();
}

void Home::openMostRecentBook(){
        qDebug() << Q_FUNC_INFO ;
        if(currentBook != defaultHelpBook)
            QBookApp::instance()->openLastContent();
        else
            goToHelp();

}

QString Home::resizeTextForRecent(QString text, bool isIcon){
        qDebug() << Q_FUNC_INFO;

        QString result = text;

        if(text.isNull()){
            return "";
        }

        if(isIcon && (result.size() > 28)){
            result.resize(25);
            result = result + "...";
        }

        if(!isIcon && result.size() > 42){
            result.resize(40);
            result = result + "...";
        }

        return result;
}

QString Home::resizeTextForLastBook(const QString& text){
        qDebug() << Q_FUNC_INFO;

        if(text.isNull()){
            return "";
        }

        QString result(text);
        if(result.size() > 52){
            result.resize(50);
            result = result + "...";
        }

        return result;

}

void Home::setMostRecentBook(const BookInfo* book){
        qDebug() << Q_FUNC_INFO;

        currentBook = book;

        if(book->m_type == BookInfo::BOOKINFO_TYPE_DEMO)
            sampleTagLbl->show();
        else
            sampleTagLbl->hide();

        /*if(book->m_type == BookInfo::BOOKINFO_TYPE_SUBSCRIPTION)
            subscriptionTagLbl->show();
        else
            subscriptionTagLbl->hide();*/

        newTagLbl->hide();

        if(book->author == "---")
            authorLbl->setText(tr("Autor Desconocido"));
        else
            authorLbl->setText(book->author);


        int percent = book->readingProgress * 100;
        if (percent > 0)
            readingPercentLbl->setText(tr("%1 % leído").arg(percent));
        else
            readingPercentLbl->setText(tr("0 % leído"));
        bookTitleLbl->setText(resizeTextForLastBook(book->title));

        if (book->thumbnail == NO_COVER_RESOURCE || book->thumbnail.isEmpty())
        {
            // Title
            coverTitleLbl->setText(bqUtils::truncateStringToLength(book->title, ICON_HOME_STRING_MAX_LENGTH));
            coverTitleLbl->show();

            // Author
            if(book->author == "--")
                coverAuthorLbl->setText(tr("Autor Desconocido"));
            else
                coverAuthorLbl->setText(bqUtils::truncateStringToLength(book->author, ICON_HOME_STRING_MAX_LENGTH));
            coverAuthorLbl->show();

            // Cover
            currentBookCoverLbl->setPixmap(NULL);
            QString coverImage = QBookApp::instance()->getImageResource(book->path);
            currentBookCoverLbl->setStyleSheet("background-image:url(" + coverImage + ")");
        }
        else
        {
            coverTitleLbl->hide();
            coverAuthorLbl->hide();
            QPixmap pixmap(book->thumbnail);
            currentBookCoverLbl->setPixmap(pixmap.scaled(currentBookCoverLbl->size(),Qt::KeepAspectRatioByExpanding));
        }

        if(book == defaultHelpBook){
            readingPercentLbl->hide();
            readLbl->hide();
            readingPercentLbl->hide();
            percentLbl->hide();
        }
        else{
            readingPercentLbl->show();
            readLbl->show();
            readingPercentLbl->show();
            percentLbl->show();
        };

        if(currentBook != defaultHelpBook)
        {
            dateAccessLbl->setText(getDateStyled(book->lastTimeRead, true));
            lastAccessLbl->show();
            dateAccessLbl->show();
        }
        else
        {
            lastAccessLbl->hide();
            dateAccessLbl->hide();
        }
        percentLbl->hide();
        readLbl->hide();
}

void Home::initHomeList()
{
    qDebug() << Q_FUNC_INFO;
#ifndef HACKERS_EDITION
    memoryCategory1List->booksList.clear();
    memoryCategory2List->booksList.clear();
    memoryCategory3List->booksList.clear();
#endif
}

void Home::getHomeBooksFinished()
{
    qDebug() << Q_FUNC_INFO;
#ifndef HACKERS_EDITION
    if(isVisible() && newHomeBooksReceived())
    {
        setNewHomeBooks();
        setupHome();
        QBook::settings().setValue("setting/home/categories", QVariant(QString(m_category1->name+";"+m_category2->name+";"+m_category3->name)));
        QBook::settings().sync();
    }
#endif
}

bool Home::newHomeBooksReceived()
{
    qDebug() << Q_FUNC_INFO;
#ifndef HACKERS_EDITION
    if(QBookApp::instance()->getModel()->bookListEquals(m_category1->booksList, memoryCategory1List->booksList) &&
       QBookApp::instance()->getModel()->bookListEquals(m_category2->booksList, memoryCategory2List->booksList) &&
       QBookApp::instance()->getModel()->bookListEquals(m_category3->booksList, memoryCategory3List->booksList))

        return false;

    return true;
#else
    return false;
#endif
}

void Home::setNewHomeBooks()
{
    qDebug() << Q_FUNC_INFO;

#ifndef HACKERS_EDITION
    m_category1 = memoryCategory1List;
    m_category2 = memoryCategory2List;
    m_category3 = memoryCategory3List;
#endif
}

void Home::doAddHomeBook(const QVariantMap &data, int type, int id, const QString& name)
{
    qDebug() << Q_FUNC_INFO << "type: " << type << "name: " << name;
#ifndef HACKERS_EDITION
    BookInfo *bookinfo = new BookInfo();
    QBookApp::instance()->fillBookInfoWithDataMap(bookinfo, data);

    switch(type){
    case 0:
    {
        memoryCategory1List->booksList.append(bookinfo);
        memoryCategory1List->name = name;
        memoryCategory1List->id = id;
        break;
    }
    case 1:
    {
        memoryCategory2List->booksList.append(bookinfo);
        memoryCategory2List->name = name;
        memoryCategory2List->id = id;
        break;
    }
    case 2:
    default:
    {
        memoryCategory3List->booksList.append(bookinfo);
        memoryCategory3List->name = name;
        memoryCategory3List->id = id;
        break;
    }
    }
#endif
}

QString Home::getDateStyled( const QDateTime& date, bool isIcon){
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
            if((today - date_book) == 1){
                if(isIcon){
                    return tr("Ayer") + date.toString(", hh:mm") + hourSufix;
                }else{
                    return tr("Ayer a las") + date.toString(" hh:mm") + hourSufix;
                }
            }
            if(today == date_book){
                if(isIcon){
                    return tr("Hoy") + date.toString(", hh:mm") + hourSufix;
                }else{
                    return tr("Hoy a las") + date.toString(" hh:mm") + hourSufix;
                }
            }
        }

        if(isIcon){
            return date.toString(tr("dd/MM, hh:mm")) + hourSufix;
        }else{
            return date.toString(tr("dd/MM/yy, ")) + tr("a las") + date.toString(" hh:mm") + hourSufix;
        }
}

void Home::modelChanged(QString path, int updateType)
{
    qDebug() << Q_FUNC_INFO << "path: " << path << "updateType: " << updateType; //If current home view contains a removed book, it calls setupHom
    //TODO only setupHome if home contains a book with path
    if(isVisible() && !QBookApp::instance()->isSynchronizing())
        setupHome();
}

void Home::mouseReleaseEvent( QMouseEvent* event )
{
    qDebug() << Q_FUNC_INFO;

    if(homeBooksListOptions->isVisible())
    {
        Screen::getInstance()->queueUpdates();
        event->accept();
        homeBooksListOptions->hide();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
    }
}

QSize Home::getHomeThumbnailSize(){
    qDebug() << Q_FUNC_INFO;

    return currentBookCoverLbl->size();
}

int Home::getCategoryId()
{
    qDebug() << Q_FUNC_INFO;
    int idValue = RECENT;
#ifndef HACKERS_EDITION
    switch(currentView)
    {
    case CATEGORY1:
        idValue = m_category1->id;
        break;
    case CATEGORY2:
        idValue = m_category2->id;
        break;
    case CATEGORY3:
        idValue = m_category3->id;
        break;
    case RECENT:default:
        break;
    }
#endif
    return idValue;
}
