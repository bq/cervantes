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

#ifndef ELFMYACCOUNTMENU_H
#define ELFMYACCOUNTMENU_H

#include "ui_SettingsMyAccountMenu.h"
#include "FullScreenWidget.h"


class SettingsGeneralTermsInfo;
class SettingsUnLinkStoreDevice;
class SettingsAdobeDrmLinkDevice;
class SettingsAdobeDrmUnLinkDevice;

class SettingsMyAccountMenu : public FullScreenWidget, protected Ui::SettingsMyAccountMenu
{
    Q_OBJECT

public:
                                    SettingsMyAccountMenu               (QWidget* parent = 0);
    virtual                         ~SettingsMyAccountMenu              ();
    void                            setup                               ();

private:

    SettingsAdobeDrmLinkDevice*     settingsAdobeDrmLinkDevice;
    SettingsAdobeDrmUnLinkDevice*   settingsAdobeDrmUnLinkDevice;
    SettingsGeneralTermsInfo*       detailInformation;
    SettingsUnLinkStoreDevice*      settingsUnLinkStoreDevice;

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    void                            paintEvent                          (QPaintEvent *);
    void                            showEvent                           ( QShowEvent * );
    virtual void                    keyReleaseEvent                     (QKeyEvent *);

private slots:

    void                            handleAdobeDrmBtn                   ();
    void                            showMyAccount                       ();
    void                            showAdobeDrm                        ();
    void                            activated                           ();
    void                            hideTopElement                      ();
    void                            handleGeneralTermsBtn               ();
    void                            connectWifiObserverAdobeDrm         ();
    void                            disconnectWifiObserverAdobeDrm      ();
    void                            showNewChild                        (QWidget*);

};


#endif // ELFMYACCOUNTMENU_H
