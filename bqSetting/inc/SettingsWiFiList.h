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

#ifndef SETTINGSWIFILIST_H
#define SETTINGSWIFILIST_H

#include <QList>
#include <QString>
#include <QSize>
#include <ConnectionManager.h>
#include "QBookForm.h"
#include "ui_SettingsWiFiList.h"

// Predeclarations
class FullScreenWidget;
class SelectionDialog;
class MyQLineEdit;
class QFwUpgradeOTA;
class PowerManagerLock;
class InfoDialog;
class Network;
class ComboBoxPowerStatus;
class SettingsWiFiListItem;
struct QSettingWifiDelegate;

class QWidget;

class SettingsWiFiList : public QBookForm, protected Ui::SettingsWiFiList
{
    Q_OBJECT

public:
    enum eBehavior
    {
        BACK_BEHAVIOR,
        NO_ACTION_BEHAVIOR,
        CLOSE_BEHAVIOR,
        AUTOMATIC_CLOSE_BEHAVIOUR
    };

    SettingsWiFiList(QWidget *parent, SettingsWiFiList::eBehavior behaviour = CLOSE_BEHAVIOR);
    virtual ~SettingsWiFiList();

    void                            ShowConfigList                          ();
    bool                            isPowerOn                               () const;


    InfoDialog*                     m_infoDialog;

    void                            setBehaviour                            (eBehavior behavior);
    eBehavior                       getBehaviour                            () { return m_behavior;}

signals:
    void                            wifiConnectionCanceled                  ();

public slots:
    void                            backButtonPressed                       ();
    void                            disconnectWifi                          (QString path, bool keepPowered = false);
    void                            forgetWifi                              (QString path, bool keepPowered = false);
    void                            wifiSelected                            (QString);
    void                            showDialog                              (QString);
    void                            showConnectingMessage                   ();
    void                            showDisconnectingMessage                ();
    void                            updateWifiList                          ();
    void                            addWifi                                 ();
    void                            paintStatus                             ();
    void                            paintWifiList                           ();
    void                            emulateLost                             ();
    void                            powerOnOffWifi                          ();
    void                            powerOnWifi                             ();
    void                            powerOffWifi                            ();
    void                            resetSleepTimer                         ();



private:
    void                            finishConnecting();
    void                            wifiError(QString);
    PowerManagerLock                *m_powerLock;
    ComboBoxPowerStatus*            comboBoxPowerStatus;

    void                            setUiPowerOnOff                                 (bool powerOn);
    void                            setupWifis                                      (QList<Network> networks);
    void                            updateList                                      ();
    ConnectionManager               *manager;
    QSettingWifiDelegate*           settingWifiDelegate;
    SelectionDialog*                dialog;
    MyQLineEdit*                    edit;
    QSize                           m_itemSize;
    QList<Network>                  m_available_wifis;
    bool                            m_configurating;
    bool                            isConfigList;
    bool                            dis_connecting;
    bool                            connectedHidden;
    bool                            connectionRequested;
    bool                            b_showWifiSetting;

    QFwUpgradeOTA*                  fwCheck;

    FullScreenWidget*               m_currentForm;

    QProcess*                       connectHelper;
    bool                            deleted;
    NetworkService                  *connectingService;
    eBehavior                       m_behavior;
    QString                         connectingServicePath;
    QTimer                          connectingTimer;
    QBasicTimer                     m_sleepTimer;
    int                             i_timeSleepMSecs;
    QList<SettingsWiFiListItem*>    wifis;
    QList<Network>                  networks;

    int                             page;

private slots:
    void                            noWifisAvailable                        ();
    void                            disconnectWifiRequest                   (QString);
    void                            forgetWifiRequest                       (QString);
    void                            connectHelperOutput                     ();
    void                            connectHelperFinished                   (int,QProcess::ExitStatus);
    void                            passwordRequested                       ();
    void                            passwordSent                            ();
    void                            wrongPassword                           (QString);
    void                            cancelPassword                          ();
    void                            connectionFailed                        (QString, QString);
    void                            connectionTimeout                       ();
    void                            connectingError                         ();
    void                            connected                               ();
    void                            connectedNetwork                        ();
    void                            networksCountChanged                    ();
    void                            handleDisconnected                      ();
    void                            handleConnected                         ();
    void                            handleWifiSelected                      (Network);
    void                            handleWifiLongPressed                   (Network);
    void                            handleSomeThingHasChanged               ();
    void                            back                                    ();
    void                            forward                                 ();
    void                            addWifiEditClicked                      ();
    void                            addWifiEditHide                         ();
    void                            addWifiEditDialogAccepted               ();
    void                            addWifiEditDialogRejected               ();
    void                            hideAddNetworkDialog                    ();


protected:
    int                             i_currentLevel;
    int                             i_lastWifiListCount;
    virtual void                    deactivateForm                          ();
    virtual void                    activateForm                            ();
    virtual void                    timerEvent                              (QTimerEvent*);

    /* http://qt-project.org/forums/viewthread/7340 */
    void                            paintEvent                              (QPaintEvent *);
};
#endif // SETTINGSWIFILIST_H
