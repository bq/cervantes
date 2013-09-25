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

#include "ViewerMenu.h"
#include "Viewer.h"
#include "QDocView.h"
#include "bqUtils.h"

#include <QWidget>
#include <QDebug>
#include <QKeyEvent>

#define TITLE_STRING_MAX_LENGTH 36

ViewerMenu::ViewerMenu(QWidget *parent): QWidget(parent)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);
    connect(appaeranceBtn,  SIGNAL(clicked()),  SIGNAL(fontBtnPress()));
    connect(searchBtn,      SIGNAL(clicked()),  SIGNAL(searchReq()));
    connect(gotoPageBtn,    SIGNAL(clicked()),  SIGNAL(goToPageBtnPress()));
    connect(summaryBtn,     SIGNAL(clicked()),  SIGNAL(summaryReq()));
    connect(contentsBtn,    SIGNAL(clicked()),  SIGNAL(indexReq()));
    hide();
    chapterCont->hide();
    chapterNameLbl->hide();
    chapterPageLbl->hide();
}

ViewerMenu::~ViewerMenu()
{
    /// Do nothing
    qDebug() << Q_FUNC_INFO;
}

void ViewerMenu::setCurrentChapter(const QString& title, int pagesLeft)
{
    chapterCont->show();
    chapterNameLbl->setText(bqUtils::truncateStringToLength(title, TITLE_STRING_MAX_LENGTH));
    chapterNameLbl->show();
    if(pagesLeft == 1)
        chapterPageLbl->setText(tr("%1 pág restante").arg(pagesLeft));
    else
        chapterPageLbl->setText(tr("%1 pág restantes").arg(pagesLeft));
    chapterPageLbl->show();
}

void ViewerMenu::hideBar()
{
    chapterCont->hide();
}

