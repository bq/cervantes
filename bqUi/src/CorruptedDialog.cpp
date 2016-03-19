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

#include "CorruptedDialog.h"
#include "QBook.h"
#include <QDebug>

CorruptedDialog::CorruptedDialog(QWidget* parent, QString message) : QDialog(parent, Qt::Popup | Qt::Dialog)
{
        qDebug() << Q_FUNC_INFO ;
        setupUi(this);
        errorLbl->setText(message);
        connect(formatBtn, SIGNAL(clicked()), this, SLOT(formatPartition()));
        connect(mountBtn, SIGNAL(clicked()), this, SLOT(mount()));
        connect(noMountBtn, SIGNAL(clicked()), this, SLOT(doNoMount()));
}

CorruptedDialog::~CorruptedDialog()
{
    qDebug() << Q_FUNC_INFO;
}

void CorruptedDialog::formatPartition()
{
    qDebug() << Q_FUNC_INFO;
    done(ActionFormat);
}

void CorruptedDialog::mount()
{
    qDebug() << Q_FUNC_INFO;
    done(ActionMount);
}

void CorruptedDialog::doNoMount()
{
    qDebug() << Q_FUNC_INFO;
    done(ActionNoMount);
}
