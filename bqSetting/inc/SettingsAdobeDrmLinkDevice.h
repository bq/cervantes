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

#ifndef SETTINGSADOBEDRMLINKDEVICE_H
#define SETTINGSADOBEDRMLINKDEVICE_H
#include "FullScreenWidget.h"
#include "Keyboard.h"
#include "ui_SettingsAdobeDrmLinkDevice.h"

class InfoDialog;
class SettingsGeneralTermsInfoAdobe;

class SettingsAdobeDrmLinkDevice : public FullScreenWidget , protected Ui::SettingsAdobeDrmLinkDevice{

    Q_OBJECT

public:
                SettingsAdobeDrmLinkDevice                          (QWidget *parent = 0);
    virtual     ~SettingsAdobeDrmLinkDevice                         ();
                Keyboard* keyboard;
    void        setup                                               ();

private:

    InfoDialog* m_waitingDialog;
    SettingsGeneralTermsInfoAdobe* adobeInformation;
    bool        m_userIDTyped;
    bool        m_userPWTyped;
    QString     m_initialIDString;
    QString     m_initialPWString;

signals:

    void        activatedOK                                         ();
    void        showNewChild                                        (QWidget* widget);
    void        hideChild                                           ();

private slots:

    void        showActivatedOK                                     ();
    void        showActivatedKO                                     (int err);
    void        handleID                                            ();
    void        handlePW                                            ();
    void        handleOK                                            ();
    void        handleLicenseBtn                                    ();

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    void        paintEvent                                          ( QPaintEvent* );
    void        hideEvent                                           ( QHideEvent * );
    void        showEvent                                           ( QShowEvent * ) ;

};

#endif // SETTINGSADOBEDRMLINKDEVICE_H
