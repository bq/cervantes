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

#include "SettingsDateTimeMenuItem.h"
#include <QDebug>
#include <QPainter>

SettingsDateTimeMenuItem::SettingsDateTimeMenuItem (QWidget* parent) : GestureWidget(parent){
        qDebug() << "--->" << Q_FUNC_INFO;
        connect(this, SIGNAL(tap()), this, SLOT(handleTap()));
        setupUi(this);
}

SettingsDateTimeMenuItem::~SettingsDateTimeMenuItem (){
        qDebug() << "--->" << Q_FUNC_INFO;
}

void SettingsDateTimeMenuItem::paint (TimeZone* _timeZone){
        qDebug() << "--->" << Q_FUNC_INFO;
        timeZone = _timeZone;

        timeZoneCountriesLbl->setText(timeZone->description);
        gtmLbl->setText(timeZone->timeZone);
        if(timeZone->defaultTimeZone)
            checkTimezoneLbl->show();
        else
            checkTimezoneLbl->hide();

}

void SettingsDateTimeMenuItem::handleTap ( ){
        qDebug() << "--->" << Q_FUNC_INFO;
        emit setTimeZone(timeZone);
}

void SettingsDateTimeMenuItem::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }
