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

#ifndef ELFUPDATEDEVICEAUTO_H
#define ELFUPDATEDEVICEAUTO_H

#include "FullScreenWidget.h"
#include "ui_SettingsUpdateDeviceAuto.h"
#include "QFwUpgradeOTA.h"
#include "ProgressDialog.h"
#include "ADConverter.h"

class SettingsUpdateDeviceAuto : public FullScreenWidget , protected Ui::SettingsUpdateDeviceAuto
{
    Q_OBJECT

public:

                        SettingsUpdateDeviceAuto            (QWidget *);
    virtual             ~SettingsUpdateDeviceAuto           ();
    void                checkUpdateAvailable                ();
    void                doCheck                             ();

protected slots:

    void                updateAutomatic                     ();
    void                gotOTACheckResult                   (int);
    void                nextPage                            ();
    void                previousPage                        ();


protected:

    void                upgradeDownloading();
    void                setupPagination                     ();
    /* http://qt-project.org/forums/viewthread/7340 */
    void                paintEvent                          (QPaintEvent *);
    QFwUpgradeOTA*      m_fwCheck;
    ProgressDialog*     m_OTAdownloadingDialog;
    QScrollBar*         vbar;
    uint                m_totalProgressBar;
    ADConverter *       m_converter;
};

#endif // ELFUPDATEDEVICEAUTO_H
