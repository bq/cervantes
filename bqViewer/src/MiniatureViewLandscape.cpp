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

#include "MiniatureViewLandscape.h"

#include <QDebug>
#include <QPainter>

#include "QBookApp.h"
#include "Viewer.h"

#define PAGE_FORMAT QString("%1/%2")
//const QString PAGE_FORMAT("%1/%2");
const QString PERCENT_FORMAT("%1%");

MiniatureViewLandscape::MiniatureViewLandscape(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    frame->hide();
}

MiniatureViewLandscape::~MiniatureViewLandscape()
{
    qDebug() << Q_FUNC_INFO;
}

void MiniatureViewLandscape::paintEvent (QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void MiniatureViewLandscape::setFrameGeometry(const double xP, const double yP, const double scale)
{
    // TODO: transformar a modo apaisado.

    int x = screenshotLbl->x() + xP * screenshotLbl->width();
    int y = screenshotLbl->y() + yP * screenshotLbl->height();
    int w = screenshotLbl->width()  /* *0.9 */  /scale;
    int h = screenshotLbl->height() /* *0.9 */ /scale;
    frame->setGeometry(x, y, w, h);

    // FIXME: El frame no se ajusta 100% bien posiblemente a causa del
    // ajuste proporcionado por el 0.9:
    // Estimar en casos se necesita y aplicarlo.

    // FIXME: También necesitamos ajustar la forma de miniature a la del texto.
}

void MiniatureViewLandscape::setPageChanged(int page, int, int total)
{
    /// FORMAT: page/total
    pageValueLbl->setText(PAGE_FORMAT.arg(page).arg(total));
}

void MiniatureViewLandscape::setZoomChanged(double newZoom, double /*minZoom*/)
{
    /// FORMAT: zoom%
    zoomValueLbl->setText(PERCENT_FORMAT.arg((int)(newZoom*100)));
}

void MiniatureViewLandscape::updatePdfMiniatureScreenshot(QPixmap& screenshot)
{
    if (screenshot.isNull()) return;
    screenshotLbl->setPixmap(screenshot.scaled(screenshotLbl->size()));
}
