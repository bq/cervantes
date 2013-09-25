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
#include <QDebug>
#include <QKeyEvent>
#include <QLabel>

#include "DeviceInfo.h"
#include "QBook.h"
#include "SettingsTechnicalInfo.h"
#include "Battery.h"
#include "Storage.h"
#include "Wifi.h"

SettingsTechnicalInfo::SettingsTechnicalInfo(QWidget *parent) : FullScreenWidget(parent)
{
        setupUi(this);
        connect(backBtn,SIGNAL(clicked()), this,SIGNAL(hideMe()));

        Storage *storage = Storage::getInstance();
        connect(storage, SIGNAL(partitionMounted(StoragePartition*)), this, SLOT(fillInfo()));
        connect(storage, SIGNAL(partitionUmounted(StoragePartition*)), this, SLOT(fillInfo()));

        battery = Battery::getInstance();
        connect(battery, SIGNAL(batteryLevelChanged(int)), this, SLOT(refreshBatteryLevel(int)));
}

SettingsTechnicalInfo::~SettingsTechnicalInfo()
{
}

void SettingsTechnicalInfo::keyReleaseEvent(QKeyEvent *event){

        qDebug() << "--->" << Q_FUNC_INFO;
        if( event->key() == QBook::QKEY_BACK )
        {
            qDebug() << ": Closing...";
            emit hideMe();
            event->accept();
            return;
        }
        QWidget::keyReleaseEvent(event);
}

void SettingsTechnicalInfo::fillInfo(){

    qDebug() << Q_FUNC_INFO;
    DeviceInfo *deviceInfo = DeviceInfo::getInstance();
    Storage *storageInfo = Storage::getInstance();
    Wifi *wifiInfo = Wifi::getInstance();

    MACaddressValLbl->setText(wifiInfo->getMacAddress());
    internalMemValLbl->setText(human_readable_filesize(storageInfo->getTotalInternalMemory(), 2));
    if (storageInfo->getRemovablePartition() && storageInfo->getRemovablePartition()->isMounted())
        externalMemValLbl->setText(human_readable_filesize(storageInfo->getTotalExternalMemory(), 2));
    else
        externalMemValLbl->setText("--");

    batteryValLbl->setText(QString::number(battery->getLevel()) + "%");

    QHash<QString, QString> extraInformation = deviceInfo->getSpecificDeviceInfo();

    qDebug() << extraInformation;

    QHash<QString, QString> specificInfo = deviceInfo->getSpecificDeviceInfo();

    internalVersionValLbl->setText(specificInfo.value("Internal Revision"));
    rootFsValLbl->setText(specificInfo.value("Static rootfs version")
                            + "/" + specificInfo.value("Actual rootfs version")
                            + "/" + specificInfo.value("Private repo version"));
    kernelVersionVaLbl->setText(specificInfo.value("Kernel version"));
    softwareValLbl->setText(specificInfo.value("Software version"));
    QString serialNum = deviceInfo->getSerialNumber();
    if(serialNum.startsWith("SN-")) // Remove prefix if present
        serialNum.remove(0,3);
    idDeviceValLbl->setText(serialNum);
}

void SettingsTechnicalInfo::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void SettingsTechnicalInfo::showEvent (QShowEvent * )
{
    qDebug() << Q_FUNC_INFO;
    fillInfo();
}

void SettingsTechnicalInfo::refreshBatteryLevel(int value)
{
    qDebug() << Q_FUNC_INFO;
    batteryValLbl->setText(QString::number(value) + "%");
}
