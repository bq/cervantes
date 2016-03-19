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

#include "WizardDateTimeItem.h"
#include <QDebug>
#include <QPainter>
#include "Screen.h"

WizardDateTimeItem::WizardDateTimeItem (QWidget* parent) : GestureWidget(parent){
        qDebug() << "--->" << Q_FUNC_INFO;
        connect(this, SIGNAL(tap()), this, SLOT(handleTap()));
        setupUi(this);
}

WizardDateTimeItem::~WizardDateTimeItem (){
        qDebug() << "--->" << Q_FUNC_INFO;
}

void WizardDateTimeItem::paint (TimeZone* _timeZone){
        qDebug() << "--->" << Q_FUNC_INFO;
        timeZone = _timeZone;

        timeZoneCountriesLbl->setText(timeZone->description);
        timeZoneCountriesLbl->show();
        gtmLbl->setText(timeZone->timeZone);
        gtmLbl->show();
        if(timeZone->defaultTimeZone)
            checkTimezoneLbl->show();
        else
            checkTimezoneLbl->hide();

}

void WizardDateTimeItem::hideLbl (){
        qDebug() << "--->" << Q_FUNC_INFO;
        timeZone = 0;

        timeZoneCountriesLbl->hide();
        gtmLbl->hide();
        checkTimezoneLbl->hide();

}

void WizardDateTimeItem::handleTap ( ){
        qDebug() << "--->" << Q_FUNC_INFO;
        if(timeZone != 0) {
            Screen::getInstance()->queueUpdates();
            checkTimezoneLbl->show();
            Screen::getInstance()->flushUpdates();
            emit setTimeZone(timeZone);
        }
}

void WizardDateTimeItem::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }
