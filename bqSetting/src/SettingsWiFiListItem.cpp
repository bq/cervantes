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

#include "SettingsWiFiListItem.h"

#include "GestureWidget.h"
#include "bqUtils.h"
#include "QBook.h"
#include <QDebug>
#include <QPainter>

#define WIFI_ITEM_SSID_MAX_LENGTH 30

SettingsWiFiListItem::SettingsWiFiListItem(QWidget* parent) : GestureWidget(parent),
    current_network("none")
{
        qDebug() << Q_FUNC_INFO;
        setupUi(this);

        wifiConnect->hide();
        lockWifiLbl->hide();
        noSignalWifi->hide();
        signalWifi_1->hide();
        signalWifi_2->hide();
        signalWifi_3->hide();
        signalWifi_4->hide();

        connect(this,SIGNAL(tap()),this, SLOT(handleTap()));
        connect(this,SIGNAL(longPressStart()),this, SLOT(processLongpress()));
}

void SettingsWiFiListItem::setWiFi(Network n)
{
        qDebug() << Q_FUNC_INFO;
        current_network = n;
}

SettingsWiFiListItem::~SettingsWiFiListItem()
{
        qDebug() << Q_FUNC_INFO;
}

void SettingsWiFiListItem::paint()
{
        qDebug() << Q_FUNC_INFO;

        QString connectingStr = tr("Connecting... ");

        if(current_network.isConnecting()) {
            wifiNameItemLbl->setText( connectingStr +
                                bqUtils::truncateStringToLength(current_network.getName(),
                                                                WIFI_ITEM_SSID_MAX_LENGTH - connectingStr.length()));
        } else
            wifiNameItemLbl->setText(bqUtils::truncateStringToLength(current_network.getName(),WIFI_ITEM_SSID_MAX_LENGTH));


        if(current_network.isFavorite())
            wifiRemember->show();
        else
            wifiRemember->hide();

        wifiNameItemLbl->show();

        wifiConnect->hide();
        if(current_network.isConnected()){
            wifiConnect->show();
        }

        lockWifiLbl->hide();
        // FIXME: FER Check open networks
        if(current_network.getSecurity() != "none"){
            lockWifiLbl->show();
        }

        noSignalWifi->hide();
        signalWifi_1->hide();
        signalWifi_2->hide();
        signalWifi_3->hide();
        signalWifi_4->hide();

        switch(current_network.getLevel() * 5 / 100){
        case 0:
            signalWifi_1->setStyleSheet("background:url(:/res/wifi_0_Ico.png) no-repeat center left;");
            signalWifi_1->show();
            break;
        case 1:
            signalWifi_1->setStyleSheet("background:url(:/res/wifi_1_Ico.png) no-repeat center left;");
            signalWifi_1->show();
            break;
        case 2:
            signalWifi_1->setStyleSheet("background:url(:/res/wifi_2_Ico.png) no-repeat center left;");
            signalWifi_1->show();
            break;
        case 3:
            signalWifi_1->setStyleSheet("background:url(:/res/wifi_3_Ico.png) no-repeat center left;");
            signalWifi_1->show();
            break;
        case 4:
            signalWifi_1->setStyleSheet("background:url(:/res/wifi_4_Ico.png) no-repeat center left;");
            signalWifi_1->show();
            break;
        default:
            signalWifi_1->setStyleSheet("background:url(:/res/wifi_4_Ico.png) no-repeat center left;");
            signalWifi_1->show();
            break;
        }

}

void SettingsWiFiListItem::handleTap()
{
        qDebug() << Q_FUNC_INFO;
        if(wifiNameItemLbl->isVisible() && !QBook::settings().value("wifi/disabled",false).toBool())
            emit wifiSelected(current_network);
}

void SettingsWiFiListItem::processLongpress()
{
        qDebug() << Q_FUNC_INFO;
        if(wifiNameItemLbl->isVisible() && !QBook::settings().value("wifi/disabled",false).toBool())
            emit wifiLongPressed(current_network);
}

void SettingsWiFiListItem::hideLabels()
{
    lockWifiLbl->hide();
    wifiConnect->hide();
    signalWifi_1->hide();
    signalWifi_2->hide();
    signalWifi_3->hide();
    signalWifi_4->hide();
    wifiNameItemLbl->hide();
    wifiRemember->hide();
}

void SettingsWiFiListItem::paintEvent (QPaintEvent *)
{
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }

