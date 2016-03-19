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

#include "QBook.h"
#include "QBookForm.h"
#include "WizardFinish.h"
#include "FullScreenWidget.h"

#include <QWidget>
#include <QDebug>
#include <QPainter>
#include <QKeyEvent>

WizardFinish::WizardFinish(QWidget* parent):
    FullScreenWidget(parent)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);
    connect(finishStepBtn, SIGNAL(clicked()) , this, SIGNAL(closeWizard()));
}

WizardFinish::~WizardFinish()
{
    qDebug() << Q_FUNC_INFO;
}

void WizardFinish::keyReleaseEvent(QKeyEvent *event)
{
    qDebug() << Q_FUNC_INFO << event;
    if(event->key()==QBook::QKEY_MENU) {
        event->accept();
    } else if (event->key()==QBook::QKEY_BACK) {
        //goPreviousPage();
    } else {
        FullScreenWidget::keyReleaseEvent(event);
    }
}

void WizardFinish::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }
