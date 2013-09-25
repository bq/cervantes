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

//***************************************************************************
//* $Workfile: $
//* $Revision: $
//*
//***************************************************************************

#ifndef QBookDevel_H
#define QBookDevel_H

#include <QDialog>
#include <QDebug>
#include <QString>
#include <QFileInfo>

class PowerManagerLock;

class QBookDevel
{

public:
    static bool isTelnetEnabled(void);
    static bool isNfsEnabled(void);
    static bool isUsbNetworkEnabled(void);
    static bool areLogsEnabled(void);
    static bool areCoresEnabled(void);

    static void enableCores();
    static void enableLogs();
    static void enableUsbNetwork(bool);
    static void enableNfs();
    static void enableTelnet();

    static void disableCores();
    static void disableLogs();
    static void disableTelnet();
    static int  getPid(const char*);

private:
    static PowerManagerLock* m_powerLock;


};


#endif
