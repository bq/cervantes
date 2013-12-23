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

#include "LibraryBooksFilterLayer.h"

#include <QButtonGroup>
#include <QFile>
#include <QPainter>
#include <QDebug>
#include "Screen.h"
#include "QBookApp.h"
#include "QBook.h"

#define CHECKED "background-image: url(':/res/check.png')"
#define UNCHECKED "background-image:none;"


LibraryBooksFilterLayer::LibraryBooksFilterLayer(QWidget* parent) :
        QWidget(parent)
{
    setupUi(this);

    connect(allBooksBtn,    SIGNAL(clicked()), this, SLOT(myBooksClicked()));
    connect(storeBooksBtn,  SIGNAL(clicked()), this, SLOT(storeBooksClicked()));
    connect(myCollectionsBtn,  SIGNAL(clicked()), this, SLOT(myCollectionsClicked()));
    connect(browserFileBtn, SIGNAL(clicked()), this, SLOT(browserFileClicked()));

    m_allBooksName = allBooksBtn->text();
    m_storeBooksName = storeBooksBtn->text();

    allBooksBtn->setStyleSheet(CHECKED);
}

LibraryBooksFilterLayer::~LibraryBooksFilterLayer()
{}

void LibraryBooksFilterLayer::paintEvent( QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void LibraryBooksFilterLayer::myBooksClicked ()
{
    Screen::getInstance()->queueUpdates();
    allBooksBtn->setStyleSheet(CHECKED);
    storeBooksBtn->setStyleSheet(UNCHECKED);
    myCollectionsBtn->setStyleSheet(UNCHECKED);
    browserFileBtn->setStyleSheet(UNCHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit myBooksSelected();
}

void LibraryBooksFilterLayer::storeBooksClicked ()
{
    Screen::getInstance()->queueUpdates();
    allBooksBtn->setStyleSheet(UNCHECKED);
    storeBooksBtn->setStyleSheet(CHECKED);
    myCollectionsBtn->setStyleSheet(UNCHECKED);
    browserFileBtn->setStyleSheet(UNCHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit storeBooksSelected();
}

void LibraryBooksFilterLayer::myCollectionsClicked ()
{
    Screen::getInstance()->queueUpdates();
    allBooksBtn->setStyleSheet(UNCHECKED);
    storeBooksBtn->setStyleSheet(UNCHECKED);
    myCollectionsBtn->setStyleSheet(CHECKED);
    browserFileBtn->setStyleSheet(UNCHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit myCollectionsSelected();
}

void LibraryBooksFilterLayer::browserFileClicked ()
{
    Screen::getInstance()->queueUpdates();
    allBooksBtn->setStyleSheet(UNCHECKED);
    storeBooksBtn->setStyleSheet(UNCHECKED);
    myCollectionsBtn->setStyleSheet(UNCHECKED);
    browserFileBtn->setStyleSheet(CHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit browserFileSelected();
}

void LibraryBooksFilterLayer::setAllBooksChecked()
{
    allBooksBtn->setStyleSheet(CHECKED);
    storeBooksBtn->setStyleSheet(UNCHECKED);
    myCollectionsBtn->setStyleSheet(UNCHECKED);
    browserFileBtn->setStyleSheet(UNCHECKED);
}

void LibraryBooksFilterLayer::setStoreBooksChecked()
{
    allBooksBtn->setStyleSheet(UNCHECKED);
    storeBooksBtn->setStyleSheet(CHECKED);
    myCollectionsBtn->setStyleSheet(UNCHECKED);
    browserFileBtn->setStyleSheet(UNCHECKED);
}

void LibraryBooksFilterLayer::setBrowserChecked()
{
    allBooksBtn->setStyleSheet(UNCHECKED);
    storeBooksBtn->setStyleSheet(UNCHECKED);
    myCollectionsBtn->setStyleSheet(UNCHECKED);
    browserFileBtn->setStyleSheet(CHECKED);
}

void LibraryBooksFilterLayer::setCollectionsChecked()
{
    allBooksBtn->setStyleSheet(UNCHECKED);
    storeBooksBtn->setStyleSheet(UNCHECKED);
    myCollectionsBtn->setStyleSheet(CHECKED);
    browserFileBtn->setStyleSheet(UNCHECKED);
}

void LibraryBooksFilterLayer::setAllBooksNumber( int size )
{
    allBooksBtn->setText(m_allBooksName + " ("+ QString::number(size) + ")");
}

void LibraryBooksFilterLayer::setStoreBooksNumber( int size )
{
    #ifndef HACKERS_EDITION
    qDebug() << Q_FUNC_INFO << QBookApp::instance()->isLinked() << QBook::settings().value("shopName", "").toString();
    if(!QBookApp::instance()->isLinked())
    {
    #endif
        storeBooksBtn->hide();
    #ifndef HACKERS_EDITION
    }
    else
    {
        const QString& shopName = QBook::settings().value("shopName", "").toString();
        if(!shopName.isEmpty())
        {
            QString txt = m_storeBooksName.arg(shopName);
            storeBooksBtn->setText(txt + " ("+ QString::number(size) + ")");
            storeBooksBtn->show();
        }
        else
        {
            storeBooksBtn->hide();
        }
    }
    #endif
}

QString LibraryBooksFilterLayer::getAllBooksName() const
{
    return allBooksBtn->text();
}

QString LibraryBooksFilterLayer::getStoreBooksName() const
{
    return storeBooksBtn->text();
}

QString LibraryBooksFilterLayer::getMyCollectionsName() const
{
    return myCollectionsBtn->text();
}

QString LibraryBooksFilterLayer::getBrowserFileName() const
{
    return browserFileBtn->text();
}

void LibraryBooksFilterLayer::setStoreBooksBtnEnabled(bool enabled)
{
    storeBooksBtn->setEnabled(enabled);
}
