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

#include "DeviceInfo.h"

#ifndef Q_WS_QWS
#include "DeviceInfoEmu.h"
#elif MX508
#include "DeviceInfoMx508.h"
#endif


DeviceInfo* DeviceInfo::_instance = NULL;

DeviceInfo* DeviceInfo::getInstance()
{
    return _instance;
}

void DeviceInfo::staticInit()
{
    if (_instance == NULL)
    {
#ifndef Q_WS_QWS
            _instance = new DeviceInfoEmu();
#elif MX508
            _instance = new DeviceInfoMx508();
#endif
    }
}

void DeviceInfo::staticDone()
{
    if(_instance)
    {
        delete _instance;
        _instance = NULL;
    }
}
