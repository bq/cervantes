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

#include <QPainter>
#include "SettingsRestoreDefaultValues.h"
#include "SelectionDialog.h"
#include "InfoDialog.h"
#include "QBook.h"

SettingsRestoreDefaultValues::SettingsRestoreDefaultValues(QWidget *parent)
    : FullScreenWidget(parent)
{
    setupUi(this);
    connect(backBtn, SIGNAL(clicked()), this, SIGNAL(hideMe()));
    connect(overrideDeviceBtn, SIGNAL(clicked()), this, SLOT(restoreToDefault()));
}

SettingsRestoreDefaultValues::~SettingsRestoreDefaultValues()
{
    // Do Nothing
}

void SettingsRestoreDefaultValues::restoreToDefault()
{
    SelectionDialog* confirmDialog = new SelectionDialog(this,tr("Do you want to restore the device to default settings? This will delete all your personal data"));
    confirmDialog->exec();
    bool result = confirmDialog->result();
    delete confirmDialog;
    if(!result)
        return;

    InfoDialog* dialog = new InfoDialog(this,tr("This process can take a few minutes."));
    dialog->show();
    qApp->processEvents();

   // bqDeviceSettingsMenu::restore();

    QApplication::quit();
    delete dialog;
}

void SettingsRestoreDefaultValues::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }
