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

#include "BrightnessSliderItem.h"

#include "BrightnessSlider.h"
#include "Screen.h"

#include <QPainter>
#include <QStyleOption>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QDebug>

BrightnessSliderItem::BrightnessSliderItem(QWidget *parent) :
    QWidget(parent)
{
    m_parent = (BrightnessSlider*)parent;
    m_id = m_parent->registerItem(this);
}

BrightnessSliderItem::~BrightnessSliderItem()
{
    qDebug() << Q_FUNC_INFO;
}

void BrightnessSliderItem::paintEvent( QPaintEvent* event )
{
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void BrightnessSliderItem::on()
{
    setStyleSheet("background-image:url(:/res/progress_on.png);");
}

void BrightnessSliderItem::off()
{
    setStyleSheet("background-image:url(:/res/progress_off.png);");
}

void BrightnessSliderItem::disableOn()
{
    setStyleSheet("background-image:url(:/res/progress_on_disabled.png);");
}

void BrightnessSliderItem::disableOff()
{
    setStyleSheet("background-image:url(:/res/progress_off_disabled.png);");
}


