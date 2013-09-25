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

#ifndef SETTINGSCONFIGURENETWORK_H
#define SETTINGSCONFIGURENETWORK_H

#include "QBookForm.h"
#include "Network.h"
#include "Keyboard.h"
#include "ui_SettingsConfigureNetwork.h"

class SettingConfigureNetworkListActions;

class SettingsConfigureNetwork : public QBookForm, protected Ui::SettingsConfigureNetwork
{
    Q_OBJECT

public:

                                SettingsConfigureNetwork                  (QString path, QBookForm *parent = 0);
    virtual                     ~SettingsConfigureNetwork                 ();

public slots:

    void                        closeDialog                         ();

private:

    QString                     m_path;
    QList<MyQLineEdit*>           editWidgets;
    Keyboard			*keyboard;
    MyQLineEdit			*currentEdit;
    bool                        isValidField(MyQLineEdit*);
    void                        fillConfig();
    void                        updateFields();

private slots:
    void                        saveConfig();
    void                        showOptions();
    void                        setDHCP();
    void                        setManual();
    void                        onEditClicked(MyQLineEdit *fakeSender=NULL);
    void                        onEditChanged(const QString&);
    void                        onHideKeyboard();
    void                        handleSaveBtnStyle();

protected:

    void                                    activateForm                        ();
    void                                    deactivateForm                      ();
    SettingConfigureNetworkListActions*     m_actions;
    bool                                    fields_ok;

    /* http://qt-project.org/forums/viewthread/7340 */
    void                        paintEvent                          (QPaintEvent *);

};

#endif 
