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

#include "DeviceInfoDialog.h"
#include "DeviceInfo.h"
#include "Battery.h"
#include "Storage.h"
#include "Screen.h"
#include "QBook.h"
#include "Wifi.h"
#include <QNetworkInterface>
#include <QDebug>
#include <QPainter>

DeviceInfoDialog::DeviceInfoDialog(QWidget *parent) : QDialog(parent, Qt::Popup | Qt:: Dialog)

{	
        qDebug() << Q_FUNC_INFO;
        setupUi(this);
        connect(okButton, SIGNAL(clicked()), this, SLOT(handleOk()));

        QFile fileSpecific(":/res/qbookapp_styles.qss");
        QFile fileCommons(":/res/qbookapp_styles_generic.qss");
        fileSpecific.open(QFile::ReadOnly);
        fileCommons.open(QFile::ReadOnly);

        QString styles = QLatin1String(fileSpecific.readAll() + fileCommons.readAll());
        setStyleSheet(styles);

        fillInfo();

}

DeviceInfoDialog::~DeviceInfoDialog()
{
        qDebug() << Q_FUNC_INFO;
}

void DeviceInfoDialog::handleOk(void)
{
        qDebug() << Q_FUNC_INFO;

        this->accept();
        Screen::getInstance()->refreshScreen();
}

void DeviceInfoDialog::fillInfo()
{
        qDebug() << Q_FUNC_INFO;
        DeviceInfo *deviceInfo = DeviceInfo::getInstance();
        Battery *batteryInfo = Battery::getInstance();
        Storage *storageInfo = Storage::getInstance();
        Wifi *wifiInfo = Wifi::getInstance();

        MACaddressValLbl->setText(wifiInfo->getMacAddress());
        internalMemValLbl->setText(human_readable_filesize(storageInfo->getTotalInternalMemory(), 2));
        if (storageInfo->getRemovablePartition() && storageInfo->getRemovablePartition()->isMounted())
            externalMemValLbl->setText(human_readable_filesize(storageInfo->getTotalExternalMemory(), 2));
        else
            externalMemValLbl->setText("--");

        batteryValLbl->setText(QString::number(batteryInfo->getLevel()) + "%");

        QHash<QString, QString> extraInformation = deviceInfo->getSpecificDeviceInfo();

        qDebug() << extraInformation;

        QHash<QString, QString> specificInfo = deviceInfo->getSpecificDeviceInfo();

        internalVersionValLbl->setText(specificInfo.value("Internal Revision"));
        rootFsValLbl->setText(specificInfo.value("Static rootfs version") + "/" + specificInfo.value("Actual rootfs version"));
        kernelVersionVaLbl->setText(specificInfo.value("Kernel version"));
        softwareValLbl->setText(specificInfo.value("Software version"));
        QString serialNum = deviceInfo->getSerialNumber();
        if(serialNum.startsWith("SN-")) // Remove prefix if present
            serialNum.remove(0,3);
        idDeviceValLbl->setText(serialNum);
}

void DeviceInfoDialog::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }
