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

#ifndef DEVICEINFODIALOG_H
#define DEVICEINFODIALOG_H

#include <QDialog>
#include "ui_DeviceInfoDialog.h"

class PowerManagerLock;

class DeviceInfoDialog: public QDialog, protected Ui::DeviceInfoDialog
{
    Q_OBJECT

public:

                            DeviceInfoDialog                    (QWidget *parent = 0);
                            ~DeviceInfoDialog                   ();

private slots:

    void                    handleOk                            ();
    void                    fillInfo                            ();

protected:
    /* http://qt-project.org/forums/viewthread/7340 */
    void                paintEvent                      (QPaintEvent *);

};


#endif
