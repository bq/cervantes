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

#include "BrightnessSlider.h"

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

BrightnessSlider::BrightnessSlider(QWidget* parent) :
    Slider(parent)
{
    qDebug() << Q_FUNC_INFO;
}

BrightnessSlider::~BrightnessSlider()
{
    qDebug() << Q_FUNC_INFO;
}

void BrightnessSlider::applyChanges(int itemId)
{
    qDebug() << Q_FUNC_INFO << m_lastItemSelected << itemId << m_on;

    if(itemId == 0)
    {
        FrontLight::getInstance()->setBrightness(2);
        qDebug() << Q_FUNC_INFO << "LightValue: " << 2;
    }
    else
    {
        int brightnessVal = (itemId+1)*m_registeredItemsSizeInv;// NOTE: Items ids go from 0 to total-1
        FrontLight::getInstance()->setBrightness(brightnessVal);
        qDebug() << Q_FUNC_INFO << "LightValue: " << brightnessVal;
    }
}

void BrightnessSlider::recalculateLastItem()
{
    int totalItems = m_registeredItems.size();
    int brightnessVal = FrontLight::getInstance()->getBrightness();
    m_inc = ((float)this->width()) / totalItems;
    m_lastItemSelected = qBound(0, (brightnessVal*totalItems/100)-1, totalItems-1);
    qDebug() << Q_FUNC_INFO << this->x() << this->width() << m_inc << m_lastItemSelected;
}
