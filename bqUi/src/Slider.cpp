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

#include "Slider.h"

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

Slider::Slider(QWidget* parent) :
    QWidget(parent)
  , m_lastItemSelected(0)
  , m_registeredItemsSizeInv(0)
  , m_inc(0.0)
  , m_lastX(0)
  , m_on(false)
{
    qDebug() << Q_FUNC_INFO;
}

Slider::~Slider()
{
    qDebug() << Q_FUNC_INFO;
}

void Slider::paintEvent( QPaintEvent* event )
{
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void Slider::mouseReleaseEvent(QMouseEvent* event)
{
    //qDebug() << Q_FUNC_INFO << event->x() << this->x() << this->width() << m_inc;

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

void Slider::mouseMoveEvent(QMouseEvent* event)
{
    //qDebug() << Q_FUNC_INFO << event->x() << this->x() << this->width() << m_inc;

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

int Slider::registerItem(BrightnessSliderItem* item)
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

void Slider::increaseItemSelected()
{
    qDebug() << Q_FUNC_INFO;

    if(m_lastItemSelected < m_registeredItems.size()-1)
        itemSelected(m_lastItemSelected+1);
}

void Slider::decreaseItemSelected()
{
    qDebug() << Q_FUNC_INFO;

    if(m_lastItemSelected > 0)
        itemSelected(m_lastItemSelected-1);
}

void Slider::selectMaxItem()
{
    qDebug() << Q_FUNC_INFO;

    if(m_lastItemSelected < m_registeredItems.size()-1)
        itemSelected(m_registeredItems.size()-1);
}

void Slider::selectMinItem()
{
    qDebug() << Q_FUNC_INFO;

    if(m_lastItemSelected > 0)
        itemSelected(0);
}

void Slider::switchOff()
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

void Slider::switchOn()
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

void Slider::itemSelected(int itemId)
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

    applyChanges(itemId);
}
