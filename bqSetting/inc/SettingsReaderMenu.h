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

#ifndef ELFREADMODE_H
#define ELFREADMODE_H

#include "FullScreenWidget.h"
#include "ui_SettingsReaderMenu.h"

class SettingsReaderPageTurning;
class SettingsReaderProgressBar;

class SettingsReaderMenu : public FullScreenWidget , protected Ui::SettingsReaderMenu

{
    Q_OBJECT

public:

    enum TurnPageMode {
        MODE_ONEHAND, // bottom -> nextScreen & top -> previousScreen
        MODE_TWOHANDS // right -> nextScreen & left -> previous
    };


                    SettingsReaderMenu                          (QWidget *);
    virtual         ~SettingsReaderMenu                         ();

    void            goToReaderProgressMode                      ();
    bool            fromViewer                                  () { return b_fromViewer; }
    void            setFromViewer                               (bool isFromViewer) { b_fromViewer = isFromViewer; }

private:

    SettingsReaderPageTurning*      settingsReaderPageTurning;
    SettingsReaderProgressBar*      settingsReaderProgressBar;
    bool                            b_fromViewer;

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    void                            paintEvent                          (QPaintEvent *);

private slots:

    void                            showReaderPageTurning               ();  
    void                            showReaderProgressBar               ();
    void                            hideTopElement                      ();

signals:
    void                            returnToViewer                      ();
    void                            showNewChild                        (QWidget*);
    void                            hideChild                           ();

};

#endif // ELFREADMODE_H
