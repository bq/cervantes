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

#ifndef SCREEN_H
#define SCREEN_H

#include <QObject>

/* update the whole screen, not only the update region */
#define FLAG_FULLSCREEN_UPDATE      (1 << 0)
#define FLAG_PARTIALSCREEN_UPDATE   0

/* wait for the update to complete before continuing */
#define FLAG_WAITFORCOMPLETION      (1 << 1)

/* SAFE = QUICK + FLAG_REFRESH, also usable in other modes */
#define FLAG_REFRESH			(1 << 2)

/* when updates are queued, ignore this and force the update onto the screen */
#define FLAG_IGNORE_QUEUE		(1 << 3)

class QString;
class QImage;
class QPoint;
class QRegion;

class Screen : public QObject
{
    Q_OBJECT

public:

    enum ScreenMode {
        MODE_BLOCK,
        MODE_AUTO,
        MODE_SAFE,
        MODE_QUICK,
        MODE_FAST,
        MODE_FASTEST
    };

    enum SchemeMode {
        SCHEME_QUEUE,
        SCHEME_MERGE
    };

    enum ScreenOrientation
    {
        ROTATE_0            = 0,
        ROTATE_90           = 1,
        ROTATE_180          = 2,
        ROTATE_270          = 3
    };

    static Screen* getInstance();
    static void staticInit();
    static void staticDone();

    virtual void setMode(ScreenMode mode, bool justOnce = false, QString from = "UNKNOWN") = 0;
    virtual void setMode(ScreenMode mode, bool justOnce = false, int refreshFlags = 0, QString from = "UNKNOWN") = 0;
    virtual void restoreMode() = 0;
    virtual void refreshScreen(ScreenMode mode = MODE_SAFE, QString from = "UNKNOWN") = 0;
    virtual void refreshScreen(ScreenMode mode, int flags, QString from = "UNKNOWN") = 0;
    virtual void setUpdateScheme(SchemeMode,bool) = 0;
    virtual void lockScreen() = 0;
    virtual void releaseScreen() = 0;
    virtual void resetQueue() = 0;
    virtual void resetFlushQueue() = 0;
    virtual void queueUpdates() = 0;
    virtual void flushUpdates() = 0;
    virtual void flushImage(QImage&,const QPoint&) = 0;
    virtual void flushColorScreen(Qt::GlobalColor = Qt::white) = 0;
    inline ScreenMode getCurrentMode() { return _currentMode; }
    inline SchemeMode getCurrentUpdateScheme() { return _currentUpdateScheme; }
    virtual int screenWidth() const = 0;
    virtual int screenHeight() const = 0;
    inline ScreenOrientation screenOrentation() {return _screenOrientation;}
    virtual void setScreenOrentation(ScreenOrientation orientation) = 0;

protected:
    Screen();
    Screen::ScreenMode _currentMode;
    Screen::SchemeMode _currentUpdateScheme;
    Screen::ScreenOrientation  _screenOrientation;

private:
    static Screen* _instance;    
};

#endif // SCREEN_H
