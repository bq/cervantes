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

#ifndef ELFSELECTDICTIONARYMENU_H
#define ELFSELECTDICTIONARYMENU_H

#include "ui_SettingsDictionariesMenu.h"
#include "Dictionary.h"
#include "SettingsPagerCont.h"
#include "SettingsDateTimeMenuItem.h"
#include "FullScreenWidget.h"
#include "PowerManager.h"

class QStringList;
class QString;
class QButtonGroup;
class SettingsDictionariesMenuItem;
class SettingsDictionariesInfo;
class ProgressDialog;

class SettingsDictionariesMenu : public FullScreenWidget, protected Ui::SettingsDictionariesMenu
{
    Q_OBJECT

public:

                                                SettingsDictionariesMenu        (QWidget*);
    virtual                                     ~SettingsDictionariesMenu       ();


private:

        int                                     page;
        bool                                    b_downloadCanceled;
        QList<SettingsDictionariesMenuItem*>    items;
        QString                                 currentDictio;
        void                                    setupDictionaries               ();
        int                                     getTotalPages                   ();
        void                                    getCurrentDictionary            ();
        bool                                    newDictionaryAvailable          ( );
        void                                    deleteDictionaryFiles           ( QStringList );
        void                                    connectDictionaryWifiObserver   ( );
        SettingsDictionariesInfo*               detailInformation;
        ProgressDialog*                         downloadDialog;
        QStringList                             newDictionaryNames;
        PowerManagerLock*                       m_powerLock;


protected:

        void                                    setDictionarySelected           ();
        QList<DictionaryParams>                 dictionaryList;
        QList<DictionaryParams>                 availableDictsList;
        QList<DictionaryParams>                 pendingDictsList;

        /* http://qt-project.org/forums/viewthread/7340 */
        void                                    paintEvent                  (QPaintEvent* );

        virtual void                            showEvent                   (QShowEvent* );


private slots:

        void                                    init                                ( );
        void                                    changeDictionary                    ( QString );
        void                                    showDictioInformation               ( );
        void                                    back                                ( );
        void                                    forward                             ( );
        void                                    paintDictios                        ( );
        void                                    hideTopElement                      ( );
        void                                    activateExtraDicts                  ( );
        void                                    downloadCanceled                    ( );
        void                                    downloadDictionaries                ( );
        void                                    disconnectDictionaryWifiObserver    ( );

};


#endif // ELFSELECTDICTIONARYMENU_H
