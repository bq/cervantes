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

#include "ViewerBookListActions.h"
#include "BookInfo.h"

#include <QWidget>
#include <QDebug>
#include <QPainter>
#include <QButtonGroup>

ViewerBookListActions::ViewerBookListActions(QWidget *parent) :
    QWidget(parent),g_readButtons(new QButtonGroup(this))
{
    setupUi(this);
    g_readButtons->addButton(noReadBtn,     BookInfo::NO_READ_BOOK);
    g_readButtons->addButton(readingBtn,    BookInfo::READING_BOOK);
    g_readButtons->addButton(readBtn,       BookInfo::READ_BOOK);

    connect(g_readButtons,      SIGNAL(buttonPressed(int)), this, SIGNAL(readStateChanged(int)));

}

ViewerBookListActions::~ViewerBookListActions()
{
    qDebug() << Q_FUNC_INFO;
}

void ViewerBookListActions::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ViewerBookListActions::setButtonsState( BookInfo::readStateEnum state)
{
    switch(state)
    {
    case BookInfo::NO_READ_BOOK:
        noReadBtn->hide();
        readingBtn->show();
        readBtn->show();
    break;
    case BookInfo::READING_BOOK:
        noReadBtn->show();
        readingBtn->hide();
        readBtn->show();
    break;
    case BookInfo::READ_BOOK:
        noReadBtn->show();
        readingBtn->show();
        readBtn->hide();
    break;
    }
}
