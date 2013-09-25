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

#ifndef WizardLanguage_H
#define WizardLanguage_H

#include "ui_WizardLanguage.h"
#include "FullScreenWidget.h"

class QWidget;

class WizardLanguage : public FullScreenWidget, protected Ui::WizardLanguage
{
    Q_OBJECT

public:
    WizardLanguage(QWidget* parent = 0);
    virtual ~WizardLanguage();
    enum language{
        ENGLISH,
        SPANISH,
        PORTUGUES,
        CATALAN,
        BASQUE,
        GALICIAN
    };

    int settingsLanguage;
    int choosenLanguage;

signals:

    void languageSelected();

private slots:

    void                                    eng_BtnClicked                              ();
    void                                    spa_BtnClicked                              ();
    void                                    por_BtnClicked                              ();
    void                                    cat_BtnClicked                              ();
    void                                    bas_BtnClicked                              ();
    void                                    gal_BtnClicked                              ();
    void                                    languageAccepted                            ();
    int                                     getLangugeFromSettings                      ();
    void                                    quitApp                                     ();
    void                                    handleDeviceInfo                            ();

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    void                                    paintEvent                                  (QPaintEvent *);
};

#endif // WizardLanguage_H
