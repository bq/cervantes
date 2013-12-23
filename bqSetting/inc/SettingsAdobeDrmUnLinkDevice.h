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

#ifndef SETTINGSADOBEDRMUNLINKDEVICE_H
#define SETTINGSADOBEDRMUNLINKDEVICE_H

#include "FullScreenWidget.h"
#include "ui_SettingsAdobeDrmUnLinkDevice.h"

class SettingsGeneralTermsInfoAdobe;

class SettingsAdobeDrmUnLinkDevice : public FullScreenWidget , protected Ui::SettingsAdobeDrmUnLinkDevice{

    Q_OBJECT

public:
                SettingsAdobeDrmUnLinkDevice                            (QWidget *parent = 0);
    virtual     ~SettingsAdobeDrmUnLinkDevice                           ();
    void        setEmail                                                ();
    void        setup                                                   ();

private slots:

    void        handleDeactivate                                        ();
    void        showDeactivatedOK                                       ();
    void        showDeactivatedKO                                       ();
    void        handleLicenseBtn                                        ();

private:
    SettingsGeneralTermsInfoAdobe* adobeInformation;

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    void        paintEvent                                              (QPaintEvent *);

signals:
    void        showNewChild                                            (QWidget *);
    void        hideChild                                               ();
};
#endif // SETTINGSADOBEDRMUNLINKDEVICE_H
