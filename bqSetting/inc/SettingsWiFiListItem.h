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

#ifndef SETTINGSWIFILISTITEM_H
#define SETTINGSWIFILISTITEM_H

#include "ui_SettingsWiFiListItem.h"
#include "GestureWidget.h"
#include "Network.h"

class SettingsWiFiListItem : public GestureWidget, protected Ui::SettingsWiFiListItem
{
    Q_OBJECT

public:
                                    SettingsWiFiListItem                    (QWidget * parent = 0);
    virtual                          ~SettingsWiFiListItem();

    void                            setWiFi                                 (Network);
    void                            paint                                   ();
    void                            hideLabels                              ();

signals:

    void                            wifiSelected                            (Network);
    void                            wifiLongPressed                         (Network);

protected slots:

    void                            handleTap                               ();
    void                            processLongpress                        ();

    /* http://qt-project.org/forums/viewthread/7340 */
    void                            paintEvent                              (QPaintEvent *);

private:

    Network                         current_network;
};

#endif // SETTINGSWIFILISTITEM_H
