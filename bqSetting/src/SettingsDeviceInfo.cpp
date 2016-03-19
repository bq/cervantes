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

#include <QProgressBar>
#include <QDebug>
#include <QKeyEvent>
#include <QPainter>

#include "SettingsDeviceInfo.h"
#include "Settings.h"
#include "SelectionDialog.h"
#include "InfoDialog.h"
#include "SettingsLanguageMenu.h"
#include "SettingsDateTimeMenu.h"
#include "SettingsSleepTimeMenu.h"
#include "SettingsReaderMenu.h"
#include "SettingsUpdateDeviceAuto.h"
#include "SettingsTechnicalInfo.h"
#include "ConnectionManager.h"
#include "Battery.h"
#include "Storage.h"
#include "QBook.h"
#include "QBookApp.h"
#include "Model.h"
#include "DeviceInfo.h"
#include "FrontLight.h"

SettingsDeviceInfo::SettingsDeviceInfo(QWidget *parent) : FullScreenWidget(parent)
{
        setupUi(this);

        Storage *storage = Storage::getInstance();
        connect(storage, SIGNAL(partitionMounted(StoragePartition*)), this, SLOT(updateExternalMemoryInfo()));
        connect(storage, SIGNAL(partitionUmounted(StoragePartition*)), this, SLOT(updateExternalMemoryInfo()));

        /* Please, keep the same order than interface */

        connect(backBtn,SIGNAL(clicked()), this, SIGNAL(hideMe()));
        connect(basicInfoBtn, SIGNAL(clicked()), this, SLOT(showDeviceInfo()));
#ifndef HACKERS_EDITION
        connect(updateDeviceBtn, SIGNAL(clicked()), this, SLOT(updateDevice()));
#else
        updateDeviceBtn->hide();
#endif
	connect(restoreDefaulValuesBtn, SIGNAL(clicked()), this, SLOT(restoreDevice()));

        /**********************************************/

        upgradeMenu = new SettingsUpdateDeviceAuto(this);
        deviceInfo = new SettingsTechnicalInfo(this);
        battery = Battery::getInstance();
        connect(battery, SIGNAL(batteryLevelChanged(int)), this, SLOT(refreshBatteryLevel(int)));

        connect(deviceInfo, SIGNAL(hideMe()), this, SLOT(hideTopElement()));
        connect(upgradeMenu, SIGNAL(hideMe()), this, SLOT(hideTopElement()));
        connect(upgradeMenu, SIGNAL(hideMe()), this, SLOT(restoreOTACheck()));

        upgradeMenu->hide();
        deviceInfo->hide();

}

void SettingsDeviceInfo::setup()
{
    qDebug() << Q_FUNC_INFO;

    batteryProgressBar->setValue(battery->getLevel());
    batteryPercentValueLbl->setText(QString::number(battery->getLevel()) + "%");

    Storage *storage = Storage::getInstance();
    QString free = human_readable_filesize(storage->getFreeInternalMemory(), 2);
    QString text = human_readable_filesize(storage->getTotalInternalMemory(), 2);
    QString freePrivate = human_readable_filesize(storage->getFreePrivateMemory(), 2);

    freeSpaceValue->setText(free + " ");
    totalSpaceLbl->setText(text);
    privateFreeSpaceValueLbl->setText(freePrivate + " ");
    privateFreeTotalSpaceLbl->hide();

    updateExternalMemoryInfo();


}

SettingsDeviceInfo::~SettingsDeviceInfo()
{
        qDebug() << Q_FUNC_INFO;
}

void SettingsDeviceInfo::showUpgradeMenu()
{
        qDebug() << Q_FUNC_INFO;

        ((Settings*)parent())->showElement(upgradeMenu);
        upgradeMenu->doCheck();
}

void SettingsDeviceInfo::restoreOTACheck()
{
        qDebug() << Q_FUNC_INFO;
	connect(ConnectionManager::getInstance(), SIGNAL(connected()), QBookApp::instance(), SLOT(checkFwVersion()));
}

void SettingsDeviceInfo::restoreDevice()
{

    qDebug() << "--->" << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    SelectionDialog* confirmDialog = new SelectionDialog(this,tr("Do you want to restore device? This will delete all your personal account data"));
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
    confirmDialog->exec();
    bool result = confirmDialog->result();
    delete confirmDialog;
    if(!result)
        return;

    disconnect(Storage::getInstance(), SIGNAL(partitionMounted(StoragePartition*)), QBookApp::instance(), SLOT(handlePartitionMounted(StoragePartition*)));
    PowerManagerLock* powerLock = PowerManager::getNewLock(this);
    powerLock->activate();
    disconnect(Battery::getInstance(), SIGNAL(batteryLevelFull()), QBookApp::instance(),SLOT(handleFullBattery()));
    QBookApp::instance()->cancelSync();

    InfoDialog *dialog = new InfoDialog(this,tr("This process can take a few minutes. At the end, the device will be powered off"));
    dialog->show();
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);

    QBookApp::instance()->disableUserEvents();
    QBookApp::instance()->getSyncHelper()->SetServerTimestamp(0);
    QBookApp::instance()->showRestoringImage();
    QBookApp::instance()->syncModel();

    Screen::getInstance()->queueUpdates();

    if(DeviceInfo::getInstance()->hasFrontLight() && FrontLight::getInstance()->isFrontLightActive())
        FrontLight::getInstance()->switchFrontLight(false);

    // Remove private books from model
    QBookApp::instance()->getModel()->removeDir(Storage::getInstance()->getPrivatePartition()->getMountPoint());

        QFuture<void> future = QtConcurrent::run(this, &SettingsDeviceInfo::restoreSettinsAndPartitions, dialog);
        if (!future.isFinished())
            dialog->exec();

    PowerManager::powerOffDevice(true);

}

void SettingsDeviceInfo::restoreSettinsAndPartitions(QDialog *dialog)
{
    qDebug() << Q_FUNC_INFO <<  ": Format private partition";

    // Clear data partition (except dictionary files)
    QString cmd = "rm -r `find " + Storage::getInstance()->getDataPartition()->getMountPoint() + "/* | grep -v dictionaries`";
    system(cmd.toAscii().constData());

    // Restore connman dir on data partition, as it comes from factory
    QDir(Storage::getInstance()->getDataPartition()->getMountPoint()).mkpath("connman");

    // Format private partition
    Storage *storage = Storage::getInstance();
    storage->formatStoragePartition(storage->getPrivatePartition());

    if (dialog)
        dialog->accept();
}

void SettingsDeviceInfo::updateDevice()
{

        qDebug() << Q_FUNC_INFO;

        if(ConnectionManager::getInstance()->isConnected()) {
		// Disconnect QBookApp autocheck on connect
		qDebug() << Q_FUNC_INFO << "Entering upgrade menu, disconnecting ota checks";
		disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), QBookApp::instance(), SLOT(checkFwVersion()));
                showUpgradeMenu();
        } else{
    #ifndef FAKE_WIFI
                QBookApp::instance()->requestConnection(true);
                connectWifiObserverUpgrade();
                connect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disconnectWifiObserverUpgrade()));
    #endif
        }

}

void SettingsDeviceInfo::connectWifiObserverUpgrade()
{
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(showUpgradeMenu()),Qt::UniqueConnection);
}

void SettingsDeviceInfo::disconnectWifiObserverUpgrade()
{
    disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(showUpgradeMenu()));
}

void SettingsDeviceInfo::showDeviceInfo()
{
        qDebug() << Q_FUNC_INFO;
        ((Settings*)parent())->showElement(deviceInfo);

}


/* virtual */ void SettingsDeviceInfo::keyReleaseEvent(QKeyEvent *event)
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

void SettingsDeviceInfo::updateExternalMemoryInfo()
{
        qDebug() << Q_FUNC_INFO;
        Storage *storage = Storage::getInstance();
        StoragePartition *removablePartition = storage->getRemovablePartition();
        if (!removablePartition || (removablePartition && !removablePartition->isMounted()))
        {
            qDebug() << Q_FUNC_INFO << ": removablePartition is not mounted";
            sdFreeSpaceLbl->hide();
            sdFreeSpaceValueLbl->setText("");
            sdTotalSpaceLbl->setText("");
            sdFreeSpaceValueLbl->setEnabled(false);
            sdTotalSpaceLbl->setEnabled(false);
        }
        else{

            unsigned long long freeExternalMemory = storage->getFreeExternalMemory();
            unsigned long long totalExternalMemory = storage->getTotalExternalMemory();
            qDebug() << Q_FUNC_INFO << ": free " << freeExternalMemory << " total " << totalExternalMemory;

            QString free = human_readable_filesize(freeExternalMemory, 2);
            QString total = human_readable_filesize(totalExternalMemory, 2);
            sdFreeSpaceLbl->show();
            sdFreeSpaceValueLbl->setText(free + " ");
            sdTotalSpaceLbl->setText(total);
            sdFreeSpaceValueLbl->setEnabled(true);
            sdTotalSpaceLbl->setEnabled(true);
        }
}

void SettingsDeviceInfo::hideTopElement()
{
        qDebug() << Q_FUNC_INFO;
        Screen::getInstance()->queueUpdates();
        setup();
        ((Settings *)parent())->hideElement();
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
}

void SettingsDeviceInfo::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }

void SettingsDeviceInfo::showEvent (QShowEvent * )
{
    qDebug() << Q_FUNC_INFO;
    setup();
}

void SettingsDeviceInfo::refreshBatteryLevel(int value)
{
    qDebug() << Q_FUNC_INFO;
    batteryProgressBar->setValue(value);
    batteryPercentValueLbl->setText(QString::number(value) + "%");
}
