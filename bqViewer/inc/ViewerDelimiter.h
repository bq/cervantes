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

#ifndef VIEWERDELIMITER_H
#define VIEWERDELIMITER_H

#include "ui_ViewerDelimiter.h"

class QWidget;

class ViewerDelimiter : public QWidget,
                        protected Ui::ViewerDelimiter
{
    Q_OBJECT

public:
    explicit ViewerDelimiter(QWidget *parent = 0);
    
    void            showIn            (const QRect&, int const highlightHeight, bool const isOnTheLeft);
    QPoint          getHighlightPoint ();

protected:
    virtual void    paintEvent        (QPaintEvent* event);
    virtual void    mousePressEvent   (QMouseEvent* event);
    virtual void    mouseReleaseEvent (QMouseEvent* event);
    virtual void    mouseMoveEvent    (QMouseEvent* event);

signals:

    void pressEvent         (QPoint);
    void moveEvent          (QPoint);
    void releaseEvent       (QPoint);

public slots:

    void viewUpperMarginUpdate(int);

protected:
    void place();

    QRect       m_bbox;
    QPoint      m_grabPoint;
    QPoint      m_highlightPoint;
    QPoint      m_topLeftOffsetToText;
    int         m_highlightHeight;
    static int  m_viewUpperMargin;
    bool        m_isOnTheLeft;
    float       m_FineAdjustFactor;
    
};

#endif // VIEWERDELIMITER_H
