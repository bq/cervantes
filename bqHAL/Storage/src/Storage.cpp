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

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include <QCoreApplication>
#include <QDebug>
#include <Storage.h>
#include <StoragePartition.h>
#include <StorageDevice.h>
#include <QBook.h>
#include "PowerManager.h"
#include "PowerManagerLock.h"
#include <QFile>

#if MX508
#include "StorageMx508.h"
#else

#include "StorageEmu.h"
#endif

#define UDEV_MONITOR_BUFFER_SIZE_STORAGE 128*1024*1024

Storage* Storage::_instance = NULL;

Storage* Storage::getInstance()
{
    return _instance;
}

void Storage::staticInit()
{
    if (_instance == NULL)
    {
#ifndef Q_WS_QWS
            _instance = new StorageEmu();
#elif MX508
            _instance = new StorageMx508();
#endif
    }
}

void Storage::staticDone()
{
    if(_instance)
    {
        delete _instance;
        _instance = NULL;
    }
}

const QList<StoragePartition*> Storage::getPartitions() {
    return (const QList<StoragePartition*>) _partitions.values();
}

const QList<StorageDevice*> Storage::getDevices() {
        return (const QList<StorageDevice*>) _devices.values ();
}

/*virtual*/ void Storage::umountStoragePartition(StoragePartition *partition)
{
    if (partition)
        if (partition->umount())
            emit partitionUmounted(partition);
}

/*virtual*/ void Storage::mountStoragePartition(StoragePartition *partition)
{
    if (partition)
    {
        if (partition->mount())
        {
            emit partitionMounted(partition);
        }
        else
        {
            emit partitionUmounted(partition);
        }
    }
}

Storage::Storage()
    : privatePartition(NULL)
    , publicPartition(NULL)
    , removablePartition(NULL)
    , dataPartition(NULL)
    , usb_active(false)
{
        qDebug() << Q_FUNC_INFO;
        _partitions = QMap<QString, StoragePartition*>();
        _devices = QMap<QString, StorageDevice*>();

        udev = udev_new();
        if (!udev) {
                qDebug() << Q_FUNC_INFO << "Cannot create udev";
                return;
        }

        enumerate_mmcblk_devices(udev);

        udev_monitor = udev_monitor_new_from_netlink(udev, "udev");
        if (!udev_monitor) {
                qDebug() << Q_FUNC_INFO << "Cannot create mon";
                return;
        }

        if (udev_monitor_filter_add_match_subsystem_devtype(udev_monitor, "block", NULL) != 0) {
                qDebug() << Q_FUNC_INFO << "Error with udev_monitor_filter_add_match_subsystem_devtype";
                return;
        }

        udev_monitor_set_receive_buffer_size(udev_monitor, UDEV_MONITOR_BUFFER_SIZE_STORAGE);

        if (udev_monitor_enable_receiving(udev_monitor) < 0) {
                qDebug() << Q_FUNC_INFO << "Error, cannot receive from kernel";
                return;
        }
        int fd = udev_monitor_get_fd(udev_monitor);
        if (fd <= 0) {
                qDebug() << Q_FUNC_INFO << "Error at udev_monitor_get_fd";
                return;
        }

        notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
        notifier->setEnabled(true);
        connect(notifier, SIGNAL(activated(int)), this, SLOT(checkDevice(int)));
}

StoragePartition* Storage::createStoragePartition(StoragePartition::PartitionType partitionType,
                                                  StoragePartition::FilesystemType fsType,
                                                  const QString& udevPath,
                                                  const QString& device,
                                                  const QString& mountPoint) {
        StoragePartition *partition = new StoragePartition(partitionType,fsType,udevPath,device,mountPoint);
        if (!partition) {
                qDebug() << Q_FUNC_INFO << "Cannot create partition object";
                return NULL;
        }
        return partition;
}

StorageDevice* Storage::createStorageDevice(const QString& udevPath, const QString& device, StorageDevice::Type type) {
        StorageDevice *dev = new StorageDevice(udevPath, device, type);
        if (!dev)
                return NULL;
        return dev;
}

void Storage::checkDevice(int fd) {
    PowerManagerLock* powerLock = PowerManager::getNewLock(this);
    powerLock->activate();
    qDebug() << Q_FUNC_INFO << "FD: "<< fd;

    struct udev_device *dev;
    dev = udev_monitor_receive_device(udev_monitor);
    if (dev)
    {
        QString device = udev_device_get_devnode(dev);
        QString path = udev_device_get_devpath(dev);
        QString action = udev_device_get_action(dev);
        QString devtype = udev_device_get_devtype(dev);
        QString sysname("/dev/");
        sysname += QString(udev_device_get_sysname(dev));
        QString parent = udev_device_get_devpath( udev_device_get_parent (dev));
        if (sysname.contains(QRegExp("mmcblk[0-9]\\d*"))  || sysname.contains(QRegExp("mmcblk[0-9]")))
        {
            qDebug() << "Got " << path << ":" << action;
            qDebug() << "    Got Device";
            qDebug() << "    Node: " <<  udev_device_get_devnode(dev);
            qDebug() << "    Subsystem: "<< udev_device_get_subsystem(dev);
            qDebug() << "    Devtype: " << devtype;
            qDebug() << "    Action: " << action;
            qDebug() << "    Sysname: " << sysname;
            qDebug() << "    sysnum: " << udev_device_get_sysnum (dev);
            qDebug() << "    parent: " << parent;

            processRemovableUdevEvents(devtype, path, sysname, action);
        }
        udev_device_unref(dev);
    }
    delete powerLock;
}



Storage::~Storage()
{
    qDebug() << Q_FUNC_INFO;
    foreach(StorageDevice *storageDev, _devices) {
        delete storageDev;
    }
    _devices.clear();

    foreach(StoragePartition *partition, _partitions) {
        delete partition;
    }
    _partitions.clear();

    privatePartition = NULL;
    publicPartition = NULL;
    removablePartition = NULL;
    dataPartition = NULL;

    delete notifier;
    notifier = NULL;

    udev_monitor_unref(udev_monitor);
    udev_unref(udev);
}

int Storage::enumerate_mmcblk_devices(struct udev *udev)
{
    struct udev_enumerate *udev_enumerate;

    qDebug() << Q_FUNC_INFO << ": enumerate 'block' devices";
    udev_enumerate = udev_enumerate_new(udev);
    if (udev_enumerate == NULL)
        return -1;
    udev_enumerate_add_match_subsystem(udev_enumerate,"block");
    udev_enumerate_scan_devices(udev_enumerate);
    struct udev_list_entry *list_entry;

    udev_list_entry_foreach(list_entry, udev_enumerate_get_list_entry(udev_enumerate))
    {
        struct udev_device *device;

        device = udev_device_new_from_syspath(udev_enumerate_get_udev(udev_enumerate),
                                      udev_list_entry_get_name(list_entry));
        if (device != NULL) {
            QString sysname("/dev/");
            sysname += QString(udev_device_get_sysname(device));
            if (sysname.contains(QRegExp("mmcblk[1-9]\\d*"))  || sysname.contains(QRegExp("mmcblk[1-9]")))
            {
                QString path = udev_device_get_devpath(device);
                QString parent = udev_device_get_devpath( udev_device_get_parent (device));
                QString action = "add";
                QString devtype = udev_device_get_devtype(device);

                qDebug() << "Enumerate device " << path << ":" << action;
                qDebug() << "    Got Device";
                qDebug() << "    Node: " <<  udev_device_get_devnode(device);
                qDebug() << "    Subsystem: "<< udev_device_get_subsystem(device);
                qDebug() << "    Devtype: " << devtype;
                qDebug() << "    Action: " << action;
                qDebug() << "    Sysname: " << sysname;
                qDebug() << "    sysnum: " << udev_device_get_sysnum (device);
                qDebug() << "    parent: " << parent;

                processRemovableUdevEvents(devtype, path, sysname, action);
            }

            udev_device_unref(device);
        }
    }

    udev_enumerate_unref(udev_enumerate);
    return 0;
}

void Storage::processRemovableUdevEvents(const QString& devtype, const QString& path, const QString& sysname, const QString& action)
{
    qDebug() << Q_FUNC_INFO << ": devtype = " << devtype << ", path = " << path << ", sysname = " << sysname << ", action = " << action;
    if (devtype == "disk")      // device
    {
        QMap<QString, StorageDevice*>::iterator it = _devices.find(path);

        if (action == "remove")
        {
            // QMap::find returns an iterator pointing to the item with key key in the map.
            // If the map contains no item with key key, the function returns end().
            if ( it != _devices.end())
            {
                // delete device
                emit deviceRemoved(it.value());
                _devices.remove(it.key());
                delete it.value();
            }
            // else if devices is not into _devices, maybe because we lost the "add" action...

            if (usb_active)
                restartSharingOverUSB();
        }
        else if (action == "add")
        {
            // QMap::find returns an iterator pointing to the item with key key in the map.
            // If the map contains no item with key key, the function returns end().
            if ( it == _devices.end())
            {
                // insert a new device
                // is a block device
                it = _devices.insert(path, createStorageDevice (path, sysname, StorageDevice::REMOVABLE));
            }
            // add new device
            it.value()->setInserted(true);
            emit deviceInserted(it.value());
        }
        else
        {
            qDebug() << Q_FUNC_INFO << ": Unknown device action " << action << ". Discarted device";
            if ( it != _devices.end())
            {
                _devices.remove(it.key());
                delete it.value();
            }
        }

        return;
    }

    if (devtype == "partition")
    {
        QMap<QString, StoragePartition*>::iterator it = _partitions.find(path);

        if (action == "add")
        {
            // QMap::find returns an iterator pointing to the item with key key in the map.
            // If the map contains no item with key key, the function returns end().
            if (it == _partitions.end())
            {
                it = _partitions.insert(path, createStoragePartition (StoragePartition::REMOVABLE, StoragePartition::FS_VFAT,
                                                                 path, sysname , "/mnt/sd"));
            }
            removablePartition = it.value();
            mountStoragePartition(removablePartition);
        }
        else if (action == "remove")
        {
            // QMap::find returns an iterator pointing to the item with key key in the map.
            // If the map contains no item with key key, the function returns end().
            if (it != _partitions.end())
            {
                umountStoragePartition(it.value());
                if (removablePartition == it.value())
                {
                    qDebug() << Q_FUNC_INFO << ": Cleaning removable partition";
                    removablePartition = NULL;
                }
                _partitions.remove(it.key());
                it.value()->deleteLater();
            }
            // else if partition is not into _partitions, maybe because we lost the "add" action...
        }
        else if (action == "change")
        {
        	qDebug() << Q_FUNC_INFO << ": got a change action for a partition, emitting deviceChanged";
            	emit deviceChanged(sysname);
            	// NOTE: We used to insert here too the removable partition, but that should be covered by the add
        }
        else
        {
            qDebug() << Q_FUNC_INFO << ": action " << action << " not supported for partitions. Discarted partition";
            if (it != _partitions.end())
            {
                _partitions.remove(it.key());
                delete (it.value());
            }
        }

        return;
    }
}

void Storage::addUdevPartition(StoragePartition *partition, const QString& sysname)
{
    partition->setDevice(sysname);
    if (partition->isMounted())
        partition->umount();

    partition->checkFS();
    if (partition->isCorrupted())
    {
        emit partitionCorrupted(partition);
    }
    else
    {
	/* Check first that device already exists */
	while (!QFile::exists(partition->getDevice())) {
		qDebug() << "Device " << partition->getDevice() << " does not exists yet, waiting before trying to mount it.";
		QCoreApplication::processEvents();
	}
        bool res = partition->mount();
        if (res != false)
        {
            qDebug() << Q_FUNC_INFO << ": Partition " << partition->getDevice() << " mounted successfully at " << partition->getMountPoint();
            emit partitionMounted(partition);
        }
        else
        {
            qDebug() << Q_FUNC_INFO << ": Partition " << partition->getDevice() << " mount failed at " << partition->getMountPoint();
            emit partitionMountFailed(partition);
        }
    }
}

void Storage::removeUdevPartition(StoragePartition *partition)
{
    bool res = partition->umount();
    if (res != false)
    {
        qDebug() << Q_FUNC_INFO << ": Partition " << partition->getDevice() << " umounted successfully at " << partition->getMountPoint();
        emit partitionUmounted(partition);
    }
    else
    {
        qDebug() << Q_FUNC_INFO << ": Partition " << partition->getDevice() << " umount failed at " << partition->getMountPoint();
        emit partitionUmountFailed(partition);
    }
}

bool Storage::startSharingOverUSB()
{
    usb_active = true;
    return true;
}

bool Storage::stopSharingOverUSB()
{
    usb_active = false;
    return true;
}
