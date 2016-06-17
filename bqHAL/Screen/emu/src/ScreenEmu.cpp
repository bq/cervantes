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

#include "ScreenEmu.h"
#include "QBook.h"
#include <QDebug>


ScreenEmu::ScreenEmu()
{
    qDebug() << "--->" << Q_FUNC_INFO;
}

void ScreenEmu::setMode(Screen::ScreenMode mode, bool justOnce, QString from)
{
    setMode(mode, justOnce, FLAG_PARTIALSCREEN_UPDATE, from);
}

void ScreenEmu::setMode(Screen::ScreenMode mode, bool justOnce, int refreshFlags, QString from)
{
//    qDebug() << "--->" << Q_FUNC_INFO << "Mode: " << mode << " once: " << justOnce
//             << "flags:" << refreshFlags << "from: " << from;
}

void ScreenEmu::refreshScreen(Screen::ScreenMode mode, QString from)
{
    qDebug() << "--->" << Q_FUNC_INFO << "Mode: " << mode << "from: " << from;
}

void ScreenEmu::setUpdateScheme(SchemeMode mode, bool justOnce)
{
    qDebug() << "--->" << Q_FUNC_INFO << "Mode" << mode << "just once" << justOnce;
}

void ScreenEmu::lockScreen()
{
    qDebug() << "--->" << Q_FUNC_INFO;
}

void ScreenEmu::releaseScreen()
{
    qDebug() << "--->" << Q_FUNC_INFO;
}

void ScreenEmu::flushImage(QImage &/*img*/, const QPoint &/*point*/)
{
    qDebug() << "--->" << Q_FUNC_INFO;
}

void ScreenEmu::restoreMode()
{
    qDebug() << "--->" << Q_FUNC_INFO;
}

void ScreenEmu::refreshScreen(Screen::ScreenMode, int, QString)
{
    qDebug() << "--->" << Q_FUNC_INFO;
}

void ScreenEmu::queueUpdates()
{
    qDebug() << "--->" << Q_FUNC_INFO;
}

void ScreenEmu::flushUpdates()
{
    qDebug() << "--->" << Q_FUNC_INFO;
}

void ScreenEmu::resetQueue()
{
    qDebug() << "--->" << Q_FUNC_INFO;
}

void ScreenEmu::resetFlushQueue()
{
    qDebug() << "--->" << Q_FUNC_INFO;
}

void ScreenEmu::flushColorScreen(Qt::GlobalColor color)
{
    qDebug() << "--->" << Q_FUNC_INFO;
}

int ScreenEmu::screenWidth() const
{
    qDebug() << "--->" << Q_FUNC_INFO;
    switch(QBook::getInstance()->getResolution())
    {
        case QBook::RES1072x1448:
            return 1072;
            break;
        case QBook::RES758x1024:
            return 758;
            break;
        case QBook::RES600x800: default:
            return 600;
            break;
    }
}

int ScreenEmu::screenHeight() const
{
    qDebug() << "--->" << Q_FUNC_INFO;
    switch(QBook::getInstance()->getResolution())
    {
        case QBook::RES1072x1448:
            return 1448;
            break;
        case QBook::RES758x1024:
            return 1024;
            break;
        case QBook::RES600x800: default:
            return 800;
            break;
    }
}

void ScreenEmu::setScreenOrentation(ScreenOrientation /*orientation*/)
{
    qDebug() << Q_FUNC_INFO;
}
