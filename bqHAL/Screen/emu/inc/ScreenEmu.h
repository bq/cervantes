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

#include "Screen.h"
#include <QObject>

class ScreenEmu : public Screen
{
    Q_OBJECT

public:
    ScreenEmu();

    void setMode(ScreenMode mode, bool justOnce = false, int refreshFlags = FLAG_PARTIALSCREEN_UPDATE, QString from = "UNKNOWN");
    void setMode(Screen::ScreenMode mode, bool justOnce = false, QString from = "UNKNOWN");
    void refreshScreen(Screen::ScreenMode mode = MODE_SAFE, QString from = "UNKNOWN");
    void waitPanelNotBusy();
    void setUpdateScheme(SchemeMode, bool);
    void lockScreen();
    void releaseScreen();
    void flushImage(QImage&,const QPoint&);
    void flushColorScreen(Qt::GlobalColor = Qt::white);
    void restoreMode();
    void refreshScreen(Screen::ScreenMode, int, QString);
    virtual void queueUpdates();
    virtual void flushUpdates();
    virtual void resetQueue();
    virtual void resetFlushQueue();


    virtual int screenWidth() const;
    virtual int screenHeight() const;
    virtual void setScreenOrentation(ScreenOrientation orientation);
};

