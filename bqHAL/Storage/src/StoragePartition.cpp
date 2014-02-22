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

#include <sys/mount.h>
#include <errno.h>
#include <mntent.h>
#include <sys/statvfs.h>

#include <QDebug>
#include <QProcess>
#include <QDir>
#include <QEventLoop>

#include "StoragePartition.h"

#define MOUNT_TRIES 3
#define QBOOKINI_DEFAULT_DIRECTORY "/mnt/data"

StoragePartition::StoragePartition(StoragePartition::PartitionType partitionType,
                                   StoragePartition::FilesystemType fsType,
                                   const QString&  udevPath,
                                   const QString&  device,
                                   const QString&  mountPoint) :
    m_partitionType(partitionType)
  , m_fsType(fsType)
  , m_udevPath(udevPath)
  , m_device(device)
  , m_mountPoint(mountPoint)
  , m_shared(false)
{
        m_mounted = realIsMounted();
        qDebug() << Q_FUNC_INFO << ":";
        qDebug() << "\tPartitionType: " << m_partitionType ;
        qDebug() << "\tfsType: " << m_fsType;
        qDebug() << "\tudevPath: " << m_udevPath;
        qDebug() << "\tdevice: " << m_device;
        qDebug() << "\tmountPoint: " << m_mountPoint;

}

const QString & StoragePartition::getMountPoint() const {
        return m_mountPoint;
}

const QString & StoragePartition::getUdevPath() const {
        return m_udevPath;
}

const QString & StoragePartition::getDevice() const {
        return m_device;
}

void StoragePartition::setDevice(const QString& newDevice)
{
    qDebug() << Q_FUNC_INFO << ": change device " << m_device << " to " << newDevice;
    m_device = newDevice;
}

unsigned long long StoragePartition::getFreeSpace() const {
    if (!isMounted())
    {
        qDebug() << Q_FUNC_INFO << ": Not mounted";
        return 0;
    }

    struct statvfs buf;
    QByteArray bamnt = m_mountPoint.toLocal8Bit();
    const char *mnt = bamnt.constData();

    if (statvfs(mnt, &buf) != 0)
    {
        qDebug() << Q_FUNC_INFO << ": Error getting statvfs: " << errno << " -> " << strerror(errno);
        return 0;
    }

    qDebug() << Q_FUNC_INFO << "Free space: " << (unsigned long long)buf.f_bfree * buf.f_bsize;
    return (unsigned long long)buf.f_bfree * buf.f_bsize;
}

unsigned long long StoragePartition::getTotalSpace() const
{
    if (!isMounted())
            return 0;

    struct statvfs buf;
    QByteArray bamnt = m_mountPoint.toLocal8Bit();
    const char *mnt = bamnt.constData();

    if (statvfs(mnt, &buf) != 0)
            return 0;

    qDebug() << Q_FUNC_INFO << "Total space: " << (unsigned long long)buf.f_blocks * buf.f_bsize;
    return (unsigned long long)buf.f_blocks * buf.f_bsize;
}

StoragePartition::FilesystemType StoragePartition::getFilesystemType() const {
        return m_fsType;
}
StoragePartition::PartitionType StoragePartition::getPartitionType() const {
        return m_partitionType;
}
bool StoragePartition::isMounted() const {
        return m_mounted;
}
bool StoragePartition::mount()
{
    int res = 0;
    int tries = 0;
    char *type;
    unsigned long flags = 0;
    char *data = NULL;

    if (m_mounted)
            return true;

    if (m_fsType == FS_VFAT) {
            type = (char*)"vfat";
            flags |= MS_NOATIME;
            data = "shortname=mixed,showexec,utf8";
    } else if (m_fsType == FS_EXT3) {
            type = (char*)"ext3";
            flags |= MS_NOATIME;
    } else {
            qDebug() << Q_FUNC_INFO << " " << m_device << " unkown filesystem type";
            return false;
    }

    /* mount external partitions synchronous
     * Introduces a slight speed penalty, but also ensures
     * that writes are finished correctly, thus reducing the
     * number of possible corruptions when the user unexpectedly
     * removes the sd card.
     */
    if (getPartitionType() == StoragePartition::REMOVABLE)
        flags |= MS_SYNCHRONOUS;

    QByteArray bamnt = m_mountPoint.toLocal8Bit();
    const char *mnt = bamnt.constData();
    QByteArray badev;
    const char *dev;

    do {
        badev = m_device.toLocal8Bit();
        dev = badev.constData();

        qDebug() << Q_FUNC_INFO << ": Mounting '" << dev << "'' into '" << mnt << ": type " << type << ": flags " << flags;

        res = ::mount(dev, mnt, type, flags, data);
        if(res == 0) break;
        qDebug() << Q_FUNC_INFO << ": Failed for '" << dev << "'' into '" << mnt << "': " << strerror(errno);
    } while (res != 0 && tries++ < MOUNT_TRIES);

    if (res != 0)
    {
        return false;
    }

    system("sync; echo 3 > /proc/sys/vm/drop_caches");

    m_mounted = true;
    qDebug() << Q_FUNC_INFO << ": mounted device " << dev << " at " << mnt;

    createDirectoryTree();
    return true;
}

bool StoragePartition::umount() {
    int res;
    int tries = 0;
    int flags = 0;

    if (!m_mounted)
            return true;

    flags |= MNT_DETACH;

    QByteArray bamnt = m_mountPoint.toLocal8Bit();
    const char *mnt = bamnt.constData();
    do
    {
        qDebug() << Q_FUNC_INFO << ": umounting " << mnt << ", flags " << flags;
        res = ::umount2(mnt, flags);
        if (res != 0)
        {
            qDebug() << Q_FUNC_INFO << ": umount failed for " << mnt << ": " << strerror(errno);
            flags |= MNT_FORCE;
        }
    } while (res != 0 && tries++ < MOUNT_TRIES);

    if (res != 0)
    {
        return false;
    }

    m_mounted = false;
    qDebug() << Q_FUNC_INFO << ": umounted device " << m_device;
    return true;
}

bool StoragePartition::isCorrupted() const
{
    return m_corrupted;
}

/**
  Check if file system is corrupted.
  Now only partitions in vfat are checked.
  */
void StoragePartition::checkFS()
{
//    if (fsType == FS_VFAT)
//    {
//        if (isMounted())
//            if (!umount())
//            {
//                qDebug() << Q_FUNC_INFO << ": Cannot umount this partition";
//                return;
//            }

//        QByteArray badev = device.toLocal8Bit();
//        const char *dev = badev.constData();
//        qDebug() << "--->" << Q_FUNC_INFO << "checking dev: " << dev;
//        QProcess dosfsckproc;
//        dosfsckproc.start("dosfsck", QStringList() << "-n" << "-T2" << dev);
//        dosfsckproc.waitForFinished();
//        int exitCode = dosfsckproc.exitCode ();
////        EXIT STATUS dosfsck
////               0      No recoverable errors have been detected.
////               1      Recoverable errors have been detected or dosfsck has discovered an internal inconsistency.
////               2      Usage error. dosfsck did not access the file system.

//        if (exitCode == 0)
//            corrupted = false;
//        else
//            corrupted = true;
//    }
//    else
//        corrupted = false;

//    qDebug() << Q_FUNC_INFO << " is corrupted? " << (corrupted?"true":"false");
    m_corrupted = false;
}

bool StoragePartition::format(const QString& name)
{
    bool result = false;
    if (m_fsType == FS_VFAT)
    {
        QByteArray badev = m_device.toLocal8Bit();
        const char *dev = badev.constData();
        qDebug() << "--->" << Q_FUNC_INFO << "formating dev: " << dev << " with type VFAT";
        QProcess dosfsckproc;
        dosfsckproc.start("mkdosfs", QStringList() << "-n" << name << dev);
        dosfsckproc.waitForFinished();
        int exitCode = dosfsckproc.exitCode ();
        qDebug() << Q_FUNC_INFO << ": exitCode = " << exitCode;
        if (exitCode == 0)
        {
            result = true;
        }
        else
            result = false;
    }
    else if (m_fsType == FS_EXT3)
    {
        QByteArray badev = m_device.toLocal8Bit();
        const char *dev = badev.constData();
        qDebug() << "--->" << Q_FUNC_INFO << "formating dev: " << dev << " with type EXT3";
        QProcess dosfsckproc;
        QEventLoop waitingLoop;
        connect(&dosfsckproc, SIGNAL(finished ( int, QProcess::ExitStatus)), &waitingLoop, SLOT(quit()));
        dosfsckproc.start("mkfs.ext3", QStringList() << "-L" << name << dev);
        waitingLoop.exec();

        int exitCode = dosfsckproc.exitCode ();
        qDebug() << Q_FUNC_INFO << ": exitCode = " << exitCode;
        if (exitCode == 0)
        {
            result = true;
        }
        else
            result = false;
    }
    return result;
}

bool StoragePartition::removeDir(const QString &dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }

    return result;
}

void StoragePartition::setMounted(bool mounted) {
        m_mounted = mounted;
}

void StoragePartition::setShared(bool shared) {
        m_shared = shared;
}

bool StoragePartition::realIsMounted() {
        struct mntent *mp;
        FILE *f = fopen ("/proc/mounts", "r");
        if (f == NULL)
                return false;

        QByteArray badev;
        const char *dev;

        while ((mp = getmntent (f)) != NULL)
        {
            badev = m_device.toLocal8Bit();
            dev = badev.constData();
            // if (strcmp(mp->mnt_fsname, dev) == 0 && strcmp(mp->mnt_dir, mnt) == 0) {
            // We check only device as mount point can be a symbolic link
            if (strcmp(mp->mnt_fsname, dev) == 0)
            {
                    fclose(f);
                    return true;
            }
        }
        fclose(f);
        return false;
}

void StoragePartition::createDirectoryTree()
{
    switch(m_partitionType)
    {
        case StoragePartition::PUBLIC:
        {
            // setup adobe drm
            QDir dir(m_mountPoint);

            QString path = qgetenv("ADEPT_ACTIVATION_DIR");

            if (!dir.exists(path))
            {
                qDebug() << Q_FUNC_INFO << ": creating directory '.adobe-digital-editions'";
                if (!dir.mkdir(path))
                    qWarning() << Q_FUNC_INFO << ": Error creating directory '.adobe-digital-editions'";
            }

            path = qgetenv("ADOBE_DE_DOC_FOLDER");

            if (!dir.exists(path))
            {
                qDebug() << Q_FUNC_INFO << ": creating directory 'Digital Editions'";
                if (!dir.mkdir(path))
                    qWarning() << Q_FUNC_INFO << ": Error creating directory 'Digital Editions'";
            }

            path = qgetenv("ADOBE_SI_FILE_FOLDER");

            if (!dir.exists(path))
            {
                qDebug() << Q_FUNC_INFO << ": creating directory 'screensaver-images'";
                if (!dir.mkdir(path))
                    qWarning() << Q_FUNC_INFO << ": Error creating directory 'screensaver-images'";
            }
        }
        break;

        case StoragePartition::DATA:
        {
            QDir dir(m_mountPoint);

            if (!dir.exists(".adept"))
            {
                qDebug() << Q_FUNC_INFO << ": creating directory '.adept'";
                if (!dir.mkdir(".adept"))
                    qWarning() << Q_FUNC_INFO << ": Error creating directory '.adept'";
            }

        }
        break;

        default:
        break;
    }
}

void StoragePartition::setVolumeLabel(const QString &label)
{
	qDebug() << Q_FUNC_INFO << label;
	if (m_fsType != FS_VFAT) {
		qDebug() << Q_FUNC_INFO << "Can only rename VFAT partitions";
		return;
	}
	qDebug() << Q_FUNC_INFO << "Setting new label for device " << getDevice() << ":" << label;

    QProcess mlabelproc;
    mlabelproc.start("mlabel", QStringList() << "-i" << getDevice() << "::" + label);
    mlabelproc.waitForFinished();

}
