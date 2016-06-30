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

#include "Storage.h"
#include "StoragePartition.h"
#include "StorageMx508.h"

#include "QBook.h"
#include "PowerManager.h"
#include "DeviceInfo.h"

#include <unistd.h>
#include <QDebug>
#include <QList>
#include <QString>
#include <QDir>
#include <QEventLoop>
#include <QTimer>

void listOpenedFd()
{
    qDebug() << Q_FUNC_INFO;

    pid_t pid = getpid();
    qDebug() << Q_FUNC_INFO << pid;

    QString path("/proc/");
    path.append(QString::number(pid));
    path.append("/fd");

    QDir pidDir(path);

    qDebug() << Q_FUNC_INFO << path;

    QStringList list = pidDir.entryList();

    QStringList::const_iterator it = list.begin();
    QStringList::const_iterator itEnd = list.end();

    QFileInfo fi;
    for(; it != itEnd; ++it)
    {
        QString file_path = pidDir.filePath(*it);
        fi.setFile(file_path);

        qDebug() << Q_FUNC_INFO << fi.isSymLink() << fi.symLinkTarget();
    }
}

StorageMx508::StorageMx508  ()
    : Storage()
{
    qDebug() << Q_FUNC_INFO;
    m_powerLock = PowerManager::getNewLock(this);
}

StorageMx508::~StorageMx508()
{
    qDebug() << Q_FUNC_INFO;

    if(m_powerLock)
        delete m_powerLock;
    m_powerLock = NULL;
}

void StorageMx508::init (){
    qDebug() << Q_FUNC_INFO;

    QString publicBlock;
    QString privateBlock;
    int hwid = DeviceInfo::getInstance()->getHwId();
    if(hwid == DeviceInfo::E60Q22 || hwid == DeviceInfo::E60QH2)
    {
        publicBlock = "mmcblk0p4";
        privateBlock = "mmcblk0p7";
    }
    else // E606A2 and E60672
    {
        publicBlock = "mmcblk0p7";
        privateBlock = "mmcblk0p5";
    }


    privatePartition = createStoragePartition (StoragePartition::PRIVATE, StoragePartition::FS_EXT3,
                                               "/block/mmcblk0/" + privateBlock , "/dev/" + privateBlock, "/mnt/private");

    publicPartition = createStoragePartition (StoragePartition::PUBLIC, StoragePartition::FS_VFAT,
                                               "/block/mmcblk0/" + publicBlock, "/dev/" + publicBlock, "/mnt/public");

    _partitions.insert("/block/mmcblk0/" + publicBlock, publicPartition);
    _partitions.insert("/block/mmcblk0/" + privateBlock, privatePartition);

    publicPartition->mount();
    privatePartition->mount();

    // Hide ADEPT_ACTIVATION_DIR folder
    QFile mtoolsrcFile("/root/.mtoolsrc");// TODO: is there another way not to used this hard-coded path?
    if(!mtoolsrcFile.exists())
    {
        qDebug() << Q_FUNC_INFO << mtoolsrcFile.fileName();
        if(mtoolsrcFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            qDebug() << Q_FUNC_INFO << "Opened";
            mtoolsrcFile.write(QString("drive C: file=\"/dev/" + publicBlock + "\"\n").toStdString().c_str());
            mtoolsrcFile.write("mtools_skip_check=1\n");

            mtoolsrcFile.close();
            sync();

            system("mattrib +h C:/.adobe-digital-editions");
        }
    }

    system("cat /root/.mtoolsrc");// Debugging // TODO: is there another way not to used this hard-coded path?
}

void StorageMx508::initAppData(){
    qDebug() << Q_FUNC_INFO;

    dataPartition = createStoragePartition (StoragePartition::DATA, StoragePartition::FS_EXT3,
                                               "/block/mmcblk0/mmcblk0p6", "/dev/mmcblk0p6", "/mnt/data");
    _partitions.insert("/block/mmcblk0/mmcblk0p6", dataPartition);
    dataPartition->mount();
}

unsigned long long StorageMx508::getFreeInternalMemory()
{
    return publicPartition->getFreeSpace();
}

unsigned long long StorageMx508::getFreePrivateMemory()
{
    return privatePartition->getFreeSpace();
}

unsigned long long StorageMx508::getFreeExternalMemory() {
    if (removablePartition)
        return removablePartition->getFreeSpace();
    else
        return 0;
}

unsigned long long StorageMx508::getTotalInternalMemory()
{
    return publicPartition->getTotalSpace();
}

unsigned long long StorageMx508::getTotalPrivateMemory()
{
    return privatePartition->getTotalSpace();
}

unsigned long long StorageMx508::getTotalExternalMemory()
{
    if (removablePartition)
        return removablePartition->getTotalSpace();
    else
        return 0;
}

QString StorageMx508::getMountsPath() {
        return QString("/mnt");
}

StoragePartition* StorageMx508::getPrivatePartition() {
        return privatePartition;
}
StoragePartition* StorageMx508::getPublicPartition() {
        return publicPartition;
}
StoragePartition* StorageMx508::getRemovablePartition() {
        return removablePartition;
}
StoragePartition* StorageMx508::getDataPartition() {
        return dataPartition;
}

bool StorageMx508::startSharingOverUSB() {
        qDebug() << Q_FUNC_INFO;

        //Block sleep        
        m_powerLock->activate();

        Storage::startSharingOverUSB();

        system("rmmod g_zero");
        system("rmmod g_file_storage");

	sharedDevices.clear();

    QString command = QString("modprobe g_file_storage stall=0 file=" + publicPartition->getDevice());
    if (DeviceInfo::getInstance()->getHwId() == DeviceInfo::E60QH2){
        command = QString("modprobe g_file_storage stall=0 vendor=0x2A47 product=0xAD78 file=" + publicPartition->getDevice());
    }

	sharedDevices << publicPartition->getDevice();

        for (QMap<QString, StorageDevice*>::iterator it = _devices.begin(); it != _devices.end(); ++it)
        {
            StorageDevice *dev = it.value();
            if (dev->getType() == StorageDevice::REMOVABLE && dev->isInserted() && removablePartition)
	    {
                command += "," + removablePartition->getDevice();
		sharedDevices << removablePartition->getDevice();
	    }
        }

        command += " removable=1";
        system(command.toAscii().data());

        //TODO: Netronix think we need wait 0.5 seconds here.... ¿?
        // Added waiting loop to avoid white screen when going to sleep
        QEventLoop waitingLoop;
        QTimer::singleShot(500, &waitingLoop, SLOT(quit()));
        waitingLoop.exec();

        if (removablePartition && removablePartition->isMounted())
                if(removablePartition->umount())
                    emit partitionUmounted(removablePartition);
        if (publicPartition->isMounted())
                if(publicPartition->umount())
                    emit partitionUmounted(publicPartition);

	connect(this, SIGNAL(deviceChanged(QString)), this, SLOT(deviceUnshared(QString)));

        // FIXME: We should check for errors
        return true;
}


bool StorageMx508::stopSharingOverUSB() {
        qDebug() << Q_FUNC_INFO;
        sync();

        system("rmmod g_file_storage");

        publicPartition->checkFS();
        if (publicPartition->isCorrupted())
            emit partitionCorrupted(publicPartition);
        else if (publicPartition->mount())
                emit partitionMounted(publicPartition);

        for (QMap<QString, StorageDevice*>::iterator it = _devices.begin(); it != _devices.end(); ++it)
        {
            if (it.value()->getType() == StorageDevice::REMOVABLE && it.value()->isInserted())
            {
                if (removablePartition->isMounted())
                    removablePartition->umount();

                removablePartition->checkFS();
                if(removablePartition->isCorrupted())
                    emit partitionCorrupted(removablePartition);
                else if (removablePartition->mount())
                    emit partitionMounted(removablePartition);
            }
        }

        Storage::stopSharingOverUSB();

        m_powerLock->release();

        // FIXME: check for errors here
        return true;
}

bool StorageMx508::restartSharingOverUSB() {
        system("rmmod g_file_storage");

        QString command = QString("modprobe g_file_storage stall=0 file=" + publicPartition->getDevice());

        for (QMap<QString, StorageDevice*>::iterator it = _devices.begin(); it != _devices.end(); ++it)
        {
            StorageDevice *dev = it.value();
            if (dev->getType() == StorageDevice::REMOVABLE && dev->isInserted() && removablePartition)
                command += "," + removablePartition->getDevice();
        }

        command += " removable=1";
        system(command.toAscii().data());

        //TODO: Netronix think we need wait 0.5 seconds here.... ¿?
        // Added waiting loop to avoid white screen when going to sleep
        QEventLoop waitingLoop;
        QTimer::singleShot(500, &waitingLoop, SLOT(quit()));
        waitingLoop.exec();

        return true;
}

void StorageMx508::deviceUnshared(QString path)
{
    qDebug() << "device: " << path;
    if (sharedDevices.contains(path)) {
        qDebug() << Q_FUNC_INFO << path << " unshared, removing from sharedDevices list";
        sharedDevices.removeOne(path);
        if (sharedDevices.isEmpty()) {
             qDebug() << Q_FUNC_INFO << "All shared devices have been unshared, emitting sharingOverUSBEnded";
             emit sharingOverUSBEnded();
        }
    } else {
        qDebug() << Q_FUNC_INFO << path << " unshared, but it wasn't in the sharedDevices list. This should not happen!";
    }
}


void StorageMx508::formatStoragePartition(StoragePartition *partition)
{
    qDebug() << "partition: " << partition->getPartitionType();
    if (!partition)
        return;

    // Partition has to be umounted
    if (partition->umount())
    {
        QString label;
        switch(partition->getPartitionType())
        {
            case StoragePartition::PRIVATE:
                label = QBook::settings().value("readerPartitionName", "reader").toString();
                break;
            case StoragePartition::PUBLIC:
                label = "public";
                break;
            case StoragePartition::REMOVABLE:
                label = "removable";
                break;
            default:
                qDebug() << Q_FUNC_INFO << ": this partition type not support format! " << partition->getPartitionType();
                return;
                break;
        }

        bool result = partition->format(label);
        if (!result)
        {
            qDebug() << Q_FUNC_INFO << "Error formating partition. Removing all files manually";
            partition->mount();
            partition->removeDir(partition->getMountPoint());
        }

        partition->checkFS();

    }
    else
    {
        qDebug() << Q_FUNC_INFO << ": Error umounting partition!! mounted partition cannot formated. Trying to rm -rf /";
        partition->removeDir(partition->getMountPoint());
    }
}

