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

#include "ViewerAnnotationActions.h"

#include <QWidget>
#include <QFile>
#include <QPainter>
#include <QDebug>
#include "QBook.h"
#include "BookLocation.h"

ViewerAnnotationActions::ViewerAnnotationActions( QWidget* parent ) :
    QWidget(parent)
{
    setupUi(this);

    connect(deleteActionBtn,  SIGNAL(clicked()), this, SLOT(deleteMark()));
    connect(editActionBtn,    SIGNAL(clicked()), this, SLOT(editMark()));
    connect(gotoActionBtn,    SIGNAL(clicked()), this, SLOT(goToMark()));
    arrowCont->hide();
    arrowCont2->hide();
}

ViewerAnnotationActions::~ViewerAnnotationActions()
{}

void ViewerAnnotationActions::setBookmark(BookLocation* location)
{
    m_location = location;
    editActionBtn->hide();
}

void ViewerAnnotationActions::setNote(BookLocation* location)
{
    m_location = location;
    editActionBtn->show();
}

void ViewerAnnotationActions::deleteMark()
{
    emit deleteMark(m_location);
}

void ViewerAnnotationActions::editMark()
{
    emit editMark(m_location);
}

void ViewerAnnotationActions::goToMark()
{
    emit goToMark(m_location);
}

int ViewerAnnotationActions::getPopUpWidth()
{
    if(m_location->type != BookLocation::BOOKMARK)
        return deleteActionBtn->width() + gotoActionBtn->width() + editActionBtn->width();
    else
        return deleteActionBtn->width() + gotoActionBtn->width();
}

int ViewerAnnotationActions::getPopUpHeight()
{
    return popupCont->height();
}

void ViewerAnnotationActions::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
