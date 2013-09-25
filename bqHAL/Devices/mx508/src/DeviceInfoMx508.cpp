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

#include "DeviceInfoMx508.h"
#include "hgversion.h"
#include "version.h"

#include <QDebug>
#include <QStringList>
#include <QFile>
#include <QHash>
#include <ntx.h>
#include <QProcess>
#include <ntx.h>

#define E606A2_PCBID 22
#define E60672_PCBID 23
#define E60Q22_PCBID 33


DeviceInfoMx508::DeviceInfoMx508()
{
    qDebug() << "--->" << Q_FUNC_INFO;

    char sn[36];
    
    if (ntx_read_serial("/dev/mmcblk0", sn, 36) == 0)
        serial = QString(sn);
    else
        serial = QString("SN-undefined");
}

/**
  * This function returns a QStringList with format "label: value"
  * or empty string to add space.
  * Maximum number of elements is 6
  */
QHash<QString, QString> DeviceInfoMx508::getSpecificDeviceInfo()
{
    qDebug() << "--->" << Q_FUNC_INFO;

    QHash<QString, QString> elements;


    /** Add internal software info:
      * Mercurial revision
      * Rootfs version in compilation/Actual rootfs installed
      * Kernel revision
      * Private repo (services and debug)
      */

    elements.insert("Internal Revision", HG_VERSION);
    elements.insert("Static rootfs version", ROOTFS_VERSION);
    elements.insert("Private repo version", HG_SERVICES_VERSION);

#ifdef BATTERY_TEST // To mark as special build
    elements.insert("Software version", "BATTERY_TEST " + QString(QBOOKAPP_VERSION));
#elif defined(SHOWCASE)
    elements.insert("Software version", "SHOWCASE " + QString(QBOOKAPP_VERSION));
#elif defined(HACKERS_EDITION)
    elements.insert("Software version", "HACKERS " + QString(QBOOKAPP_VERSION));
#else
    elements.insert("Software version", QBOOKAPP_VERSION);
#endif

    //Installed
    QFile *versionFile = new QFile("/.version");
    if (versionFile->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString verNumber(versionFile->readLine());
        QString hgVersion(versionFile->readLine());
        elements.insert("Actual rootfs version", verNumber.trimmed() + "-" + hgVersion.trimmed());
    }

    //Kernel revision (uname --kernel-release)
    QProcess uName;
    QStringList arguments;
    arguments << "--kernel-release";

    uName.start("uname", arguments);
    uName.waitForFinished(1000);
    if (uName.exitStatus() != QProcess::NormalExit || uName.exitCode() != 0) {
        qDebug() << Q_FUNC_INFO << "uname exited with error. Check failed";
        qDebug() << uName.readAllStandardOutput();
        qDebug() << uName.readAllStandardError();

    }
    elements.insert("Kernel version", QString(uName.readAll()).trimmed());
    return elements;
}

bool DeviceInfoMx508::hasFrontLight()
{
        struct ntxhwconfig hwc;

        memset(&hwc, 0, sizeof(struct ntxhwconfig));

        ntx_read_hwconfig("/dev/mmcblk0", &hwc);
        qDebug() << "front_light=" << hwc.values.front_light;
        return hwc.values.front_light != 0 && hwc.values.pcb != E606A2_PCBID;
}

int DeviceInfoMx508::getHwId()
{
    qDebug() << "--->" << Q_FUNC_INFO;

    struct ntxhwconfig hwc;
    memset(&hwc, 0, sizeof(struct ntxhwconfig));
    ntx_read_hwconfig("/dev/mmcblk0", &hwc);


    switch(hwc.values.pcb)
    {
    case E606A2_PCBID:
        qDebug() << Q_FUNC_INFO << "E606A2";
        return E606A2;
    case E60672_PCBID:
        qDebug() << Q_FUNC_INFO << "E60672";
        return E60672;
    case E60Q22_PCBID:
        qDebug() << Q_FUNC_INFO << "E60Q22";
        return E60Q22;
    default:
        qWarning() << Q_FUNC_INFO << "UNKNOWN_HW_MODEL" << hwc.values.pcb;
    }
    return UNKNOWN_HW_MODEL;
}
