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
    QWidget(parent)
  , m_lastItemSelected(0)
  , m_registeredItemsSizeInv(0)
  , m_inc(0.0)
  , m_lastX(0)
  , m_on(false)
{
    qDebug() << Q_FUNC_INFO;
}

BrightnessSlider::~BrightnessSlider()
{
    qDebug() << Q_FUNC_INFO;
}

void BrightnessSlider::paintEvent( QPaintEvent* event )
{
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void BrightnessSlider::mouseReleaseEvent(QMouseEvent* event)
{
//    qDebug() << Q_FUNC_INFO << event->x() << this->x() << this->width() << m_inc;

    if(!m_on)
        return;

    int x = qBound(0, event->x(), this->width()-1);

    if(qAbs(x-m_lastX) <= m_inc)
    {
        event->accept();
        return;
    }
    m_lastX = x;

    int item = x / m_inc;
    itemSelected(item);

    event->accept();
}

void BrightnessSlider::mouseMoveEvent(QMouseEvent* event)
{
//    qDebug() << Q_FUNC_INFO << event->x() << this->x() << this->width() << m_inc;

    if(!m_on)
        return;

    int x = qBound(0, event->x(), this->width()-1);

    if(qAbs(x-m_lastX) <= m_inc)
    {
        event->accept();
        return;
    }
    m_lastX = x;

    int item = x / m_inc;
    itemSelected(item);

    event->accept();
}

int BrightnessSlider::registerItem(BrightnessSliderItem* item)
{
    qDebug() << Q_FUNC_INFO << item;

    if(!m_registeredItems.contains(item))
    {
        m_registeredItems.append(item);
        m_registeredItemsSizeInv = 100/m_registeredItems.size();
        return m_registeredItems.size()-1;
    }
    return m_registeredItems.indexOf(item);
}

void BrightnessSlider::itemSelected(int itemId)
{
    qDebug() << Q_FUNC_INFO << m_lastItemSelected << itemId << m_on;

    if(!m_on)
        return;

    Screen::getInstance()->queueUpdates();
    Screen::getInstance()->setMode(Screen::MODE_QUICK,true,FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);

    for(int i = m_lastItemSelected; i <= itemId; ++i)
        m_registeredItems.at(i)->on();

    for(int i = m_lastItemSelected; i > itemId; --i)
        m_registeredItems.at(i)->off();

    Screen::getInstance()->flushUpdates();

    m_lastItemSelected = itemId;
    m_lastX = itemId*m_inc;

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

void BrightnessSlider::increaseItemSelected()
{
    qDebug() << Q_FUNC_INFO;

    if(m_lastItemSelected < m_registeredItems.size()-1)
        itemSelected(m_lastItemSelected+1);
}

void BrightnessSlider::decreaseItemSelected()
{
    qDebug() << Q_FUNC_INFO;

    if(m_lastItemSelected > 0)
        itemSelected(m_lastItemSelected-1);
}

void BrightnessSlider::selectMaxItem()
{
    qDebug() << Q_FUNC_INFO;

    if(m_lastItemSelected < m_registeredItems.size()-1)
        itemSelected(m_registeredItems.size()-1);
}

void BrightnessSlider::selectMinItem()
{
    qDebug() << Q_FUNC_INFO;

    if(m_lastItemSelected > 0)
        itemSelected(0);
}

void BrightnessSlider::switchOff()
{
    qDebug() << Q_FUNC_INFO;

    m_on = false;

    Screen::getInstance()->queueUpdates();
    Screen::getInstance()->setMode(Screen::MODE_QUICK,true,FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);

    recalculateLastItem();

    // Switch off everything
    for(int i = 0; i <= m_lastItemSelected; ++i)
        m_registeredItems.at(i)->disableOn();

    int size = m_registeredItems.size();
    for(int i = m_lastItemSelected+1; i < size; ++i)
        m_registeredItems.at(i)->disableOff();

    Screen::getInstance()->flushUpdates();
}

void BrightnessSlider::switchOn()
{
    qDebug() << Q_FUNC_INFO;

    m_on = true;

    Screen::getInstance()->queueUpdates();
    Screen::getInstance()->setMode(Screen::MODE_QUICK,true,FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);

    // We are going to paint on from 0 to the new last item selected.
    // And then paint off from this new value to the old last item
    recalculateLastItem();

    for(int i = 0; i <= m_lastItemSelected; ++i)
        m_registeredItems.at(i)->on();

    int size = m_registeredItems.size();
    for(int i = m_lastItemSelected+1; i < size; ++i)
        m_registeredItems.at(i)->off();

    Screen::getInstance()->flushUpdates();
}

void BrightnessSlider::recalculateLastItem()
{
    int totalItems = m_registeredItems.size();
    int brightnessVal = FrontLight::getInstance()->getBrightness();
    m_inc = ((float)this->width()) / totalItems;
    m_lastItemSelected = qBound(0, (brightnessVal*totalItems/100)-1, totalItems-1);
    qDebug() << Q_FUNC_INFO << this->x() << this->width() << m_inc << m_lastItemSelected;
}
