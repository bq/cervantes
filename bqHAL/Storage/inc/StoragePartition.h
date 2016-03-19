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

#ifndef __STORAGEPARTITION_H__
#define __STORAGEPARTITION_H__

#include <QString>
#include <QList>
#include <QObject>

class StoragePartition : public QObject
{

friend class Storage;
#if MX508
friend class StorageMx508;
#endif
#ifndef Q_WS_QWS
friend class StorageEmu;
#endif
    Q_OBJECT

public:
    enum FilesystemType { FS_VFAT = 0, FS_EXT3 = 1 };
    enum PartitionType { PUBLIC = 0, PRIVATE = 1, REMOVABLE = 2, DATA = 3 };
    const QString & getMountPoint() const;
    void setVolumeLabel(const QString&);
    const QString & getDevice() const;
    void setDevice(const QString& );
    const QString & getUdevPath() const;
    unsigned long long getFreeSpace() const;
    unsigned long long getTotalSpace() const;
    FilesystemType getFilesystemType() const;
    PartitionType getPartitionType() const;
    bool isMounted() const;
    bool isCorrupted() const;
    /** check if File system is corrupted */
    void checkFS();
    bool isSharedOverUSB();

protected:
    StoragePartition(PartitionType, FilesystemType, const QString& , const QString& , const QString& );
    void createDirectoryTree();
    bool mount();
    bool format(const QString& label);
    bool removeDir(const QString &dirName);
    void setMounted(bool);
    void setShared(bool);
    bool umount();

private:
    bool realIsMounted();
    PartitionType m_partitionType;
    FilesystemType m_fsType;
    QString m_udevPath;
    QString m_device;
    QString m_mountPoint;
    bool m_mounted;
    bool m_shared;
    /** true if partition is corrupted, otherwise false */
    bool m_corrupted;


};

#endif // STORAGEPARTITION_H
