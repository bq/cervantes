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

#include "LibrarySortBooksByLayer.h"

#include <QButtonGroup>
#include <QFile>
#include <QPainter>
#include <QDebug>
#include "Screen.h"

#define CHECKED "background-image: url(':/res/check.png')"
#define UNCHECKED "background-image:none;"
#define ENABLED "color:#000000;"
#define DISABLED "color:#606060;"

LibrarySortBooksByLayer::LibrarySortBooksByLayer(QWidget* parent) :
        QWidget(parent)
{
    setupUi(this);

    connect(recentSortBtn,  SIGNAL(clicked()), this, SLOT(recentClicked()));
    connect(titleSortBtn,   SIGNAL(clicked()), this, SLOT(titleClicked()));
    connect(authorSortBtn,  SIGNAL(clicked()), this, SLOT(authorClicked()));
    connect(indexSortBtn,  SIGNAL(clicked()), this, SLOT(indexClicked()));

    recentSortBtn->setStyleSheet(CHECKED);
    recentSortBtn->setStyleSheet(ENABLED);
    titleSortBtn->setStyleSheet(ENABLED);
    authorSortBtn->setStyleSheet(ENABLED);
    indexSortBtn->setStyleSheet(ENABLED);
}

LibrarySortBooksByLayer::~LibrarySortBooksByLayer()
{}

void LibrarySortBooksByLayer::paintEvent( QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void LibrarySortBooksByLayer::recentClicked ()
{
    Screen::getInstance()->queueUpdates();
    recentSortBtn->setStyleSheet(CHECKED);
    indexSortBtn->setStyleSheet(UNCHECKED);
    titleSortBtn->setStyleSheet(UNCHECKED);
    authorSortBtn->setStyleSheet(UNCHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
    emit recentSelected();
}

void LibrarySortBooksByLayer::titleClicked ()
{
    Screen::getInstance()->queueUpdates();
    recentSortBtn->setStyleSheet(UNCHECKED);
    titleSortBtn->setStyleSheet(CHECKED);
    indexSortBtn->setStyleSheet(UNCHECKED);
    authorSortBtn->setStyleSheet(UNCHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit titleSelected();
}

void LibrarySortBooksByLayer::authorClicked ()
{
    Screen::getInstance()->queueUpdates();
    recentSortBtn->setStyleSheet(UNCHECKED);
    titleSortBtn->setStyleSheet(UNCHECKED);
    indexSortBtn->setStyleSheet(UNCHECKED);
    authorSortBtn->setStyleSheet(CHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit authorSelected();
}

void LibrarySortBooksByLayer::indexClicked ()
{
    Screen::getInstance()->queueUpdates();
    indexSortBtn->setStyleSheet(CHECKED);
    recentSortBtn->setStyleSheet(UNCHECKED);
    titleSortBtn->setStyleSheet(UNCHECKED);
    authorSortBtn->setStyleSheet(UNCHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit indexSelected();
}

void LibrarySortBooksByLayer::enableRecent(bool enable)
{
    if(enable)
        recentSortBtn->show();

    else
        recentSortBtn->hide();

    recentSortBtn->setEnabled(enable);

}

void LibrarySortBooksByLayer::enableIndex(bool enable)
{
    if(enable)
        indexSortBtn->show();

    else
        indexSortBtn->hide();

    indexSortBtn->setEnabled(enable);

}

void LibrarySortBooksByLayer::setRecentChecked()
{
    recentSortBtn->setStyleSheet(CHECKED);
    titleSortBtn->setStyleSheet(UNCHECKED);
    indexSortBtn->setStyleSheet(UNCHECKED);
    authorSortBtn->setStyleSheet(UNCHECKED);
}

void LibrarySortBooksByLayer::setTitleChecked()
{
    recentSortBtn->setStyleSheet(UNCHECKED);    
    titleSortBtn->setStyleSheet(CHECKED);
    indexSortBtn->setStyleSheet(UNCHECKED);
    authorSortBtn->setStyleSheet(UNCHECKED);
}

void LibrarySortBooksByLayer::setAuthorChecked()
{
    recentSortBtn->setStyleSheet(UNCHECKED);
    titleSortBtn->setStyleSheet(UNCHECKED);
    indexSortBtn->setStyleSheet(UNCHECKED);
    authorSortBtn->setStyleSheet(CHECKED);
}

void LibrarySortBooksByLayer::setIndexChecked()
{
    recentSortBtn->setStyleSheet(UNCHECKED);
    titleSortBtn->setStyleSheet(UNCHECKED);
    indexSortBtn->setStyleSheet(CHECKED);
    authorSortBtn->setStyleSheet(UNCHECKED);
}

QString LibrarySortBooksByLayer::getRecentSortName() const
{
    return recentSortBtn->text();
}

QString LibrarySortBooksByLayer::getTitleSortName() const
{
    return titleSortBtn->text();
}

QString LibrarySortBooksByLayer::getAuthorSortName() const
{
    return authorSortBtn->text();
}

QString LibrarySortBooksByLayer::getIndexSortName() const
{
    return indexSortBtn->text();
}
