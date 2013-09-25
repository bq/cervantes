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

#include "LibrarySortBrowserBooksByLayer.h"

#include <QButtonGroup>
#include <QFile>
#include <QPainter>
#include <QDebug>
#include "Screen.h"

#define CHECKED "background-image: url(':/res/check.png')"
#define UNCHECKED "background-image:none;"

LibrarySortBrowserBooksByLayer::LibrarySortBrowserBooksByLayer(QWidget* parent) :
        QWidget(parent)
{
    setupUi(this);

    connect(titleSortBtn,  SIGNAL(clicked()), this, SLOT(nameClicked()));
    connect(modDateSortBtn,   SIGNAL(clicked()), this, SLOT(dateClicked()));

    titleSortBtn->setStyleSheet(CHECKED);
}

LibrarySortBrowserBooksByLayer::~LibrarySortBrowserBooksByLayer()
{}

void LibrarySortBrowserBooksByLayer::paintEvent( QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void LibrarySortBrowserBooksByLayer::nameClicked ()
{
    Screen::getInstance()->queueUpdates();
    titleSortBtn->setStyleSheet(CHECKED);
    modDateSortBtn->setStyleSheet(UNCHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
    emit nameSelected();
}

void LibrarySortBrowserBooksByLayer::dateClicked ()
{
    Screen::getInstance()->queueUpdates();
    titleSortBtn->setStyleSheet(UNCHECKED);
    modDateSortBtn->setStyleSheet(CHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit dateSelected();
}

void LibrarySortBrowserBooksByLayer::setNameChecked()
{
    titleSortBtn->setStyleSheet(CHECKED);
    modDateSortBtn->setStyleSheet(UNCHECKED);
}

void LibrarySortBrowserBooksByLayer::setDateChecked()
{
    titleSortBtn->setStyleSheet(UNCHECKED);
    modDateSortBtn->setStyleSheet(CHECKED);
}

QString LibrarySortBrowserBooksByLayer::getNameSortName() const
{
    return titleSortBtn->text();
}

QString LibrarySortBrowserBooksByLayer::getDateSortName() const
{
    return modDateSortBtn->text();
}


