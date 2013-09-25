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

#ifndef UNLINKFNACACCOUNT_H
#define UNLINKFNACACCOUNT_H

#include "ui_SettingsUnLinkStoreDevice.h"
#include "FullScreenWidget.h"

class SettingsUnLinkStoreDevice : public FullScreenWidget, protected Ui::SettingsUnLinkStoreDevice
{
    Q_OBJECT

public:

                        SettingsUnLinkStoreDevice                       (QWidget *);
    virtual             ~SettingsUnLinkStoreDevice                      ();
                        void                setup                       ();

public slots:

    void                restoreSettinsAndPartitions                     (QDialog *);
    void                tryToDisassociate                               ();
    void                handleUnlinkBtn                                 ();

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    void                paintEvent                                      (QPaintEvent *);
    void                showEvent                                       ( QShowEvent * ) ;
    void                showSubscriptionInfo                            ();
    void                connectWifiObserverUnlinkBtn                    ();
    QString             getDate                                         ( const QString& ) const;
    QString             getMonth                                        ( const QString& ) const;
    QString             getStatusString                                 (int status);

protected slots:
    void                disconnectWifiObserverUnlinkBtn                 ();

};

#endif // UNLINKFNACACCOUNT_H
