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

#ifndef ADCONVERTER_H
#define ADCONVERTER_H

#define     ADC_CHECKTIME                   1000
#define     DEVICE_ADC                      "/dev/adc"
#define     REQCODE_ADC_READINPUTSOURCE     _IOR('S', 0x02, unsigned long)

#include <libudev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <QObject>
#include <QDebug>
#include <QBasicTimer>
#include <QSocketNotifier>

class ADConverter : public QObject
{
    Q_OBJECT

public:
    ADConverter();
    virtual ~ADConverter();

    enum ADCStatus {
        ADC_STATUS_NO_WIRE,
        ADC_STATUS_CHARGER,
        ADC_STATUS_PC
    };

    virtual ADConverter::ADCStatus getStatus();
    virtual bool isConnectedToPc() { qDebug() << Q_FUNC_INFO << "base class always returning true" ; return true; } // we'll rely on derived implementation, but need to be non-pure as we call from base class

    static ADConverter* getInstance();
    static void staticInit();
    static void staticDone();

signals:
    void chargerStatusChange(bool);
    void pcPresentStatusChange(bool);

private slots:
    void checkDevice(int);

private:
    void processCurrentStatus();
    void checkWireDevicesStatus(struct udev *udev);
    void processWireUdevEvents(QString,QString);
    void enumerateUdevDevices();
    struct udev *udev;
    struct udev_monitor *udev_monitor;
    QSocketNotifier *notifier;
    ADCStatus _status;
    static ADConverter* _instance;
    bool firstCheckDone;
    int processingRequests;

};

#endif // ADCONVERTER_H
