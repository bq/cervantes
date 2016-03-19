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

#include "SettingsEnergySaving.h"

#include <QDebug>
#include <QKeyEvent>
#include <QPainter>

#include "SettingsSleepTimeMenu.h"
#include "SettingsSwitchOffTimeMenu.h"
#include "Settings.h"
#include "QBook.h"

SettingsEnergySaving::SettingsEnergySaving(QWidget *parent) : FullScreenWidget(parent)
{
    setupUi(this);

    connect(backBtn,        SIGNAL(clicked()), this, SIGNAL(hideMe()));
    connect(sleepTimeBtn,   SIGNAL(clicked()), this, SLOT(handleSleepTime()));
    connect(switchOffBtn,   SIGNAL(clicked()), this, SLOT(handleSwitchOffTime()));

    sleepTime = new SettingsSleepTimeMenu(this);
    switchOffTime = new SettingsSwitchOffTimeMenu(this);

    sleepTime->hide();
    switchOffTime->hide();

    connect(sleepTime,      SIGNAL(hideMe()), this, SIGNAL(hideChild()));
    connect(switchOffTime,  SIGNAL(hideMe()), this, SIGNAL(hideChild()));
}

SettingsEnergySaving::~SettingsEnergySaving()
{}

void SettingsEnergySaving::handleSleepTime()
{
    qDebug() << Q_FUNC_INFO << "Calling sleepTime";
    emit showNewChild(sleepTime);
}

void SettingsEnergySaving::handleSwitchOffTime()
{
    qDebug() << Q_FUNC_INFO << "Calling switchOffTime";
    emit showNewChild(switchOffTime);
}

void SettingsEnergySaving::keyReleaseEvent(QKeyEvent *event)
{
    qDebug() << Q_FUNC_INFO;
    if( event->key() == QBook::QKEY_BACK )
    {
        qDebug() << ": Closing...";
        emit hideMe();
        event->accept();
        return;
    }
    QWidget::keyReleaseEvent(event);
}

void SettingsEnergySaving::paintEvent (QPaintEvent *)
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

