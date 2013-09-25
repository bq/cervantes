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

#ifndef ELFLANGUAGEMENU_H
#define ELFLANGUAGEMENU_H

#include "FullScreenWidget.h"
#include "ui_SettingsLanguageMenu.h"

class QButtonGroup;
class InfoDialog;

class SettingsLanguageMenu : public FullScreenWidget, protected Ui::SettingsLanguageMenu
{
    Q_OBJECT

public:

    enum {
        LANG_EN,
        LANG_ES,
        LANG_PT,
        LANG_CA,
        LANG_EU,
        LANG_GL,
        LANG_NUMBER
    };

                        SettingsLanguageMenu                (QWidget*);
    virtual             ~SettingsLanguageMenu               ();

protected:

    QButtonGroup*       m_buttonGroup;
    const char*         getLangByInt                        (int langInt) const;
    InfoDialog*         infoDialog;

    /* http://qt-project.org/forums/viewthread/7340 */
    void                paintEvent                          (QPaintEvent *);

private slots:

    void                setCurrentLanguage                  ();
    void                changeLanguage                      (int);
    int                 getIntByLangCode                    (const QString& code) const;
    void                quitApp                             ();
};


#endif // ELFLANGUAGELIST_H
