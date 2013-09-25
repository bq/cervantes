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

#include "WizardGeneralTerms.h"
#include "FullScreenWidget.h"
#include "QBook.h"

#include <QScrollBar>
#include <QWidget>
#include <QDebug>
#include <QPainter>
#include <QKeyEvent>

WizardGeneralTerms::WizardGeneralTerms(QWidget* parent): FullScreenWidget(parent)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);
    connect(this->okBtn, SIGNAL(clicked()), SIGNAL(acceptTerms()));
    elfGeneralTermsResult->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setupPagination();
}

WizardGeneralTerms::~WizardGeneralTerms()
{
    qDebug() << Q_FUNC_INFO;
}

void WizardGeneralTerms::previousPage()
{
    qDebug() << Q_FUNC_INFO;
    QScrollBar *vbar = elfGeneralTermsResult->verticalScrollBar();
    if (vbar)
    {
         vbar->triggerAction(QAbstractSlider::SliderPageStepSub);
    }
}

void WizardGeneralTerms::nextPage()
{
    qDebug() << Q_FUNC_INFO;
    QScrollBar *vbar = elfGeneralTermsResult->verticalScrollBar();
    if (vbar)
    {
         vbar->triggerAction(QAbstractSlider::SliderPageStepAdd);
    }
}

void WizardGeneralTerms::setupPagination()
{
    qDebug() << Q_FUNC_INFO;

    int pageNumber = 2;
    qDebug() << Q_FUNC_INFO << pageNumber;

}

void WizardGeneralTerms::keyReleaseEvent(QKeyEvent* event)
{
    qDebug() << Q_FUNC_INFO << event;

    FullScreenWidget::keyReleaseEvent(event);
}

void WizardGeneralTerms::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }


