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

#ifndef BRIGHTNESSSLIDERITEM_H
#define BRIGHTNESSSLIDERITEM_H

#include <QWidget>

// Predeclarations
class Slider;

class BrightnessSliderItem : public QWidget
{
    Q_OBJECT

public:
    BrightnessSliderItem(QWidget*);
    virtual ~BrightnessSliderItem();

    void                                on                                  ();
    void                                off                                 ();
    void                                disableOn                           ();
    void                                disableOff                          ();

protected:
    virtual void                        paintEvent                          (QPaintEvent *event);

private:
    Slider*                             m_parent;
    int                                 m_id;
};

#endif // BRIGHTNESSSLIDERITEM_H
