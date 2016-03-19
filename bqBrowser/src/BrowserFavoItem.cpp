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

#include "BrowserFavoItem.h"

#include <QPainter>
#include <QKeyEvent>
#include <QDebug>

BrowserFavoItem::BrowserFavoItem(QWidget *parent)
        : GestureWidget(parent)
        , m_checked(false)
{
    setupUi(this);
    checkBtn->setChecked(false);
    connect(this, SIGNAL(tap()), this, SLOT(handleTap()));
    connect(checkBtn, SIGNAL(clicked()),this, SLOT(handleCheckBtn()));
}

BrowserFavoItem::~BrowserFavoItem()
{
    qDebug() << Q_FUNC_INFO;
}

void BrowserFavoItem::paintEvent (QPaintEvent *)
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }

void BrowserFavoItem::setItem( const QString& eventLabel, const QString& eventurl)
{
    m_url = eventurl;
    EventLabel->setText(eventLabel);
    EventUrl->setText(eventurl);
    EventLabel->show();
    EventUrl->show();
    checkBtn->show();
    setChecked(false);
    qDebug() << Q_FUNC_INFO << EventUrl->text();
}

void BrowserFavoItem::hideItem()
{
    m_url = "";
    EventLabel->setText("");
    EventUrl->setText("");
    EventLabel->hide();
    EventUrl->hide();
    checkBtn->hide();
    setChecked(false);
}

void BrowserFavoItem::setChecked(bool checked)
{
    m_checked = checked;
    checkBtn->setChecked(checked);
}

void BrowserFavoItem::handleTap()
{
    qDebug() << Q_FUNC_INFO << EventUrl->text();
    emit handleTap(m_url);
}

void BrowserFavoItem::handleCheckBtn()
{
    qDebug() << Q_FUNC_INFO;
    setChecked(!m_checked);
    emit select();
}

bool BrowserFavoItem::isChecked()
{
    return m_checked;
}
