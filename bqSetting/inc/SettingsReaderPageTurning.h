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

#ifndef SETTINGSREADERPAGETURNING_H
#define SETTINGSREADERPAGETURNING_H

#include "FullScreenWidget.h"
#include "ui_SettingsReaderPageTurning.h"

class SettingsReaderPageTurning : public FullScreenWidget , protected Ui::SettingsReaderPageTurning
{
    Q_OBJECT

    public:

        enum TurnPageMode {
            MODE_ONEHAND, // bottom -> nextScreen & top -> previousScreen
            MODE_TWOHANDS // right -> nextScreen & left -> previous
        };


                        SettingsReaderPageTurning                          (QWidget *);
        virtual         ~SettingsReaderPageTurning                         ();

        static TurnPageMode getCurrentTurnPageMode                         ();

    private:

        QButtonGroup*   m_buttonGroupTurnMode;
        QButtonGroup*   m_buttonGroupRefreshCount;

protected:
        void            setCurrentTurnMode();
        void            setEnableSwipeMode();
        void            setRefreshCount();

        /* http://qt-project.org/forums/viewthread/7340 */
        void            paintEvent                                  (QPaintEvent *);

private slots:

        void            changeTurnMode                              (int);
        void            changeRefreshCount                          (int);
        void            enableSwipeBtnHandler                       ();

};

#endif // SETTINGSREADERPAGETURNING_H
