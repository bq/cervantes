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

#include <QPainter>
#include <QDebug>
#include <QKeyEvent>
#include <QLabel>

#include "SettingsAboutUs.h"
#include "DeviceInfo.h"
#include "QBook.h"

SettingsAboutUs::SettingsAboutUs(QWidget *parent) : FullScreenWidget(parent)
{
        setupUi(this);
        connect(backBtn,SIGNAL(clicked()), this,SIGNAL(hideMe()));
        QHash<QString, QString> specificInfo = DeviceInfo::getInstance()->getSpecificDeviceInfo();
        QString version = specificInfo.value("Software version");
        version = version.split("-").last();
        QString dateReleased = specificInfo.value("Internal Revision");
        QStringList fieldVersion = dateReleased.split("_");
        fieldVersion.removeLast();
        dateReleased = fieldVersion.last();
        QString date = dateReleased.right(2);
        QString month = dateReleased.mid(4,2);
        QString year = dateReleased.left(4);
        versionTitleLbl->setText(tr("Version %1 - %2/%3/%4").arg(version).arg(date).arg(month).arg(year));
}

SettingsAboutUs::~SettingsAboutUs()
{
}

void SettingsAboutUs::keyReleaseEvent(QKeyEvent *event){

        qDebug() << "--->" << Q_FUNC_INFO;
        if( event->key() == QBook::QKEY_BACK )
        {
            qDebug() << ": Closing...";
            emit hideMe();
            event->accept();
            return;
        }
        QWidget::keyReleaseEvent(event);
}

void SettingsAboutUs::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}


