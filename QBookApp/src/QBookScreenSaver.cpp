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

#include <QtGui>
#include <QtCore>
#include <QStringList>
#include <QImage>
#include <QTime>

#include "QBook.h"
#include "QBookScreenSaver.h"
#include "PowerManager.h"
#include "Battery.h"
#include "Screen.h"
#include "Storage.h"
#include "QBookApp.h"
#include "Model.h"
#include "BookInfo.h"
#include "DeviceInfo.h"

QBookScreenSaver::QBookScreenSaver(QWidget *parent)
        : QWidget(parent)
        , screenType(0)
{
    qDebug() << Q_FUNC_INFO << parent;
}

QBookScreenSaver::~QBookScreenSaver()
{}

void QBookScreenSaver::setScreenType(int type)
{
    screenType = type;
}

void QBookScreenSaver::paintEvent(QPaintEvent *event)
{
    qDebug() << Q_FUNC_INFO;

    QPainter painter(this);
    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.fillRect( 0, 0, Screen::getInstance()->screenWidth(), Screen::getInstance()->screenHeight(), Qt::white);

    QFont font;
    font.setFamily(QString::fromUtf8("DejaVu Sans"));
    font.setPointSize(16);
    painter.setFont(font);
    QColor color(255, 255, 255);
    QBrush brush(color);
    painter.setBackground(brush);

    switch (screenType) {
    case SLEEP:
    {
        qDebug() << Q_FUNC_INFO << "Enter sleep";
        switch (QBook::settings().value("setting/screensaver", BOOK_COVER).toInt()){
        case STORED_IMAGES:
        {
            QString randomImage = getScreenSaverImgName();

            if (!randomImage.isEmpty())
                showGeneralImage(&painter,randomImage);
            else
                showDefaultSleepImage(&painter);
            break;
        }

        case DEFAULT_IMAGE:
        {
            showDefaultSleepImage(&painter);
            break;
        }

        case BOOK_COVER:
        {
            // TODO: Extra checks like file existance, etc...
            showGeneralImage(&painter,QBook::settings().value("setting/screensaverCoverToShow", "").toString());
            break;
        }
        default:
            showDefaultSleepImage(&painter);
        }
        break;
    }

    case POWER_OFF:
    {
        qDebug() << Q_FUNC_INFO << "powerOff";

        int level = Battery::getInstance()->getLevel();

        if( level > BATTERY_LEVEL_LOW)
        {
            QString downloadedImagePath = Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("powerOffDeviceImage.png");
            #ifndef HACKERS_EDITION
            if(QBookApp::instance()->isActivated() || QBookApp::instance()->isLinked())
                painter.drawPixmap(0, 0, QPixmap(downloadedImagePath));
            else
            #endif
                painter.drawPixmap(0, 0, QPixmap(":/powerOffDeviceImage.png"));
        }
        else if(level <= 0){ // Empty battery
            painter.drawPixmap(0, 0, QPixmap(":/emptyBatteryImage.png"));
        }
        else { // Low battery
           QString batteryDownloadPath = Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("lowBatteryImage.png");
           #ifndef HACKERS_EDITION
           if(QBookApp::instance()->isActivated() || QBookApp::instance()->isLinked())
                painter.drawPixmap(0, 0, QPixmap(batteryDownloadPath));
           else
           #endif
                painter.drawPixmap(0, 0, QPixmap(":/lowBatteryImage.png"));
        }
    }
    break;

    case DEFAULT_SETTINGS:
        if(DeviceInfo::getInstance()->getHwId() == DeviceInfo::E60Q22)
            painter.drawPixmap(0, 0, QPixmap(":/factoryReset_E60Q22.png"));
        else // E606A2 or E60672
            painter.drawPixmap(0, 0, QPixmap(":/factoryReset.png"));
    break;

    event->accept();

    }
}

void QBookScreenSaver::showDefaultSleepImage(QPainter* painter){
    qDebug() << Q_FUNC_INFO;

    QString downloadedImagePath = Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("restDeviceImage.png");

    #ifndef HACKERS_EDITION
    if(QBookApp::instance()->isActivated() || QBookApp::instance()->isLinked())
        painter->drawPixmap(0, 0, QPixmap(downloadedImagePath));
    else
    #endif
        painter->drawPixmap(0, 0, QPixmap(":/restDeviceImage.png"));

    //Absolute position, specific of cliente sleep image
    if(QBook::getInstance()->getResolution() == QBook::RES758x1024)
        painter->drawText(QPoint(140, 1000), QString(QApplication::translate("QBookPowerSaver", "Press the Power button to resume", 0, QApplication::UnicodeUTF8)));
    else
        painter->drawText(QPoint(111, 782), QString(QApplication::translate("QBookPowerSaver", "Press the Power button to resume", 0, QApplication::UnicodeUTF8)));

}

void QBookScreenSaver::showGeneralImage(QPainter* painter, const QString& image){
    qDebug() << Q_FUNC_INFO;

    if (image == "" || image == ":/res/unknow_book.png")
        showDefaultSleepImage(painter);
    else{
        QRect t = QRect(0, Screen::getInstance()->screenHeight() - Screen::getInstance()->screenHeight() / 15, Screen::getInstance()->screenWidth(), Screen::getInstance()->screenHeight() / 10);

        painter->drawImage(0, 0, QImage(image).scaledToWidth(Screen::getInstance()->screenWidth()));
        painter->drawText(t, Qt::AlignCenter, QString(QApplication::translate("QBookPowerSaver", "Press the Power button to resume", 0, QApplication::UnicodeUTF8)));
    }

}

QString QBookScreenSaver::getScreenSaverImgName()
{
    qDebug() << Q_FUNC_INFO;

    QString scDirPath = qgetenv("ADOBE_SI_FILE_FOLDER");
    QDir checkDir;

    if(!checkDir.exists(scDirPath)){
        checkDir.mkpath(scDirPath);
    }

    QDir scdir(scDirPath, "*.jpg *.JPG *.jpeg *.JPEG *.png *.PNG");
    QStringList scList = scdir.entryList();
    int scTotalImages = scList.count();

    if(scTotalImages==0){
        return QString("");
    }

    qDebug() << Q_FUNC_INFO << "Found " << scTotalImages << " images.";
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
    int randomIndex = (qrand()%scTotalImages);

    qDebug() << Q_FUNC_INFO << "Random index is " << randomIndex;
    QString randomImagePath(scDirPath + "/" + scList.value(randomIndex));

    return randomImagePath;
}

void QBookScreenSaver::closeEvent(QCloseEvent *event)
{
      emit closed();
      event->accept();
}
