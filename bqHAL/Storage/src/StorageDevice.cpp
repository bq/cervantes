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

#include <QProcess>
#include <QDebug>
#include <StorageDevice.h>


StorageDevice::StorageDevice(const QString& udevPath, const QString& device, Type type) :
    m_udevPath(udevPath)
  , m_device(device)
  , m_type(type)
{
    qDebug() << Q_FUNC_INFO << ": device " << device << ", udevPath " << udevPath;
    m_inserted = realIsInserted();
}

const QString& StorageDevice::getUdevPath() const {
        return m_udevPath;
}
const QString& StorageDevice::getDevice() const {
        return m_device;
}
bool StorageDevice::isInserted() const {
        return m_inserted;
}

void StorageDevice::setInserted(bool inserted) {
        this->m_inserted = inserted;
}

bool StorageDevice::realIsInserted() const {
	/* As the removable device could be inserted before we run, we need
	 * to check instead os just listening for udev events.
	 * This is kinda ugly */

	/* FIXME: I guess we could do this also with udev */
        QStringList elements = m_device.split("/");
	QString command = QString ("grep " + elements.last() + " /proc/partitions");
        qDebug() << "Checking if device " << m_device << "is present with command: " << command;
	int err = QProcess::execute(command);

	if(err == 0)
		return true;

	return false;

}
