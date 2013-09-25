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

#include "LibraryImageFullScreen.h"

#include "QBookApp.h"
#include "GestureWidget.h"
#include "MouseFilter.h"
#include "bqUtils.h"

#include <QPainter>
#include <QDebug>
#include <QMouseEvent>

#define MAX_STRING_LENGTH 55

LibraryImageFullScreen::LibraryImageFullScreen( QWidget* parent ) :
    GestureWidget(parent)
{
    setupUi(this);
    connect(btnImageBack,           SIGNAL(clicked()),      this, SLOT(previousImage()));
    connect(btnImageNext,           SIGNAL(clicked()),      this, SLOT(nextImage()));
    connect(QBookApp::instance(),   SIGNAL(swipe(int)),     this, SLOT(handleSwipe(int)));

    connect(closeBtn,               SIGNAL(clicked()),      this, SIGNAL(hideMe()));
}

LibraryImageFullScreen::~LibraryImageFullScreen()
{}

void LibraryImageFullScreen::setup( QString path, int currentImage, int countFolderImages)
{
    qDebug() << Q_FUNC_INFO << " setup: " << path;
    QFileInfo file(path);
    imageTitle->setText(bqUtils::truncateStringToLength(file.fileName(), MAX_STRING_LENGTH));
    currentImagePath = path;

    QString pager = QString(QString::number(currentImage) + "/" + QString::number(countFolderImages));
    pageLabel->setText(pager);

    if(countFolderImages > 1)
    {
        btnImageBack->setEnabled(true);
        btnImageBack->show();
        btnImageNext->setEnabled(true);
        btnImageNext->show();
    }
    else
    {
        btnImageNext->setEnabled(false);
        btnImageNext->hide();
        btnImageBack->setEnabled(false);
        btnImageBack->hide();
    }

    imageContent->setPixmap(NULL);
    QPixmap pixmap(currentImagePath);

    qDebug() << Q_FUNC_INFO << " width: " << pixmap.width() << "  height: " << pixmap.height();
    qDebug() << Q_FUNC_INFO << " width: " << imageContent->size();
    if(pixmap.width() > pixmap.height())
    {
        QMatrix rm;
        rm.rotate(90);
        pixmap = pixmap.transformed(rm);
    }

    imageContent->setAlignment(Qt::AlignCenter);
    imageContent->setPixmap(pixmap.scaled(imageContent->size(),Qt::KeepAspectRatio));
}

void LibraryImageFullScreen::previousImage()
{
    qDebug() << Q_FUNC_INFO;
    emit previousImageRequest(currentImagePath);
}

void LibraryImageFullScreen::nextImage()
{
    qDebug() << Q_FUNC_INFO;
    emit nextImageRequest(currentImagePath);
}

void LibraryImageFullScreen::handleSwipe(int direction)
{
    if(!isVisible())
        return;

    qDebug()<< Q_FUNC_INFO << "Direction" << direction;

    switch(direction)
    {
    case MouseFilter::SWIPE_R2L:
    case MouseFilter::SWIPE_D2U:
        if(btnImageNext->isEnabled())
            nextImage();
        break;

    case MouseFilter::SWIPE_L2R:
    case MouseFilter::SWIPE_U2D:
        if(btnImageBack->isEnabled())
            previousImage();
        break;
    default:
        break;
    }
}

void LibraryImageFullScreen::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
