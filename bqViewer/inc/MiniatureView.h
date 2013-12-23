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

#ifndef MINIATUREVIEW_H
#define MINIATUREVIEW_H

#include "ui_MiniatureView.h"

class MiniatureView :  public QWidget
                      ,protected Ui::MiniatureView
{
    Q_OBJECT
public:
    MiniatureView(QWidget*);
    virtual ~MiniatureView();

    void updatePdfMiniatureScreenshot(QPixmap& screenshot);
    void setFrameGeometry(const double xoP, const double yoP, const double xfP, const double yfP);
    void setZoom(const int zoom);
    
signals:
    
public slots:

    void setPageChanged(int page, int, int total);
    void setZoomChanged(double newZoom);
    void changeAspectRatio(QRectF docRect);

protected:
    /* http://qt-project.org/forums/viewthread/7340 */
    void paintEvent (QPaintEvent *);

protected:
    double m_aspectRatio;
    QPixmap m_screenshot;
    double WIDGET_ASPECT_RATIO;
};

#endif // MINIATUREVIEW_H
