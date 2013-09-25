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

#ifndef SETTINGSDEVICEOPTIONSMENU_H
#define SETTINGSDEVICEOPTIONSMENU_H

#include "FullScreenWidget.h"
#include "ui_SettingsDeviceOptionsMenu.h"

class SettingsLanguageMenu;
class SettingsDateTimeMenu;
class SettingsEnergySaving;
class SettingsReaderMenu;
#ifndef HACKERS_EDITION
class SettingsDictionariesMenu;
#endif
class SettingsPageScreenSaver;

class SettingsDeviceOptionsMenu : public FullScreenWidget, protected Ui::SettingsDeviceOptionsMenu
{
    Q_OBJECT

public:
                                SettingsDeviceOptionsMenu                   (QWidget* parent = 0);
    virtual                     ~SettingsDeviceOptionsMenu                  ();
    void                        connectWifiObserverDictionary               ();
    void                        setup                                       ();

#ifndef HACKERS_EDITION
public slots:
    void                        showDictionaryMenu                          ();
#endif

private slots:

    void                        handleLanguage                              ();
    void                        handleTimeZone                              ();
    void                        handleEnergySaving                          ();
    void                        handleReadMode                              ();

    void                        handleScreenSaver                           ();

    void                        hideTopElement                              ();


private:

    SettingsLanguageMenu*       language;
    SettingsDateTimeMenu*       timeZone;
    SettingsEnergySaving*       energySaving;
    SettingsReaderMenu*         readMode;
#ifndef HACKERS_EDITION
    SettingsDictionariesMenu*   dictionary;
#endif
    SettingsPageScreenSaver*    screenSaver;

protected:

    virtual void                keyReleaseEvent                             (QKeyEvent *);

    /* http://qt-project.org/forums/viewthread/7340 */
    void                        paintEvent                                  (QPaintEvent *);

};


#endif // SETTINGSDEVICEOPTIONSMENU_H
