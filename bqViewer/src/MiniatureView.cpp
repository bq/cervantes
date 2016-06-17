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

#include "MiniatureView.h"

#include <QDebug>
#include <QPainter>

#include "QBookApp.h"
#include "Viewer.h"

#define BORDER_OFFSET           2
#define FRAME_WIDTH             2
#define SCREENSHOT_MARGIN       6

#define PAGE_FORMAT QString("%1/%2")
//const QString PAGE_FORMAT("%1/%2");
const QString PERCENT_FORMAT("%1%");

MiniatureView::MiniatureView(QWidget *parent) :
    QWidget(parent)
  , m_aspectRatio(0)
{
    setupUi(this);

    /// Values from viewer_styles.qss -> #miniatureTopSpacer
    if (QBook::getInstance()->getResolution() == QBook::RES1072x1448)
        WIDGET_ASPECT_RATIO = 226.0 / 267.0;
    else if (QBook::getInstance()->getResolution() == QBook::RES600x800)
        WIDGET_ASPECT_RATIO = 125.0 / 148.0;
    else
        WIDGET_ASPECT_RATIO = 160.0 / 189.0;
}

MiniatureView::~MiniatureView()
{
    qDebug() << Q_FUNC_INFO;
}

void MiniatureView::paintEvent (QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void MiniatureView::changeAspectRatio(QRectF docRect)
{
    double w = 0.0, h = 0.0;
    int xOff = BORDER_OFFSET,
        yOff = 0,
        maxWidth  = miniatureTopSpacer->width(),
        maxHeight = miniatureTopSpacer->height();

    /// WIDTH:HEIGHT
    m_aspectRatio = (double)docRect.width() / (double)docRect.height();
    if (m_aspectRatio > WIDGET_ASPECT_RATIO)
    {
        w = maxWidth - 2*SCREENSHOT_MARGIN;
        h = w/m_aspectRatio;
    }
    else
    {
        h = maxHeight - 2*SCREENSHOT_MARGIN;
        w = h*m_aspectRatio;
    }

    // Centering.
    xOff += (maxWidth  - w)/2;
    yOff += (maxHeight - h)/2;

    screenshotLbl->setGeometry(xOff, yOff, w, h);
}

void MiniatureView::setFrameGeometry(const double xoP, const double yoP, const double xfP, const double yfP)
{
    QPoint oP = screenshotLbl->mapTo(this, QPoint(0, 0));
    int x = oP.x() + xoP * screenshotLbl->width();
    int y = oP.y() + yoP * screenshotLbl->height();
    int w = (xfP-xoP) * screenshotLbl->width();
    int h = (yfP-yoP) * screenshotLbl->height();

    if (w+4 <= screenshotLbl->width()  && xfP != 1) w += 4;
    if (h+2 <= screenshotLbl->height() && yfP != 1) h += 2;

    frame->setGeometry(x, y, w, h);
}

void MiniatureView::setPageChanged(int page, int, int total)
{
    /// FORMAT: page/total    
    pageValueLbl->setText(PAGE_FORMAT.arg(page).arg(total));
}

void MiniatureView::setZoomChanged(double newZoom)
{
    /// FORMAT: zoom%
    zoomValueLbl->setText(PERCENT_FORMAT.arg((int)(newZoom*100)));
}

void MiniatureView::updatePdfMiniatureScreenshot(QPixmap& screenshot)
{
    if (screenshot.isNull()) return;
    m_screenshot = screenshot;
    screenshotLbl->setPixmap(screenshot.scaled(screenshotLbl->size()));
}
