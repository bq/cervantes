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

#include "PowerOffDialog.h"
#include "ui_ElfPowerOffDialog.h"




#include <QDialog>
#include <QTimerEvent>
#include <QBasicTimer>
#include <Qt>

PowerOffDialog::PowerOffDialog(QWidget * parent) :
    QDialog(parent,Qt::SplashScreen)
{
    setupUi(this);
    connect(ok, SIGNAL(clicked()), this, SLOT(sleepReq()));
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(cancelReq()));
    connect(ok_2, SIGNAL(clicked()), this, SLOT(powerOffReq()));
}

void PowerOffDialog::sleepReq()
{
    done(SLEEP);
}

void PowerOffDialog::cancelReq()
{
    done(CANCEL);
}

void PowerOffDialog::powerOffReq()
{
    done(POWEROFF);
}




