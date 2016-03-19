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

#ifndef QCOMMONWEBVIEW_H
#define QCOMMONWEBVIEW_H

#include <QWebView>
#include <QResizeEvent>
#include <QMouseEvent>

class BrowserCommonWebView : public QWebView
{
	Q_OBJECT

public:

    enum WheelWay{
        WHEEL_NONE=-1,
        WHEEL_LEFT=0,
        WHEEL_RIGHT,
        WHEEL_UP,
        WHEEL_DOWN
    };

    BrowserCommonWebView(QWidget* parent = 0);
    virtual ~BrowserCommonWebView();

public slots:
    void                                    doWheels                                    ();

protected:
    virtual void                            resizeEvent                                 ( QResizeEvent* event );
    virtual void                            mousePressEvent                             ( QMouseEvent * event );
    virtual void                            mouseMoveEvent                              ( QMouseEvent * event );
    virtual void                            mouseReleaseEvent                           ( QMouseEvent * event );
    virtual void                            wheelEvent                                  ( QWheelEvent * event );
    virtual void                            keyReleaseEvent                             ( QKeyEvent   * event );
    virtual QWebView*                       createWindow ( QWebPage::WebWindowType type );

signals:
    void                                    newSize                                     ( QSize );
    void                                    newPressPos                                 ( QPoint );
    void                                    newReleasePos                               ( QPoint );
    void                                    wheel                                       ( bool ,int );

private:

    QPoint                                  mDragStartPos;
    QPoint                                  mDragEndPos;
    int                                     mDragDistance;
    int                                     mMouseDragDistance;
    QMouseEvent*                            mMousePressEvent;
    bool                                    mDragVertical;
    bool                                    alreadyMoved;
    bool                                    alreadyPressed;
    WheelWay                                getWheelWay                                 ( );
    void                                    doWheel                                     ( WheelWay way );
};
#endif
