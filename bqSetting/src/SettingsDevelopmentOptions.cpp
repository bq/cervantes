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

#include "SettingsDevelopmentOptions.h"
#include "QBook.h"
#include "QBookDevel.h"
#include "SelectionDialog.h"
#include "InfoDialog.h"
#include "FrontLight.h"
#include <QDebug>
#include <QTimer>
#include <QKeyEvent>
#include <QPainter>

SettingsDevelopmentOptions::SettingsDevelopmentOptions(QWidget *parent) : FullScreenWidget(parent)
{
        qDebug() << Q_FUNC_INFO;

        setupUi(this);

        if (QBookDevel::isUsbNetworkEnabled())
            enableUsbNetBtn->setStyleSheet("background-image: url(:res/on_btn.png);");
        else
            enableUsbNetBtn->setStyleSheet("background-image: url(:res/off_btn.png);");

        if (QBookDevel::isTelnetEnabled())
            enableTelnetBtn->setStyleSheet("background-image: url(:res/on_btn.png);");
        else
            enableTelnetBtn->setStyleSheet("background-image: url(:res/off_btn.png);");

        connect(backBtn,SIGNAL(clicked()), this, SIGNAL(hideMe()));
        connect(enableUsbNetBtn, SIGNAL(clicked()), this, SLOT(handleUsbBtn()));
        connect(enableTelnetBtn, SIGNAL(clicked()), this, SLOT(handleTelnetBtn()));
}

SettingsDevelopmentOptions::~SettingsDevelopmentOptions()
{
        qDebug() << Q_FUNC_INFO;
}



void SettingsDevelopmentOptions::handleUsbBtn()
{
        qDebug() << Q_FUNC_INFO;
 	if (QBook::settings().value("setting/hackersUsbNetwork").toBool()) {
		enableUsbNetBtn->setStyleSheet("background-image: url(:res/off_btn.png);");
		QBook::settings().setValue("setting/hackersUsbNetwork", false);
        	QBookDevel::enableUsbNetwork(false);
	} else {
		enableUsbNetBtn->setStyleSheet("background-image: url(:res/on_btn.png);");
		QBook::settings().setValue("setting/hackersUsbNetwork", true);
        	QBookDevel::enableUsbNetwork(true);
	}
}

void SettingsDevelopmentOptions::handleTelnetBtn()
{
        qDebug() << Q_FUNC_INFO;
 	if (QBook::settings().value("setting/hackersTelnet").toBool()) {
		enableTelnetBtn->setStyleSheet("background-image: url(:res/off_btn.png);");
		QBook::settings().setValue("setting/hackersTelnet", false);
        	QBookDevel::disableTelnet();
	} else {
		enableTelnetBtn->setStyleSheet("background-image: url(:res/on_btn.png);");
		QBook::settings().setValue("setting/hackersTelnet", true);
        	QBookDevel::enableTelnet();
	}
}

void SettingsDevelopmentOptions::paintEvent (QPaintEvent *)
{
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

