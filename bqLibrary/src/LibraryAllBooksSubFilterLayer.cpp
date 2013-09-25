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

#include "LibraryAllBooksSubFilterLayer.h"

#include <QButtonGroup>
#include <QFile>
#include <QPainter>
#include <QDebug>
#include <QStringList>
#include "Screen.h"
#include "Library.h"

#define CHECKED "background-image: url(':/res/check.png')"
#define UNCHECKED "background-image:none"
#define ENABLED "color:#000000"
#define DISABLED "color:#606060"



LibraryAllBooksSubFilterLayer::LibraryAllBooksSubFilterLayer(QWidget* parent) :
        QWidget(parent)
{
    setupUi(this);

    connect(allBooksBtn,        SIGNAL(clicked()), this, SLOT(allBooksClicked()));
    connect(allActiveBooksBtn,  SIGNAL(clicked()), this, SLOT(allActiveBooksClicked()));
    connect(newBooksBtn,        SIGNAL(clicked()), this, SLOT(newBooksClicked()));
    connect(readBooksBtn,       SIGNAL(clicked()), this, SLOT(alreadyReadClicked()));

    m_allBooksName = allBooksBtn->text();
    m_allActiveBooksName = allActiveBooksBtn->text();
    m_newBooksName = newBooksBtn->text();
    m_readBooksName = readBooksBtn->text();
    allBooksBtn->setStyleSheet(CHECKED);
}

LibraryAllBooksSubFilterLayer::~LibraryAllBooksSubFilterLayer()
{}

void LibraryAllBooksSubFilterLayer::paintEvent( QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void LibraryAllBooksSubFilterLayer::allBooksClicked ()
{
    Screen::getInstance()->queueUpdates();
    newBooksBtn->setStyleSheet(Library::modifyStyleSheet(newBooksBtn->styleSheet(), UNCHECKED));
    allActiveBooksBtn->setStyleSheet(Library::modifyStyleSheet(allActiveBooksBtn->styleSheet(), UNCHECKED));
    readBooksBtn->setStyleSheet(Library::modifyStyleSheet(readBooksBtn->styleSheet(), UNCHECKED));
    allBooksBtn->setStyleSheet(CHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit allBooksSelected();
}

void LibraryAllBooksSubFilterLayer::allActiveBooksClicked ()
{
    Screen::getInstance()->queueUpdates();
    allBooksBtn->setStyleSheet(UNCHECKED);
    newBooksBtn->setStyleSheet(Library::modifyStyleSheet(newBooksBtn->styleSheet(), UNCHECKED));
    readBooksBtn->setStyleSheet(Library::modifyStyleSheet(readBooksBtn->styleSheet(), UNCHECKED));
    allActiveBooksBtn->setStyleSheet(CHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit allActiveBooksSelected();
}

void LibraryAllBooksSubFilterLayer::newBooksClicked ()
{
    Screen::getInstance()->queueUpdates();
    allBooksBtn->setStyleSheet(UNCHECKED);
    allActiveBooksBtn->setStyleSheet(Library::modifyStyleSheet(allActiveBooksBtn->styleSheet(), UNCHECKED));
    readBooksBtn->setStyleSheet(Library::modifyStyleSheet(readBooksBtn->styleSheet(), UNCHECKED));
    newBooksBtn->setStyleSheet(CHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit allNewBooksSelected();
}

void LibraryAllBooksSubFilterLayer::alreadyReadClicked()
{
    Screen::getInstance()->queueUpdates();
    allBooksBtn->setStyleSheet(UNCHECKED);
    newBooksBtn->setStyleSheet(Library::modifyStyleSheet(newBooksBtn->styleSheet(), UNCHECKED));
    allActiveBooksBtn->setStyleSheet(Library::modifyStyleSheet(allActiveBooksBtn->styleSheet(), UNCHECKED));
    readBooksBtn->setStyleSheet(CHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit readBooksSelected();
}

void LibraryAllBooksSubFilterLayer::setAllBooksNumber( int size )
{
    allBooksBtn->setText(m_allBooksName + " ("+ QString::number(size) + ")");
    if(size <= 0) {
        allBooksBtn->setEnabled(false);
        allBooksBtn->setStyleSheet(DISABLED);
    }
    else {
        allBooksBtn->setEnabled(true);
        allBooksBtn->setStyleSheet(Library::modifyStyleSheet(allBooksBtn->styleSheet(), ENABLED));
    }
}

void LibraryAllBooksSubFilterLayer::setAllActiveBooksNumber( int size )
{
    allActiveBooksBtn->setText(m_allActiveBooksName + " ("+ QString::number(size) + ")");
    if(size <= 0) {
        allActiveBooksBtn->setEnabled(false);
        allActiveBooksBtn->setStyleSheet(DISABLED);
    }
    else {
        allActiveBooksBtn->setEnabled(true);
        allActiveBooksBtn->setStyleSheet(Library::modifyStyleSheet(allActiveBooksBtn->styleSheet(), ENABLED));
    }
}

void LibraryAllBooksSubFilterLayer::setNewBooksNumber( int size )
{
    newBooksBtn->setText(m_newBooksName + " ("+ QString::number(size) + ")");
    if(size <= 0) {
        newBooksBtn->setEnabled(false);
        newBooksBtn->setStyleSheet(DISABLED);
    }
    else {
        newBooksBtn->setEnabled(true);
        newBooksBtn->setStyleSheet(Library::modifyStyleSheet(newBooksBtn->styleSheet(), ENABLED));
    }
}

void LibraryAllBooksSubFilterLayer::setReadBooksNumber( int size )
{
    readBooksBtn->setText(m_readBooksName + " ("+ QString::number(size) + ")");
    if(size <= 0) {
        readBooksBtn->setEnabled(false);
        readBooksBtn->setStyleSheet(DISABLED);
    }
    else {
        readBooksBtn->setEnabled(true);
        readBooksBtn->setStyleSheet(Library::modifyStyleSheet(readBooksBtn->styleSheet(), ENABLED));
    }
}

QString LibraryAllBooksSubFilterLayer::getAllBooksName() const
{
    return allBooksBtn->text();
}
QString LibraryAllBooksSubFilterLayer::getAllActiveBooksName() const
{
    return allActiveBooksBtn->text();
}

QString LibraryAllBooksSubFilterLayer::getNewBooksName() const
{
    return newBooksBtn->text();
}

QString LibraryAllBooksSubFilterLayer::getReadBooksName() const
{
    return readBooksBtn->text();
}

void LibraryAllBooksSubFilterLayer::setAllBooksChecked()
{
    allBooksBtn->setStyleSheet(CHECKED);
    allActiveBooksBtn->setStyleSheet(Library::modifyStyleSheet(allActiveBooksBtn->styleSheet(), UNCHECKED));
    newBooksBtn->setStyleSheet(Library::modifyStyleSheet(newBooksBtn->styleSheet(), UNCHECKED));
}

void LibraryAllBooksSubFilterLayer::setAllActiveBooksChecked()
{
    allBooksBtn->setStyleSheet(Library::modifyStyleSheet(newBooksBtn->styleSheet(), UNCHECKED));
    allActiveBooksBtn->setStyleSheet(CHECKED);
    newBooksBtn->setStyleSheet(Library::modifyStyleSheet(newBooksBtn->styleSheet(), UNCHECKED));
}
