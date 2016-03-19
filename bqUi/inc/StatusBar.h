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

#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QTimer>
#include <QFrame>
#include <QBasicTimer>
#include "ui_StatusBar.h"
#include "BookInfo.h"

#define EMPTY_BATTERY_THRESHOLD_ELF 10
#define ONE_BAR_BATTERY_THRESHOLD_ELF 22
#define TWO_BAR_BATTERY_THRESHOLD_ELF 35
#define THREE_BAR_BATTERY_THRESHOLD_ELF 48
#define FOUR_BAR_BATTERY_THRESHOLD_ELF 60
#define FIVE_BAR_BATTERY_THRESHOLD_ELF 72
#define SIX_BAR_BATTERY_THRESHOLD_ELF 85

class QString;
class QTimerEvent;
class QWidget;
class InfoDialog;
class QStringList;

class StatusBar : public QWidget, protected Ui::StatusBar
{
    Q_OBJECT

public:

    enum {
        ICON_SOCIAL,
        ICON_SYSTEM,
        ICON_DOWNLOADING,
        ICON_SOCIALCOUNT,
        ICON_WIFI,
        BUTTON_READ
    };

    enum WifiStatusEnum {
        WIFI_OFF,
        WIFI_PROCESSING,
        WIFI_DISCONNECTED,
        WIFI_LEVEL_1,
        WIFI_LEVEL_2,
        WIFI_LEVEL_3,
        WIFI_LEVEL_4
    };
    enum batteryLevelEnum{
        EMPTY_BAR,
        ONE_BAR,
        TWO_BAR,
        THREE_BAR,
        FOUR_BAR,
        FIVE_BAR,
        SIX_BAR,
        SEVEN_BAR,
        CHARGING
    };

                                        StatusBar                           (QWidget *parent = 0);
                                        ~StatusBar                          ();


    void                                setWifiStatus                       (WifiStatusEnum);
    void                                setMenuTitle                        (const QString&);
    void                                setIconVisible                      (int,bool) ;
    void                                setBusy                             (bool);
    void                                setSpinner                          (bool);
    void                                hideReadbutton                      ();
    void                                hideWifiCont                        ();
    void                                showButtons                         ();

private:

    int                                 i_currentImg;
    int                                 lastLevel;
    QStringList                         workingImages;
    WifiStatusEnum                      m_currentStatus;
    void                                initWorkingImages                   ();
    int                                 getBatteryImage                     (int);

private slots:

    void                                updateBusyIcon                      ();
    void                                showBatteryDialog                    ();

public slots:

    void                                updateBatteryWarning                ();
    void                                setBatteryLevel                     (int);
    void                                updateChargerState                  (bool);
    void                                updateUSBState                      (bool);
    void                                updateTime                          ();
    void                                updateLinkPcState                   (bool);
    void                                wifiAndLightPopup                   ();
    void                                setVisible                          (bool /*visible*/);
    void                                openLastBook                        ();
    void                                switchLight                         (bool /*on*/);
    void                                handleSyncStart                     ();
    void                                handleSyncEnd                       ();

signals:
    void                                openLastContent                     ();
    void                                wifiConnectionCanceled              ();

protected:

    QBasicTimer                         m_timer;
    QTimer                              m_timer_busy;
    InfoDialog*                         m_dialog;
    void                                timerEvent                          (QTimerEvent */*event*/);
    void                                updateTimeUI                        (const QString&);

    /* http://qt-project.org/forums/viewthread/7340 */
    void                                paintEvent                          (QPaintEvent *);


};


#endif // STATUSBAR_H
