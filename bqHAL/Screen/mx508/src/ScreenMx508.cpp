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

#include "ScreenMx508.h"

#include <QWSServer>
#include <QWSDisplay>
#include <QScreen>
#include <QDebug>
#include <QImage>
#include <ntx.h>


ScreenMx508::ScreenMx508()
    : Screen()
{
    _qscreen = (EInkFbScreen*)QScreen::instance();
    setMode(Screen::MODE_QUICK, false, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
}

void ScreenMx508::setMode(Screen::ScreenMode mode, bool justOnce, QString from)
{
    setMode(mode, justOnce, FLAG_PARTIALSCREEN_UPDATE, from);
}


void ScreenMx508::setMode(Screen::ScreenMode mode, bool justOnce, int refreshFlags, QString from)
{
    qDebug() << "--->" << Q_FUNC_INFO << "Mode: " << mode << " once: " << justOnce << "from: " << from;

    EInkFbModes platformSpecificMode = translateMode(mode);
    _qscreen->setRefreshMode(platformSpecificMode, refreshFlags, justOnce);

    if(!justOnce)
        _currentMode = mode;
}

void ScreenMx508::restoreMode()
{
    _qscreen->restoreRefreshMode();
}

void ScreenMx508::refreshScreen(Screen::ScreenMode mode, int flags, QString from)
{
    qDebug() << "--->" << Q_FUNC_INFO << "Mode: " << mode << "from: " << from;

    if(mode == NULL)
        mode = _currentMode;

    setMode(mode, true, flags, Q_FUNC_INFO); // Sets mode for the refresh
    QWSServer::instance()->refresh();
}

void ScreenMx508::refreshScreen(Screen::ScreenMode mode, QString from)
{
    refreshScreen(mode, 0, from);
}

void ScreenMx508::setUpdateScheme(SchemeMode mode, bool justOnce)
{
    qDebug() << "--->" << Q_FUNC_INFO << mode << justOnce;

    EInkFbUpdateSchemes platformSpecificUpdateScheme = translateScheme(mode) ;
    _qscreen->setUpdateScheme(platformSpecificUpdateScheme,justOnce);
}

void ScreenMx508::lockScreen()
{
    qDebug() << Q_FUNC_INFO;
    QWSServer::instance()->enablePainting(false);
}

void ScreenMx508::releaseScreen()
{
    qDebug() << Q_FUNC_INFO;
    QWSServer::instance()->enablePainting(true);
}

void ScreenMx508::resetQueue()
{
    qDebug() << Q_FUNC_INFO;
    _qscreen->resetQueue();
}

void ScreenMx508::resetFlushQueue()
{
    qDebug() << Q_FUNC_INFO;
    _qscreen->resetFlushQueue();
}

void ScreenMx508::queueUpdates()
{
    qDebug() << Q_FUNC_INFO;
    /* Flush pending qt-updates, we only want to queue new ones */
    QCoreApplication::flush();

    _qscreen->queueUpdates();
}

void ScreenMx508::flushUpdates()
{
    qDebug() << Q_FUNC_INFO;
    /* Flush pending qt-updates */
    QCoreApplication::flush();

    /* put the whole updated region on the screen */
    _qscreen->flushUpdates();
}

/* Params:
 * globalPoint is topleft point to blit to
 */
void ScreenMx508::flushImage(QImage& image, const QPoint& globalPoint)
{
    // QRegion denotes the part of the source image that should be blitted
    // => interesting for multisprite images
    QRect r(globalPoint,image.size());
    QRegion regionFromImage = QRegion(r);
    QScreen::instance()->blit(image, globalPoint, regionFromImage);
    QScreen::instance()->setDirty(r);
}

void ScreenMx508::flushColorScreen(Qt::GlobalColor color)
{
    QImage imageColor(screenWidth(),screenHeight(), QImage::Format_RGB32);
    imageColor.fill(color);
    flushImage(imageColor,QPoint(0,0));
}

/*virtual*/ int ScreenMx508::screenWidth() const
{
    return _qscreen->width();
}

/*virtual*/ int ScreenMx508::screenHeight() const
{
    return _qscreen->height();
}

/*virtual*/ void ScreenMx508::setScreenOrentation(ScreenOrientation orientation)
{
    _screenOrientation = orientation;
    qDebug() << Q_FUNC_INFO << orientation;
    QWSDisplay::setTransformation(_screenOrientation);
}


EInkFbModes ScreenMx508::translateMode(Screen::ScreenMode mode)
{
    EInkFbModes platformSpecificMode;

    switch(mode){

    case Screen::MODE_FASTEST:
        platformSpecificMode = MODE_EINK_FASTEST;
        break;
    case Screen::MODE_AUTO:
        platformSpecificMode = MODE_EINK_AUTO;
        break;
    case Screen::MODE_BLOCK:
        platformSpecificMode = MODE_EINK_BLOCK;
        break;
    case Screen::MODE_SAFE:
        platformSpecificMode = MODE_EINK_SAFE;
        break;
    case Screen::MODE_QUICK:
        platformSpecificMode = MODE_EINK_QUICK;
        break;
    case Screen::MODE_FAST:
        platformSpecificMode = MODE_EINK_FAST;
        break;
    default:
        platformSpecificMode = MODE_EINK_QUICK;
        break;
    }

    return platformSpecificMode;
}

EInkFbUpdateSchemes ScreenMx508::translateScheme(Screen::SchemeMode mode)
{
    EInkFbUpdateSchemes platformSpecificScheme;

    switch(mode){
    case Screen::SCHEME_QUEUE:
        platformSpecificScheme = SCHEME_EINK_QUEUE;
        break;
    case Screen::SCHEME_MERGE:
    default:
        platformSpecificScheme = SCHEME_EINK_MERGE;
        break;
    }

    return platformSpecificScheme;
}
