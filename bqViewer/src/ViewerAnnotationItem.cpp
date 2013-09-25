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

#include "ViewerAnnotationItem.h"

#include "BookLocation.h"
#include "bqUtils.h"

#include <QDebug>
#include <QWidget>
#include <QDateTime>
#include <QPainter>

#define MYNOTEITEM_MAX_LENGTH 40

ViewerAnnotationItem::ViewerAnnotationItem(QWidget *parentViewer) :
    GestureWidget(parentViewer)
{
    qDebug() << Q_FUNC_INFO;
    connect(this, SIGNAL(tap()), this, SLOT(handleTap()));
    connect(this, SIGNAL(longPressStart()), this, SLOT(handleLongPress()));
    setupUi(this);
}

ViewerAnnotationItem::~ViewerAnnotationItem()
{
    qDebug() << Q_FUNC_INFO;
}

void ViewerAnnotationItem::handleTap()
{
    qDebug() << Q_FUNC_INFO;
    emit handleTap(s_markupRef);
}

void ViewerAnnotationItem::handleLongPress()
{
    qDebug() << Q_FUNC_INFO;
    emit longPress(m_location);
}

void ViewerAnnotationItem::setItem(const BookLocation* location)
{
    qDebug() << Q_FUNC_INFO << location->text;
    m_location = const_cast<BookLocation*>(location);
    s_markupRef = location->bookmark;
    long long unixTime = (location->lastUpdated / 1000);
    QDateTime timestamp;
    timestamp.setTime_t(unixTime);
    annotationDateLbl->setText(timestamp.toString("dd/MM/yy"));

    annotationPageLbl->setText(tr("Pag. %1").arg(QString::number(location->page)));
    switch(location->type)
    {
        case BookLocation::BOOKMARK:
            typeAnnotationLbl->setStyleSheet("background-image:url(:/res/bookmark_ico.png)");
            typeAnnotationLbl->show();
            annotationTextLbl->hide();
            noteTextLbl->hide();
            break;
        case BookLocation::NOTE:
            typeAnnotationLbl->setStyleSheet("background-image:url(:/res/note_ico.png)");
            typeAnnotationLbl->show();
            annotationTextLbl->setText(bqUtils::truncateStringToLength(location->text, MYNOTEITEM_MAX_LENGTH));
            annotationTextLbl->show();
            noteTextLbl->setText(bqUtils::truncateStringToLength(location->note, MYNOTEITEM_MAX_LENGTH));
            noteTextLbl->show();
            break;
        case BookLocation::HIGHLIGHT:
            typeAnnotationLbl->setStyleSheet("background-image:url(:/res/highlighted_ico.png)");
            typeAnnotationLbl->show();
            annotationTextLbl->setText(bqUtils::truncateStringToLength(location->text, MYNOTEITEM_MAX_LENGTH));
            annotationTextLbl->show();
            noteTextLbl->hide();
            break;
        default:
            typeAnnotationLbl->hide();
            annotationTextLbl->hide();
            noteTextLbl->hide();
            break;
    }
}

void ViewerAnnotationItem::paintEvent (QPaintEvent *)
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
