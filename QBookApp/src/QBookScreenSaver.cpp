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
#include "ADConverter.h"

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
    switch(QBook::getInstance()->getResolution())
    {
        case QBook::RES1072x1448:
            font.setPointSize(6);
            break;
        case QBook::RES758x1024:
            font.setPointSize(16);
            break;
        case QBook::RES600x800: default:
            font.setPointSize(14);
            break;
    }

    painter.setFont(font);
    QColor color(255, 255, 255);
    QBrush brush(color);
    painter.setBackground(brush);

    switch (screenType) {
    case SLEEP:
    {
        qDebug() << Q_FUNC_INFO << "Enter sleep";

        // Check charging
        if(ADConverter::getInstance()->getStatus() != ADConverter::ADC_STATUS_NO_WIRE)
        {
            showChargingImage(&painter);
            event->accept();
            return;
        }

        // Check battery
        if(checkAndShowBatteryWarnings(&painter))
            return;

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

        if(!checkAndShowBatteryWarnings(&painter))
        {
            QString downloadedImagePath = Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("powerOffDeviceImage.png");
            #ifndef HACKERS_EDITION
            if(QBookApp::instance()->isActivated() || QBookApp::instance()->isLinked())
                painter.drawPixmap(0, 0, QPixmap(downloadedImagePath));
            else
            #endif
                painter.drawPixmap(0, 0, QPixmap(":/powerOffDeviceImage.png"));
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

bool QBookScreenSaver::checkAndShowBatteryWarnings(QPainter* painter)
{
    qDebug() << Q_FUNC_INFO;

    int level = Battery::getInstance()->getLevel();

    if(level > BATTERY_LEVEL_LOW)
        return false;

    if(level <= 0){ // Empty battery
        painter->drawPixmap(0, 0, QPixmap(":/emptyBatteryImage.png"));
    }
    else { // Low battery
       QString batteryDownloadPath = Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("lowBatteryImage.png");
       #ifndef HACKERS_EDITION
       if(QBookApp::instance()->isActivated() || QBookApp::instance()->isLinked())
            painter->drawPixmap(0, 0, QPixmap(batteryDownloadPath));
       else
       #endif
            painter->drawPixmap(0, 0, QPixmap(":/lowBatteryImage.png"));
    }

    if(screenType == SLEEP){        
        QString message = tr("Press the Power button to resume");
        addBatteryLevel(painter, level);
       addFooterMessage(painter, message);
    }

    return true;
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

    QString message = tr("Press the Power button to resume");
    addFooterMessage(painter, message);
}

void QBookScreenSaver::showGeneralImage(QPainter* painter, const QString& imagePath){
    qDebug() << Q_FUNC_INFO;

    if (imagePath == "" || imagePath == ":/res/unknow_book.png")
        showDefaultSleepImage(painter);
    else
    {
        int x = 0;
        int y = 0;

        QPixmap pixmap(imagePath);
        if(pixmap.width() > pixmap.height())
        {
            QMatrix rm;
            rm.rotate(90);
            pixmap = pixmap.transformed(rm);
        }

        QSize size(Screen::getInstance()->screenWidth(), Screen::getInstance()->screenHeight());
        pixmap = pixmap.scaled(size,Qt::KeepAspectRatioByExpanding);

        if(Screen::getInstance()->screenWidth() > pixmap.width())
            x = (Screen::getInstance()->screenWidth() - pixmap.width()) / 2;

        if(Screen::getInstance()->screenHeight() > pixmap.height())
            y = (Screen::getInstance()->screenHeight() - pixmap.height()) / 2;

        painter->drawPixmap(x, y, pixmap);

        // Show at the center
        QRect t = QRect(0, Screen::getInstance()->screenHeight() - Screen::getInstance()->screenHeight() / 15, Screen::getInstance()->screenWidth(), Screen::getInstance()->screenHeight() / 10);
        painter->drawText(t, Qt::AlignCenter, tr("Press the Power button to resume"));
    }
}

void QBookScreenSaver::showChargingImage(QPainter* painter)
{
    qDebug() << Q_FUNC_INFO;

    QString message = tr("Press the Power button to resume");
    int batteryLevel = Battery::getInstance()->getLevel();

    if(batteryLevel == 100)
        painter->drawPixmap(0, 0, QPixmap(":/batteryCompleteImage.png"));
    else
        painter->drawPixmap(0, 0, QPixmap(":/batteryChargingImage.png"));

    addBatteryLevel(painter, batteryLevel);
    addFooterMessage(painter, message);
}

QString QBookScreenSaver::getScreenSaverImgName()
{
    qDebug() << Q_FUNC_INFO;

    QString scDirPath = qgetenv("ADOBE_SI_FILE_FOLDER");
    QDir checkDir;

    if(!checkDir.exists(scDirPath))
        checkDir.mkpath(scDirPath);

    if(formatsList.isEmpty())
    {
        supportedImageslist = QBookApp::instance()->getSupportedImageslist();
        QList<QByteArray>::iterator it = supportedImageslist.begin();
        QList<QByteArray>::iterator itEnd = supportedImageslist.end();
        while (it != itEnd)
        {
            formatsList += QString("*." + (*it) + " ");
            it++;
        }
    }

    QDir scdir(scDirPath,formatsList);
    QStringList scList = scdir.entryList();
    int scTotalImages = scList.count();

    if(scTotalImages==0)
        return QString("");

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

void QBookScreenSaver::addFooterMessage(QPainter* painter, QString message)
{
    qDebug() << Q_FUNC_INFO << message;

    // Place the message aligned with standard sleep image
    switch(QBook::getInstance()->getResolution())
    {
        case QBook::RES1072x1448:
            painter->drawText(QPoint(198, 1414), message);
            break;
        case QBook::RES758x1024:
            painter->drawText(QPoint(140, 1000), message);
            break;
        case QBook::RES600x800: default:
            painter->drawText(QPoint(111, 782), message);
            break;
    }
}

void QBookScreenSaver::addBatteryLevel(QPainter* painter, int batteryLevel)
{
    qDebug() << Q_FUNC_INFO << batteryLevel;

    QString message = QString::number(batteryLevel) + "%";

    // Place the message right below the battery
    QRect t;
    switch(QBook::getInstance()->getResolution())
    {
        case QBook::RES1072x1448:
            t = QRect(78, 1400, 57, 25);
            break;
        case QBook::RES758x1024:
            t = QRect(51, 990, 50, 16);
            break;
        case QBook::RES600x800: default:
            t = QRect(37, 772, 40, 18);
            break;
    }

    painter->drawText(t, Qt::AlignCenter, message);
}

