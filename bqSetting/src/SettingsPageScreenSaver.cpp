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

#include "SettingsPageScreenSaver.h"

#include "QBook.h"

#include <QPainter>
#include <QDebug>
#include "QBookScreenSaver.h"

SettingsPageScreenSaver::SettingsPageScreenSaver(QWidget* parent) : FullScreenWidget(parent)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);

    connect(backBtn,                SIGNAL(clicked()), this, SIGNAL(hideMe()));
    connect(screenSaverDefaultBtn,  SIGNAL(clicked()), this, SLOT(handleScreenSaverDefault()));
    connect(imageRecentBookBtn,     SIGNAL(clicked()), this, SLOT(handleImageRecentBook()));
    connect(storedImagesBtn,        SIGNAL(clicked()), this, SLOT(handleStoredImages()));

    resetButtons();
    switch(QBook::settings().value("setting/screensaver", QBookScreenSaver::BOOK_COVER).toInt()){
    case QBookScreenSaver::DEFAULT_IMAGE:
        setBtnChecked(screenSaverDefaultBtn, true);
        break;
    case QBookScreenSaver::BOOK_COVER:
        setBtnChecked(imageRecentBookBtn, true);
        break;
    case QBookScreenSaver::STORED_IMAGES:
        setBtnChecked(storedImagesBtn, true);
        break;
    default:
        qWarning() << Q_FUNC_INFO << "Unexpected setting";
    }

}

SettingsPageScreenSaver::~SettingsPageScreenSaver()
{}

void SettingsPageScreenSaver::handleScreenSaverDefault()
{
    qDebug() << Q_FUNC_INFO;

    resetButtons();
    setBtnChecked(screenSaverDefaultBtn, true);

    QBook::settings().setValue("setting/screensaver", QBookScreenSaver::DEFAULT_IMAGE);
}

void SettingsPageScreenSaver::handleImageRecentBook()
{
    qDebug() << Q_FUNC_INFO;

    resetButtons();
    setBtnChecked(imageRecentBookBtn, true);

    QBook::settings().setValue("setting/screensaver", QBookScreenSaver::BOOK_COVER);
}

void SettingsPageScreenSaver::handleStoredImages()
{
    qDebug() << Q_FUNC_INFO;

    resetButtons();
    setBtnChecked(storedImagesBtn, true);

    QBook::settings().setValue("setting/screensaver", QBookScreenSaver::STORED_IMAGES);
}

void SettingsPageScreenSaver::resetButtons()
{
    qDebug() << Q_FUNC_INFO;

    setBtnChecked(imageRecentBookBtn, false);
    setBtnChecked(screenSaverDefaultBtn, false);
    setBtnChecked(storedImagesBtn, false);
}

void SettingsPageScreenSaver::paintEvent(QPaintEvent* )
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void SettingsPageScreenSaver::setBtnChecked( QPushButton* btn, bool checked )
{
    btn->setChecked(checked);
    btn->setEnabled(!checked);
}
