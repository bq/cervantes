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

#ifndef __STORAGEDEVICE_H__
#define __STORAGEDEVICE_H__

#include <QString>

class StorageDevice
{

friend class Storage;

public:
    const QString& getDevice() const;
    const QString& getUdevPath() const;
    bool isInserted() const;
    enum Type
    {
        INTERNAL,
        REMOVABLE
    };
    Type getType() const { return m_type; }
    
protected:
    StorageDevice(const QString& , const QString& , Type);
    void setInserted(bool);

private:
    bool m_inserted;
    QString m_udevPath;
    QString m_device;
    bool realIsInserted() const;
    Type m_type;

};

#endif // STORAGEDEVICE
