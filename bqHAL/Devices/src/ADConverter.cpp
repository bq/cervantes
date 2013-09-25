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

#include "ADConverter.h"

#include <QBasicTimer>
#include <QTimerEvent>
#include <QCoreApplication>
#include <QDebug>

#ifndef Q_WS_QWS
    #include "ADConverterEmu.h"
#elif MX508
    #include "ADConverterMx508.h"
#endif

#define UDEV_MONITOR_BUFFER_SIZE_WIRE 128*1024*1024

ADConverter* ADConverter::_instance = NULL;

ADConverter* ADConverter::getInstance()
{
    return _instance;
}

void ADConverter::staticInit()
{
    if (_instance == NULL)
    {
#ifndef Q_WS_QWS
            _instance = new ADConverterEmu();
#elif MX508
            _instance = new ADConverterMx508();
#endif
    }
}

void ADConverter::staticDone()
{
    if(_instance)
    {
        delete _instance;
        _instance = NULL;
    }
}

ADConverter::ADConverter()
{
    _status = ADC_STATUS_NO_WIRE;
    firstCheckDone = false;
    processingRequests = 0;

    udev = udev_new();
    if (!udev) {
            qDebug() << Q_FUNC_INFO << "Cannot create udev";
            return;
    }

    udev_monitor = udev_monitor_new_from_netlink(udev, "udev");
    if (!udev_monitor) {
            qDebug() << Q_FUNC_INFO << "Cannot create mon";
            return;
    }

    if (udev_monitor_filter_add_match_subsystem_devtype(udev_monitor, "platform", NULL) != 0) {
            qDebug() << Q_FUNC_INFO << "Error with udev_monitor_filter_add_match_subsystem_devtype for platform devices";
            return;
    }

    udev_monitor_set_receive_buffer_size(udev_monitor, UDEV_MONITOR_BUFFER_SIZE_WIRE);

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

ADConverter::~ADConverter()
{
    // Does nothing
}

void ADConverter::checkDevice(int fd) {
    qDebug() << Q_FUNC_INFO << "FD: "<< fd;

    struct udev_device *dev;
    dev = udev_monitor_receive_device(udev_monitor);
    if (dev)
    {
        QString action = udev_device_get_action(dev);
        QString sysname = QString(udev_device_get_sysname(dev));
        processWireUdevEvents(sysname, action);
        udev_device_unref(dev);
    }
}

void ADConverter::processWireUdevEvents(QString sysname, QString action)
{
    int cnt;

    if(action != "add" && action != "remove")
        return;

    if (!sysname.contains("usb_plug") && !sysname.contains("usb_host"))
	return;

    qDebug() << "--->" << Q_FUNC_INFO << "sysname:" << sysname << "action:" << action << "processingRequests: " << processingRequests;
    if (processingRequests > 0) {
	qDebug() << Q_FUNC_INFO << "Another call has not finished yet. Waiting for termination before processing this one";
	/* wait for max 3 seconds, before aborting */
	cnt = 0;
	while (processingRequests > 0 && cnt < 30) {
            QCoreApplication::processEvents();
	    usleep(100000);
	    cnt++;
	    qDebug() << "still waiting";
        }

        if (cnt >= 30) {
	    qDebug() << "previous event took to long to finish, aborting this one";
	    /* it seems this waiting here, sometimes blocks the isConnectedToPc function of the running action.
	     * After we return from this one, the other one finishes running, so it might be nice to reschedule
	     * this one after some seconds, to still handle the unplug event.
	     */
	    return;
	}
    }

    processingRequests++;

    bool pluggedIn = (action == "add");

    if (pluggedIn) {
        // We cannot trust the mx kernel host detection so force another check using
        // the old way.
        if(sysname.contains("usb_host") && isConnectedToPc()) {
            emit pcPresentStatusChange(true);
            _status = ADC_STATUS_PC;
        } else {
            emit chargerStatusChange(true);
            _status = ADC_STATUS_CHARGER;
        }
    } else {
        if (_status == ADC_STATUS_PC) 
            emit pcPresentStatusChange(false);
        else if (_status == ADC_STATUS_CHARGER)
            emit chargerStatusChange(false);
        _status = ADC_STATUS_NO_WIRE;
    }
    processingRequests--;
}

ADConverter::ADCStatus ADConverter::getStatus()
{
    if (!firstCheckDone) {
    	// Initial enumerate to get initial status and send signals
    	enumerateUdevDevices();
	firstCheckDone = true;
    }
    return _status;
}

void ADConverter::enumerateUdevDevices()
{
    qDebug() << "--->" << Q_FUNC_INFO;

    bool wired = false;
    struct udev_enumerate *udev_enumerate;

    udev_enumerate = udev_enumerate_new(udev);
    if (udev_enumerate == NULL)
        return;

    udev_enumerate_add_match_subsystem(udev_enumerate, "power_supply");

    udev_enumerate_scan_devices(udev_enumerate);

    struct udev_list_entry *list_entry;

    udev_list_entry_foreach(list_entry, udev_enumerate_get_list_entry(udev_enumerate))
    {
        struct udev_device *device;

        device = udev_device_new_from_syspath(udev_enumerate_get_udev(udev_enumerate),
                                      udev_list_entry_get_name(list_entry));
        if (device != NULL) {
            QString sysname = udev_device_get_sysname(device);
            QString path = udev_device_get_devpath(device);
            QString parent = udev_device_get_devpath( udev_device_get_parent (device));

            qDebug() << "Got " << path;
            qDebug() << "    Subsystem: "<< udev_device_get_subsystem(device);
            qDebug() << "    Sysname: " << sysname;

            struct udev_list_entry *prop;
            udev_list_entry_foreach(prop, udev_device_get_properties_list_entry(device))
            {
                qDebug() << "property" << udev_list_entry_get_name(prop) << "-" << udev_list_entry_get_value(prop);
                if (QString(udev_list_entry_get_name(prop)).trimmed() == "POWER_SUPPLY_ONLINE")
                {
                    if (QString(udev_list_entry_get_value(prop)).trimmed() == "0")
                    {
                        wired = false;
                    }
                    else
                    {
                        wired = true;
                    }
                }
            }
            udev_device_unref(device);
        }
    }

    if (wired)
    {
        qDebug() << Q_FUNC_INFO << "Checking if is wired to an USB or a CHARGER";
        // check if it's wire to a usb_host or usb_plug
        delete udev_enumerate;
        udev_enumerate = udev_enumerate_new(udev);
        if (udev_enumerate == NULL)
            return;

        udev_enumerate_add_match_subsystem(udev_enumerate, "platform");
        udev_enumerate_scan_devices(udev_enumerate);

        struct udev_list_entry *list_entry;

        udev_list_entry_foreach(list_entry, udev_enumerate_get_list_entry(udev_enumerate))
        {
            struct udev_device *device;

            device = udev_device_new_from_syspath(udev_enumerate_get_udev(udev_enumerate),
                                          udev_list_entry_get_name(list_entry));
            if (device != NULL)
            {
                QString sysname = udev_device_get_sysname(device);
                processWireUdevEvents(sysname, "add");
            }
        }
    }
    else
        _status = ADC_STATUS_NO_WIRE;
    qDebug() << Q_FUNC_INFO << "ADC_STATUS " << (_status == ADC_STATUS_NO_WIRE?"ADC_STATUS_NO_WIRE":(_status == ADC_STATUS_CHARGER?"ADC_STATUS_CHARGER":"ADC_STATUS_PC"));

}
