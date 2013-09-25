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

#ifndef SETTINGSQUICKSETTINGSPOPUP_H
#define SETTINGSQUICKSETTINGSPOPUP_H

#include <QWidget>
#include "ui_SettingsQuickSettingsPopup.h"

// Predeclarations
class PowerManagerLock;
class QShowEvent;
class QHideEvent;

class SettingsQuickSettingsPopup : public QWidget, protected Ui::SettingsQuickSettingsPopup
{
    Q_OBJECT

public:


                                    SettingsQuickSettingsPopup              (QWidget *);
    virtual                         ~SettingsQuickSettingsPopup             ();

private:

    void                            powerWifi                               (bool /*on*/);
    void                            paintWifi                               (bool /*on*/);


    PowerManagerLock                *m_powerLock;

protected:
    virtual void                    showEvent                               ( QShowEvent * );
    virtual void                    keyReleaseEvent                         ( QKeyEvent * );
    virtual void                    focusInEvent                            (QFocusEvent*);

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                    paintEvent                              (QPaintEvent *);


private slots:

    void                            handleClosePopupBtn                     ();
    void                            handleWifiStatusBtn                     ();
    void                            handleScreenLightStatusBtn              ();
    void                            handleBrightnessDecreasePressed         ();
    void                            handleBrightnessDecreaseLongPressed     ();
    void                            handleBrightnessIncreasePressed         ();
    void                            handleBrightnessIncreaseLongPressed     ();


public slots:

    void                            paintLight                              (bool /*on*/);
    void                            enabledWifi                             (bool);

signals:

    void                            userEvent                               ();

};

#endif // SETTINGSQUICKSETTINGSPOPUP_H
