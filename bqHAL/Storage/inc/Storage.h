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

#ifndef STORAGE_H
#define STORAGE_H

#include <libudev.h>

#include <QString>
#include <QList>
#include <QSocketNotifier>
#include <QStringList>
#include <QMap>

#include "StoragePartition.h"
#include "StorageDevice.h"

#define human_readable_filesize(size, precision) \
        (size < (1 << 10)) \
    ? (QString::number(size, 'f',precision) + tr(" b")) \
        : (size < (1 << 20)) \
    ? (QString::number(static_cast<float>(size)/(1 << 10), 'f', precision) + tr(" KB")) \
        : (size < (1 << 30)) \
    ? (QString::number(static_cast<float>(size)/(1 << 20), 'f', precision) + tr(" MB") ) \
    : (QString::number(static_cast<float>(size)/(1 << 30), 'f', precision) + tr(" GB"))


class PowerManagerLock;

typedef struct{
    QString sysname;
    QString action;
} udevDevice;

class Storage : public QObject {

Q_OBJECT

public:
    static Storage* getInstance();
    static void staticInit();
    static void staticDone();
    const QList<StoragePartition*> getPartitions();
    const QList<StorageDevice*> getDevices();
    virtual QString getMountsPath() = 0;
    /** Get Free Internal Memory in bytes */
    virtual unsigned long long getFreeInternalMemory() = 0;
    /** Get Free Private Memory in bytes */
    virtual unsigned long long getFreePrivateMemory() = 0;
    /** Get Free External Memory in bytes */
    virtual unsigned long long getFreeExternalMemory() = 0;
    /** Get Total Internal Memory in bytes */
    virtual unsigned long long getTotalInternalMemory() = 0;
    /** Get Total Private Memory in bytes */
    virtual unsigned long long getTotalPrivateMemory() = 0;
    /** Get Total External Memory in bytes */
    virtual unsigned long long getTotalExternalMemory() = 0;
    virtual StoragePartition* getPrivatePartition() = 0;
    virtual StoragePartition* getPublicPartition() = 0;
    virtual StoragePartition* getRemovablePartition() = 0;
    virtual StoragePartition* getDataPartition() = 0;
    virtual bool startSharingOverUSB();
    virtual bool stopSharingOverUSB();
    virtual bool restartSharingOverUSB() = 0;
    virtual void init() = 0;
    virtual void initAppData() = 0;
    virtual void formatStoragePartition(StoragePartition *) = 0;
    virtual void umountStoragePartition(StoragePartition *);
    virtual void mountStoragePartition(StoragePartition *);

signals:
    void partitionMounted(StoragePartition*);
    void partitionMountFailed(StoragePartition*);
    void partitionUmounted(StoragePartition*);
    void partitionUmountFailed(StoragePartition*);
    void deviceInserted(StorageDevice*);
    void deviceRemoved(StorageDevice*);
    void deviceChanged(QString);
    void partitionCorrupted(StoragePartition *);
    void deviceUnknown(StorageDevice *);
    void sharingOverUSBEnded();

protected:
    Storage();
    virtual ~Storage();
    StoragePartition *createStoragePartition(StoragePartition::PartitionType, StoragePartition::FilesystemType, const QString& , const QString& , const QString& );
    StorageDevice *createStorageDevice(const QString& , const QString& , StorageDevice::Type);

    //QMap that contains:
    // · key = String returned by udev_device_get_devpath
    // · value = Pointer to StoragePartition.
    QMap<QString, StoragePartition*> _partitions;

    //QMap that contains:
    // · key = String returned by udev_device_get_devpath
    // · value = Pointer to StorageDevice.
    QMap<QString, StorageDevice*> _devices;
    PowerManagerLock* m_powerLock;

    StoragePartition* privatePartition;
    StoragePartition* publicPartition;
    StoragePartition* removablePartition;
    StoragePartition* dataPartition;

    bool usb_active;

private:
    static Storage* _instance;
    struct udev *udev;
    struct udev_monitor *udev_monitor;
    QSocketNotifier *notifier;
    int enumerate_mmcblk_devices(struct udev *udev);
    void processRemovableUdevEvents(const QString& , const QString& , const QString& , const QString& );
    void addUdevPartition(StoragePartition *partition, const QString& sysname);
    void removeUdevPartition(StoragePartition *);

private slots:
    void checkDevice(int);

};

#endif // STORAGE_H
