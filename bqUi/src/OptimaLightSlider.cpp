/*************************************************************************

bq Cervantes e-book reader application
Copyright (C) 2011-2017  Mundoreader, S.L

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

#include "OptimaLightSlider.h"

#include "BrightnessSliderItem.h"
#include "FrontLight.h"
#include "QBook.h"
#include "Screen.h"
#include "PowerManager.h"
#include "PowerManagerLock.h"

#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <QMouseEvent>

OptimaLightSlider::OptimaLightSlider(QWidget* parent) :
    Slider(parent)
{
    qDebug() << Q_FUNC_INFO;
}

OptimaLightSlider::~OptimaLightSlider()
{
    qDebug() << Q_FUNC_INFO;
}

void OptimaLightSlider::applyChanges(int itemId)
{
    qDebug() << Q_FUNC_INFO << m_lastItemSelected << itemId << m_on;

    FrontLight::getInstance()->setOptimaLightValue(itemId);
}

void OptimaLightSlider::recalculateLastItem()
{
    int totalItems = m_registeredItems.size();
    int currentItem = FrontLight::getInstance()->getOptimaLightValue();
    m_inc = ((float)this->width()) / totalItems;
    m_lastItemSelected = qBound(0, currentItem, totalItems-1);
    qDebug() << Q_FUNC_INFO << this->x() << this->width() << m_inc << m_lastItemSelected;
}
