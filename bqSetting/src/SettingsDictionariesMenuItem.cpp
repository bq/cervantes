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

#include "SettingsDictionariesMenuItem.h"
#include <QDebug>
#include <QPainter>

SettingsDictionariesMenuItem::SettingsDictionariesMenuItem (QWidget* parent) : GestureWidget(parent){
        qDebug() << "--->" << Q_FUNC_INFO;
        connect(this, SIGNAL(tap()), this, SLOT(handleTap()));
        setupUi(this);
}

SettingsDictionariesMenuItem::~SettingsDictionariesMenuItem (){
        qDebug() << "--->" << Q_FUNC_INFO;
}

void SettingsDictionariesMenuItem::paint (DictionaryParams _dictio){
        qDebug() << "--->" << Q_FUNC_INFO << "commercialName:" << _dictio.commercialName;

        dictio = _dictio;

        if(_dictio.id.isEmpty()){
            dictionaryNameLbl->hide();
            checkDictionaryLbl->hide();
            return;
        }
        else
            dictionaryNameLbl->show();

        dictionaryNameLbl->setText(_dictio.commercialName);

        if(dictio.defaultDictio)
            checkDictionaryLbl->show();
        else
            checkDictionaryLbl->hide();
}

void SettingsDictionariesMenuItem::handleTap ( ){
        qDebug() << "--->" << Q_FUNC_INFO;
        if(!dictio.id.isEmpty())
            emit selectDictionary(dictio.id);
}

void SettingsDictionariesMenuItem::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
