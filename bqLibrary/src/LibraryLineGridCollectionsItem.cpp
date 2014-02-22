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

#include "LibraryLineGridCollectionsItem.h"

#include "BookInfo.h"
#include "bqUtils.h"

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QPainter>
#include <QDebug>


LibraryLineGridCollectionsItem::LibraryLineGridCollectionsItem(QWidget* parent) : LibraryGridViewerItem(parent)
{
    setupUi(this);

    connect(deleteCollectionBtn, SIGNAL(clicked()), this, SLOT(deleteCollection()));
    connect(editCollectionBtn, SIGNAL(clicked()), this, SLOT(editCollection()));
}

LibraryLineGridCollectionsItem::~LibraryLineGridCollectionsItem()
{}

void LibraryLineGridCollectionsItem::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QFontMetrics fontMetrics = p.fontMetrics();
    QString text = fontMetrics.elidedText(m_path, Qt::ElideRight, collectionTitleLabel->width() - 5);
    p.drawText(10, collectionTitleLabel->y(), collectionTitleLabel->width() - 5, collectionTitleLabel->height(), Qt::AlignVCenter, text);
}


void LibraryLineGridCollectionsItem::setCollection( const QString &collectionName, const int booksNumber )
{
    qDebug() << Q_FUNC_INFO;
    m_path = collectionName;
    hide();
    if(collectionName != "")
    {
        collectionTitleLabel->setText("");
        collectionTitleLabel->show();
        booksNumberValueLbl->setText(tr("%1").arg(booksNumber));
        booksNumberValueLbl->show();

        if(booksNumber == 1)
            booksNumberLlb->setText(tr("Book"));
        else
            booksNumberLlb->setText(tr("Books"));

        booksNumberLlb->show();
        deleteCollectionBtn->show();
        editCollectionBtn->show();
    }
    else
    {
        collectionTitleLabel->hide();
        booksNumberValueLbl->hide();
        booksNumberLlb->hide();
        deleteCollectionBtn->hide();
        editCollectionBtn->hide();
    }
    show();
}

void LibraryLineGridCollectionsItem::deleteCollection()
{
    qDebug() << Q_FUNC_INFO;
    if (m_path != "")
        emit deleteCollection(m_path);
}

void LibraryLineGridCollectionsItem::editCollection()
{
    qDebug() << Q_FUNC_INFO;
    if (m_path != "")
        emit editCollection(m_path);
}
