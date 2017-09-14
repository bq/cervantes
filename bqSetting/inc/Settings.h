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

#ifndef ELFSETTINGSMENU_H
#define ELFSETTINGSMENU_H

#include "ui_Settings.h"
#include "QBookForm.h"

class QBookApp;
class FullScreenWidget;
class QStackedWidget;
class SettingsDeviceInfo;
class SettingsMyAccountMenu;
class SettingsDeviceOptionsMenu;
class SettingsBrightness;
class SettingsDeviceInfo;
class SettingsAboutUs;

class Settings : public QBookForm, protected Ui::Settings
{
    Q_OBJECT

public:
                                    Settings                     (QWidget* parent = 0);
    virtual                         ~Settings                    ();
    SettingsDeviceInfo*             showInformation              ();
    SettingsDeviceOptionsMenu*      getDeviceOptions             ();
    void                            goToViewerMenu               ();
    void                            goToLightMenu                ();
    void                            hidePreviousChild            ();

public slots:

    void                showElement                     (QWidget*);
    void                hideElement                     ();

signals:

    void                goToHome                        ();
    void                goToViewer                      ();

private:

    FullScreenWidget*           m_currentForm;
    QStackedWidget*             widgetStack;
    SettingsMyAccountMenu*      m_myAccount;
    SettingsDeviceOptionsMenu*  m_deviceOptions;
    SettingsBrightness*         m_light;
    SettingsDeviceInfo*         m_deviceInfo;
    SettingsAboutUs*            m_aboutUs;
    void                        showEvent               ( QShowEvent * ) ;

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    void                paintEvent                      (QPaintEvent *);
    virtual void        activateForm                    ();
    virtual void        deactivateForm                  ();
    virtual void        keyReleaseEvent                   (QKeyEvent *);
    int                 isChildSetting;

private slots:

    void                handleDevicesBtn                ();
    void                handleInformationBtn            ();
    void                handleWifiBtn                   ();
    void                handleMyAccountBtn              ();
    void                handleDevelopmentBtn            ();
    void                handleLightBtn                  ();
    void                handleBrowserBtn                ();
    void                handleAboutUsBtn                ();
    void                connectWifiObserverBrowser      ();
    void                disconnectWifiObserverBrowser   ();
    void                goToBrowser                     ();
    void                activateDevice                  ();
    void                connectWifiObserverWizard       ();
    void                disconnectWifiObserverWizard    ();
    void                handleGoToViewer                ();
};

#endif // ELFSETTINGSMENU_H
