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

#include "PowerManager.h"
#include "ADConverterMx508.h"
#include "ADConverter.h"

#include <QDebug>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QByteArray>
#include <QEventLoop>
#include <QTimer>
#include <QElapsedTimer>
#include <QCoreApplication>

ADConverterMx508::ADConverterMx508()
        :ADConverter(),
        m_currentStatus(ADC_STATUS_NO_WIRE)

{
    m_powerLock = PowerManager::getNewLock(this);
}

static inline int readFrameNumber(QFile *file)
{
    bool ok;

    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << Q_FUNC_INFO << "ERROR: file open FAIL";
        return -1;
    }

    // Check device
    QString line = QString(file->readLine());

    while(!line.startsWith("USB Frame Index Reg")){
        if(file->atEnd()){
            qDebug() << Q_FUNC_INFO << "ERROR: Line not found";
            return -1;
        }
        line = QString(file->readLine());
    }
    file->close();

    line.chop(1); // Remove last character

    return line.right(line.size() - line.lastIndexOf(" ") - 1).toInt(&ok, 16);
}

bool ADConverterMx508::isConnectedToPc(){
    qDebug() << "--->" << Q_FUNC_INFO;
    m_powerLock->activate();

    QFile *file = new QFile("/proc/driver/fsl_usb2_udc");
    QEventLoop waitingLoop;
    QString line;
    bool result;
    int oldNumber;
    int newNumber;

    QElapsedTimer timer;
    timer.start();
    // Wait until modules are correctly loaded and file exists
    while (!file->exists() && timer.elapsed() < 1000){
        qDebug() << Q_FUNC_INFO << "Waiting for fsl_usb2_udc file";
        QCoreApplication::processEvents();
    }

    if(!file->exists()) {
        result = false;
        goto unlock;
    }

    oldNumber = readFrameNumber(file);
    if (oldNumber < 0) {
        result = false;
        goto unlock;
    }

    // Load USB modules
    system("modprobe g_zero");
    qDebug() << Q_FUNC_INFO << "Module inserted, waiting for usb traffic";

    /* FIXME: Needed time could change depending on processor load,
      it would be better to find a way to know that fsl_usb2_udc is
      updated with connected device Frame Index */
    //wait 1 second for a possible host to generate a bit of traffic
    usleep(1000000); /* 1sec */

    newNumber = readFrameNumber(file);
    if (newNumber < 0) {
        result = false;
        goto unlock_usb;
    }

    qDebug() << "Frame numbers (old/new):" << oldNumber << "/" << newNumber;
    result = (oldNumber != newNumber);

    qDebug() << Q_FUNC_INFO << "Finished";

unlock_usb:
    removeUsbModules();

unlock:
    delete file;
    m_powerLock->release();
    return result;
}

void ADConverterMx508::removeUsbModules(){

    qDebug() << "--->" << Q_FUNC_INFO;
    system("rmmod g_zero");
}


