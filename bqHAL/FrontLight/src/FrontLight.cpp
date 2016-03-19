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

#include "FrontLight.h"
#ifndef Q_WS_QWS
#include "FrontLightEmu.h"
#elif MX508
#include "FrontLightMx508.h"
#endif

FrontLight* FrontLight::_instance = NULL;

void FrontLight::staticInit()
{
    if (_instance == NULL)
    {
#ifndef Q_WS_QWS
            _instance = new FrontLightEmu();
#elif MX508
            _instance = new FrontLightMx508();
#endif
    }
}

void FrontLight::staticDone()
{
    if(_instance)
    {
        delete _instance;
        _instance = NULL;
    }
}

FrontLight* FrontLight::getInstance()
{
    return _instance;
}
