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

#include <QDebug>
#include <QKeyEvent>
#include <QPainter>

#include "SettingsReaderPageTurning.h"
#include "QBook.h"

SettingsReaderPageTurning::SettingsReaderPageTurning(QWidget *parent) : FullScreenWidget(parent)
  , m_buttonGroupTurnMode(NULL)
{
            setupUi(this);
            m_buttonGroupTurnMode = new QButtonGroup(this);
            m_buttonGroupTurnMode->addButton(upDownCheck,0);
            m_buttonGroupTurnMode->addButton(leftRightCheck, 1);

            m_buttonGroupRefreshCount = new QButtonGroup(this);
            m_buttonGroupRefreshCount->addButton(refreshSteps0, 1);
            m_buttonGroupRefreshCount->addButton(refreshSteps1, 2);
            m_buttonGroupRefreshCount->addButton(refreshSteps2, 3);
            m_buttonGroupRefreshCount->addButton(refreshSteps3, 4);
            m_buttonGroupRefreshCount->addButton(refreshSteps4, 5);
            m_buttonGroupRefreshCount->addButton(refreshSteps5, 6);
            m_buttonGroupRefreshCount->addButton(refreshSteps6, 10);

            /* Please, keep the same order than interface */

            connect(backBtn,SIGNAL(clicked()), this, SIGNAL(hideMe()));
            connect(m_buttonGroupTurnMode, SIGNAL(buttonClicked(int)),this, SLOT(changeTurnMode(int)));
            connect(m_buttonGroupRefreshCount, SIGNAL(buttonClicked(int)),this, SLOT(changeRefreshCount(int)));
            connect(enableSwipeBtn, SIGNAL(clicked()), this, SLOT(enableSwipeBtnHandler()));

            /**********************************************/

            setCurrentTurnMode();
            setEnableSwipeMode();
            setRefreshCount();


}

SettingsReaderPageTurning::~SettingsReaderPageTurning()
{

        delete m_buttonGroupTurnMode;
        m_buttonGroupTurnMode = NULL;

}

void SettingsReaderPageTurning::setRefreshCount()
{
        qDebug() << Q_FUNC_INFO;

        int refreshCount = QBook::settings().value("settings/viewer/maxRefresh", 5).toInt();
        m_buttonGroupRefreshCount->button(refreshCount)->setChecked(true);

}

void SettingsReaderPageTurning::setCurrentTurnMode()
{
        qDebug() << Q_FUNC_INFO;

        TurnPageMode turnMode = SettingsReaderPageTurning::getCurrentTurnPageMode();

        if(turnMode == MODE_TWOHANDS)
            leftRightCheck->setChecked(true);
        else
            upDownCheck->setChecked(true);
}

void SettingsReaderPageTurning::setEnableSwipeMode()
{
        qDebug() << Q_FUNC_INFO;

        enableSwipeBtn->setChecked(QBook::settings().value("setting/enableSwipe", true).toBool());
}

void SettingsReaderPageTurning::enableSwipeBtnHandler()
{
        qDebug() << Q_FUNC_INFO;

        bool enableSwipe = QBook::settings().value("setting/enableSwipe", true).toBool();
        QBook::settings().setValue("setting/enableSwipe", !enableSwipe);
        QBook::settings().sync();
        setEnableSwipeMode();
}

void SettingsReaderPageTurning::changeTurnMode(int buttonId)
{
        qDebug() << Q_FUNC_INFO;

        QString turnMode;

        if(buttonId == 0) {
            turnMode = "MODE_ONEHAND";
            upDownCheck->setChecked(true);
         } else {
            turnMode = "MODE_TWOHANDS";
            leftRightCheck->setChecked(true);
        }

        QBook::settings().setValue("setting/turnPageMode", turnMode);
        QBook::settings().sync();
}

void SettingsReaderPageTurning::changeRefreshCount(int buttonId)
{
        qDebug() << Q_FUNC_INFO;
        QBook::settings().setValue("settings/viewer/maxRefresh", buttonId);
}

SettingsReaderPageTurning::TurnPageMode SettingsReaderPageTurning::getCurrentTurnPageMode()
 {
        qDebug() << Q_FUNC_INFO;

        QMap<QString,TurnPageMode> mapTurnMode;
        mapTurnMode["MODE_ONEHAND"] = MODE_ONEHAND;
        mapTurnMode["MODE_TWOHANDS"] = MODE_TWOHANDS;
        QString turnPageConfig = QBook::settings().value("setting/turnPageMode", "MODE_TWOHANDS").toString();
        return mapTurnMode[turnPageConfig];
 }

void SettingsReaderPageTurning::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }
