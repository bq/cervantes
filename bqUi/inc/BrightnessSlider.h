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

#ifndef BRIGHTNESSSLIDER_H
#define BRIGHTNESSSLIDER_H

#include <QWidget>
#include <QList>

// Predeclarations
class BrightnessSliderItem;
class PowerManagerLock;

class BrightnessSlider : public QWidget
{
    Q_OBJECT

public:
    BrightnessSlider(QWidget* parent);
    virtual ~BrightnessSlider();

    int                                 registerItem                        ( BrightnessSliderItem* item );

    void                                increaseItemSelected                ();
    void                                decreaseItemSelected                ();

    void                                selectMaxItem                       ();
    void                                selectMinItem                       ();

    void                                switchOn                            ();
    void                                switchOff                           ();

protected:
    virtual void                        paintEvent                          ( QPaintEvent* event );

    virtual void                        mouseReleaseEvent                   ( QMouseEvent* );
    virtual void                        mouseMoveEvent                      ( QMouseEvent* );

    void                                itemSelected                        ( int itemId );

    void                                recalculateLastItem                 ();

private:
    QList<BrightnessSliderItem*>        m_registeredItems;
    int                                 i_brightnessValue;
    int                                 m_lastItemSelected;
    int                                 m_registeredItemsSizeInv;
    float                               m_inc;
    int                                 m_lastX;

    bool                                m_on;
};

#endif // BRIGHTNESSSLIDER_H
