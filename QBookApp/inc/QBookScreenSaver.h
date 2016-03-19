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

#ifndef QBOOKSCREENSAVER_H
#define QBOOKSCREENSAVER_H

#include <QtGui>

class QBookScreenSaver : public QWidget
{
    Q_OBJECT


public:

    enum screenType{
        SLEEP,
        POWER_OFF,
        DEFAULT_SETTINGS
    };

    enum sleepImage{
        DEFAULT_IMAGE,
        STORED_IMAGES,
        BOOK_COVER
    };

    QBookScreenSaver(QWidget *parent = 0);
    virtual ~QBookScreenSaver();

    void setScreenType(int type);

protected:
    void showChargingImage(QPainter*);
    void paintEvent(QPaintEvent* event);
    void closeEvent(QCloseEvent *event);
    QString getScreenSaverImgName();
    void showGeneralImage(QPainter*, const QString&);
    void showDefaultSleepImage(QPainter*);
    bool checkAndShowBatteryWarnings(QPainter*);

signals:
    void closed();

private:
    int screenType;
    QString formatsList;
    QList<QByteArray> supportedImageslist;
    void addFooterMessage(QPainter*, QString);
    void addBatteryLevel(QPainter*, int);
};

#endif
