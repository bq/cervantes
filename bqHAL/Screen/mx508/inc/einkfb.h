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

/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://www.qtsoftware.com/contact.
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSCREENEINKFB_QWS_H
#define QSCREENEINKFB_QWS_H

#include <QtGui/qscreen_qws.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_QWS_EINKFB

class EInkFb_Shared
{
public:
    volatile int lastop;
    volatile int optype;
    volatile int fifocount;   // Accel drivers only
    volatile int fifomax;
    volatile int forecol;     // Foreground colour caching
    volatile unsigned int buffer_offset;   // Destination
    volatile int linestep;
    volatile int cliptop;    // Clip rectangle
    volatile int clipleft;
    volatile int clipright;
    volatile int clipbottom;
    volatile unsigned int rop;

};

struct fb_cmap;
struct fb_var_screeninfo;
struct fb_fix_screeninfo;
class EInkFbScreenPrivate;

enum EInkFbModes {
    MODE_EINK_AUTO,
    MODE_EINK_BLOCK,
    MODE_EINK_SAFE,     /* the same as QUICK with FLAG_REFRESH set */
    MODE_EINK_QUICK,    /* normal updates without additional constraints */
    MODE_EINK_FAST,     /* focus on speed, colordepth reduction possible */
    MODE_EINK_FASTEST,  /* highest speed available, image artifacts possible */
    MODE_EINK_TEXT      /* Mode when displaying text, only available for WC wfs (not available for 606A2) */
};

/**
 * Flags for setRefreshMode()
 * We use a bitfield for the flags to be able to easily add more flags
 */
#define FLAG_FULLSCREEN_UPDATE		(1 << 0)
#define FLAG_PARTIALSCREEN_UPDATE	0
#define FLAG_WAITFORCOMPLETION		(1 << 1)
#define FLAG_REFRESH			(1 << 2)
#define FLAG_IGNORE_QUEUE		(1 << 3)

enum EInkFbUpdateSchemes {
    SCHEME_EINK_QUEUE,
    SCHEME_EINK_MERGE
};

class Q_GUI_EXPORT EInkFbScreen : public QScreen
{
public:
    explicit EInkFbScreen(int display_id);
    virtual ~EInkFbScreen();

    virtual void exposeRegion(QRegion region, int changing);
    virtual void setDirty(const QRect&);

    virtual bool initDevice();
    virtual bool connect(const QString &displaySpec);

    virtual bool useOffscreen();

    virtual void disconnect();
    virtual void shutdownDevice();
    virtual void setMode(int,int,int);
    virtual void save();
    virtual void restore();
    virtual void blank(bool on);
    virtual void set(unsigned int,unsigned int,unsigned int,unsigned int);
    virtual uchar * cache(int);
    virtual void uncache(uchar *);
    virtual int sharedRamSize(void *);

    EInkFb_Shared * shared;

    virtual void setRefreshMode(int mode, bool justOnce);
    virtual void setRefreshMode(int mode, int newFlags, bool justOnce);
    virtual void restoreRefreshMode();
    virtual void setUpdateScheme(int newScheme, bool justOnce);
    virtual void restoreUpdateScheme();
    virtual int  setRotation(int rot);
    virtual void resetBlock();
    virtual void blockUpdates();
    virtual void unblockUpdates();
    virtual void resetQueue();
    virtual void resetFlushQueue();
    virtual void queueUpdates();
    virtual void flushUpdates();
    virtual int updateDisplay(int left, int top, int width, int height, int wave_mode,
                              int wait_for_complete, uint flags, int fullUpdates);

protected:

    void deleteEntry(uchar *);

    bool canaccel;
    int dataoffset;
    int cacheStart;

    static void clearCache(QScreen *instance, int);

    int haltUpdates;
    int haltCount;
    int queueCount;
    int currentMode;
    int currentFlags;
    int currentScheme;
    int useModeOnce;
    int useSchemeOnce;

    int previousMode;
    int previousFlags;
    int previousHalt;
    int previousScheme;

    int marker_val;
    
    QRect pendingArea;
    bool pendingPresent;

private:

    void delete_entry(int);
    void insert_entry(int,int,int);
    void setupOffScreen();
    void createPalette(fb_cmap &cmap, fb_var_screeninfo &vinfo, fb_fix_screeninfo &finfo);
    void setPixelFormat(struct fb_var_screeninfo);

	void setupController();
	void restoreController();
	
    EInkFbScreenPrivate *d_ptr;
};

#endif // QT_NO_QWS_EINKFB

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSCREENEINKFB_QWS_H
