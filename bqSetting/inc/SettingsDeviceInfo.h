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

#ifndef ELFDEVICEMENU_H
#define ELFDEVICEMENU_H

#include "FullScreenWidget.h"
#include "ui_SettingsDeviceInfo.h"

class SettingsRestoreDefaultValues;
class SettingsUpdateDeviceAuto;
class SettingsTechnicalInfo;
class SettingsDictionariesMenu;
class Battery;

class SettingsDeviceInfo : public FullScreenWidget, protected Ui::SettingsDeviceInfo
{
    Q_OBJECT

public:
                                    SettingsDeviceInfo              (QWidget* parent = 0);
    virtual                         ~SettingsDeviceInfo             ();
    void                            setup                           ();

public slots:
    void                            showUpgradeMenu                 ();
    void                            restoreSettinsAndPartitions     (QDialog *);

private slots:

    void                            updateDevice                    ();
    void                            showDeviceInfo                  ();
    void                            updateExternalMemoryInfo        ();
    void                            hideTopElement                  ();
    void                            connectWifiObserverUpgrade      ();
    void                            disconnectWifiObserverUpgrade   ();
    void                            restoreOTACheck                 ();
    void                            restoreDevice                   ();
    void                            refreshBatteryLevel             ( int );


private:

    int                             isChildSetting;
    FullScreenWidget*               m_currentForm;
    SettingsUpdateDeviceAuto*       upgradeMenu;
    SettingsTechnicalInfo*          deviceInfo;
    Battery*                        battery;

protected:

    virtual void                    keyReleaseEvent                   (QKeyEvent *);

    /* http://qt-project.org/forums/viewthread/7340 */
    void                            paintEvent                      (QPaintEvent *);
    void                            showEvent                       ( QShowEvent * ) ;


};

#endif // ELFDEVICEMENU_H
