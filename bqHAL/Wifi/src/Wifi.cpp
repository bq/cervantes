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

#include "Wifi.h"
#ifndef Q_WS_QWS
#include "WifiEmu.h"
#elif MX508
#include "WifiMx508.h"
#endif

#include "PowerManager.h"


Wifi* Wifi::_instance = NULL;

Wifi* Wifi::getInstance()
{
    return _instance;
}

void Wifi::staticInit()
{
    if (_instance == NULL)
    {
#ifndef Q_WS_QWS
        _instance = new WifiEmu();
#elif MX508
        _instance = new WifiMx508();
#endif
    }
}

void Wifi::staticDone()
{
    if (_instance) {
        delete _instance;
        _instance = NULL;
    }
}
