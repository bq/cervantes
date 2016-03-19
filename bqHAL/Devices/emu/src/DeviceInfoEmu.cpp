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

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "DeviceInfoEmu.h"
#include "Storage.h"
#include "QBook.h" // TODO: Remove this dependency
#include <QFile>
#include <QApplication>
#include <QString>
#include <QStringList>
#include <QSettings>
#include <QTextStream>
#include <QDebug>

#define UBOOT_VERSION  QString("%1/version/uboot_version").arg(Storage::getInstance()->getMountsPath())
#define KERNEL_VERSION QString("%1/version/version").arg(Storage::getInstance()->getMountsPath())
#define ROOTFS_VERSION QString("%1/version/rootfs.rev").arg(Storage::getInstance()->getMountsPath())

extern "C" void QBookScreen_GetInfo(int*);

static const char*	s_szInfoItemPrefix="          ";

void DeviceInfoEmu::setInitInfo(QSettings& settings)
{
    QFile file;
    QString key,value;

    int num = QBook::settings().value("setting/deviceinfo/infonum").toInt();
    for ( int i=0; i<3; i++ )
    {
        num++;
        key = QString("setting/deviceinfo/item%1").arg(num);
        switch (i) {
        case 0:
            file.setFileName(UBOOT_VERSION);
            value=QString("u-boot version\n");
            break;
        case 1:
            file.setFileName(KERNEL_VERSION);
            value=QString("kernel version\n");
            break;
        case 2:
            file.setFileName(ROOTFS_VERSION);
            value=QString("ebook version\n");
            break;
        }
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug("version file err");
        }
        else
        {
            QTextStream in(&file);
            QString line = in.readLine();

            while( !line.isNull() )
            {
                if(line.startsWith("Linux version"))
                {
                    QStringList list = line.split("(");
                    if( list.length() > 0 )
                        settings.setValue(key, value.append(list[0]));
                }
                else
                    settings.setValue(key, value.append(line));

                line = in.readLine();
            }
        }
        file.close();
    }	// end for
    settings.sync();
}

QHash<QString, QString> DeviceInfoEmu::getSpecificDeviceInfo()
{
    QHash<QString, QString> data;
    return data;
}

void DeviceInfoEmu::populateHWinfo(QStringList& data, const QSettings& s)
{
        Q_UNUSED(s);
    QString deviceIdTitle = QApplication::tr("Device ID");
    QString deviceID = "E-M-U-L-A-T-O-R";

    deviceID += (QString(s_szInfoItemPrefix));
    data.append(deviceIdTitle+QString(": ")+deviceID);
}

QString DeviceInfoEmu::getSerialNumber()
{
    static char *SN = NULL;
    if (SN != NULL)
        return SN;

    if (access("serial.txt", F_OK | R_OK) != 0) {
        SN = "SN-TEST000009";
        return SN;
    }

    FILE *fp;
    SN = (char*)calloc(16, sizeof(char));
    fp = fopen("serial.txt", "r");
    int r = fread(SN, 1, 16, fp);
    SN[r-1] = '\0';
    qDebug() << "Read " << r << "bytes from serial.txt: " << SN;
    fclose(fp);

    return SN;
}

bool DeviceInfoEmu::hasFrontLight()
{
        return true; //Irrelevant. True if you would like to see front light in emulator
}

int DeviceInfoEmu::getHwId(){
    qDebug() << Q_FUNC_INFO;

    return E60672;
}
