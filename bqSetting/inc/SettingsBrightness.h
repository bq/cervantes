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

#ifndef SETTINGSBRIGHTNESS_H
#define SETTINGSBRIGHTNESS_H

#include "FullScreenWidget.h"
#include "ui_SettingsBrightness.h"

class SettingsBrightness : public FullScreenWidget , protected Ui::SettingsBrightness{

    Q_OBJECT

public:
                        SettingsBrightness                  (QWidget *);
    virtual             ~SettingsBrightness                 ();


private:



protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void        paintEvent                          (QPaintEvent *);
    virtual void        showEvent                           (QShowEvent*);

private slots:

    void                handleScreenLightStatusBtn          ();
    void                handleBrightnessDecreasePressed     ();
    void                handleBrightnessDecreaseLongPressed ();
    void                handleBrightnessIncreasePressed     ();
    void                handleBrightnessIncreaseLongPressed ();

public slots:

    void                paintLight                          (bool /*on*/);

};


#endif // SETTINGSBRIGHTNESS_H
