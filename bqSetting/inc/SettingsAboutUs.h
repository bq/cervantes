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

#ifndef SETTINGSABOUTUS_H
#define SETTINGSABOUTUS_H

#include "FullScreenWidget.h"
#include "ui_SettingsAboutUs.h"

class SettingsAboutUs : public FullScreenWidget, protected Ui::SettingsAboutUs
{
    Q_OBJECT

public:
                                    SettingsAboutUs              (QWidget*);
    virtual                         ~SettingsAboutUs             ();
    //void                            setup                        ();

protected:

    virtual void                    keyReleaseEvent                   (QKeyEvent *);

    /* http://qt-project.org/forums/viewthread/7340 */
    void                            paintEvent                      (QPaintEvent *);
    //void                            showEvent                       ( QShowEvent * ) ;

};

#endif // SETTINGSABOUTUS_H
