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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <QObject>
#include <QDebug>
#include <QDir>
#include <QStringList>
#include <Storage.h>
#include <StoragePartition.h>
#include <QFileInfo>
#include <StorageEmu.h>

StorageEmu::StorageEmu()
    : Storage()
{
    qDebug() << "StorageEmu::StorageEmu\n";
	QStringList paths;
	paths << "mnt/private" << "mnt/public" << "mnt/data" << "mnt/sd";
	QDir dir = QDir("x86-bin");
	foreach (QString path, paths) {
		qDebug() << "Creating emulator fake partition " << path;
		dir.mkpath(path);
	}
}

void StorageEmu::init() {
        _devices.insert("none_removable", createStorageDevice ("removable", "none", StorageDevice::REMOVABLE));

	privatePartition = createStoragePartition (StoragePartition::PRIVATE, StoragePartition::FS_EXT3,
                                                   "private", "none", "x86-bin/mnt/private");
	privatePartition->setMounted(true);

	publicPartition = createStoragePartition (StoragePartition::PUBLIC, StoragePartition::FS_VFAT,
                                                   "public", "none", "x86-bin/mnt/public");
	publicPartition->setMounted(true);


        _partitions.insert("private", privatePartition);
        _partitions.insert("public", publicPartition);

        removablePartition = createStoragePartition (StoragePartition::REMOVABLE, StoragePartition::FS_VFAT,
                                                   "removable", "none", "x86-bin/mnt/sd");
        removablePartition->setMounted(true);
        _partitions.insert("removable", removablePartition);
}

void StorageEmu::initAppData(){
    qDebug() << "--->" << Q_FUNC_INFO;

    dataPartition = createStoragePartition (StoragePartition::DATA, StoragePartition::FS_EXT3,
                                               "data", "none", "x86-bin/mnt/data");
    dataPartition->setMounted(true);

    _partitions.insert("data", dataPartition);
}

unsigned long long StorageEmu::getFreeInternalMemory() {
	return 1000000000;
}

unsigned long long StorageEmu::getFreePrivateMemory() {
    return 1000000000;
}

unsigned long long StorageEmu::getFreeExternalMemory() {
	return 1000000000;
}

unsigned long long StorageEmu::getTotalInternalMemory() {
	return 2000000000;
}
unsigned long long StorageEmu::getTotalPrivateMemory() {
    return 2000000000;
}
unsigned long long StorageEmu::getTotalExternalMemory() {
	return 2000000000;
}


QString StorageEmu::getMountsPath() {
	return QString("/x86-binmntmmc");
}
	
StoragePartition* StorageEmu::getPrivatePartition() {
	return privatePartition;
}
StoragePartition* StorageEmu::getPublicPartition() {
	return publicPartition;
}
StoragePartition* StorageEmu::getRemovablePartition() {
	return removablePartition;
}

StoragePartition* StorageEmu::getDataPartition() {
        return dataPartition;
}

bool StorageEmu::startSharingOverUSB() {
	return true;
}

bool StorageEmu::stopSharingOverUSB() {
	return true;
}

bool StorageEmu::restartSharingOverUSB() {
    return true;
}

void StorageEmu::umountStoragePartition(StoragePartition *partition)
{
    if (!partition)
	return;

    partition->setMounted(false);
    emit partitionUmounted(partition);
}

void StorageEmu::mountStoragePartition(StoragePartition *partition)
{
    if (!partition)
	return;

    partition->setMounted(true);
    emit partitionMounted(partition);
}

void StorageEmu::formatStoragePartition(StoragePartition *partition)
{
    if (!partition)
        return;

    QString path = partition->getMountPoint() + QDir::separator();
    QFileInfo file(path);
    QString absolutePath = file.absolutePath() + QDir::separator();
    QString rmCommand = QString("rm -rf ") + absolutePath;
    QString mkCommand = QString("mkdir ") + absolutePath;

    switch(partition->getPartitionType())
    {
        case StoragePartition::PRIVATE:
        case StoragePartition::PUBLIC:
            system(rmCommand.toAscii().data());
            break;
        default:
            qDebug() << Q_FUNC_INFO << ": this partition type not support format! " << partition->getPartitionType();
            return;
            break;
    }
    system(mkCommand.toAscii().data());

}



