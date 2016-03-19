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

#ifndef FRONTLIGHT_H
#define FRONTLIGHT_H

#include <QObject>


class PowerManagerLock;

class FrontLight : public QObject
{
    Q_OBJECT

public:
    static void staticInit();
    static FrontLight* getInstance();
    static void staticDone();

    virtual bool switchFrontLight(bool state) = 0;
    virtual void setFrontLightActive(bool on) = 0;
    virtual bool isFrontLightActive() = 0;
    virtual bool setBrightness(int) = 0;
    virtual int getBrightness() = 0;

signals:
    void frontLightPower(bool);

private:
    static FrontLight* _instance;

};

#endif // FrontLight
