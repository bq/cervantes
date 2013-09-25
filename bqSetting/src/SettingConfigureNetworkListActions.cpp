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

#include "SettingConfigureNetworkListActions.h"

#include <QFile>
#include <QPainter>
#include <QDebug>

SettingConfigureNetworkListActions::SettingConfigureNetworkListActions( QWidget* parent ) :
    QWidget(parent)
{
    setupUi(this);

    connect(DHCPBtn,        SIGNAL(clicked()), this, SIGNAL(setDHCP()));
    connect(manualBtn,      SIGNAL(clicked()), this, SIGNAL(setManual()));

}

SettingConfigureNetworkListActions::~SettingConfigureNetworkListActions()
{}

void SettingConfigureNetworkListActions::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
