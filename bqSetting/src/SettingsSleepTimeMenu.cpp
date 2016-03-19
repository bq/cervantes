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

#include "SettingsSleepTimeMenu.h"
#include "QBook.h"
#include "QBookApp.h"
#include "PowerManager.h"
#include "PowerManagerDefs.h"

SettingsSleepTimeMenu::SettingsSleepTimeMenu(QWidget *parent) : FullScreenWidget(parent)
{
    setupUi(this);

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->addButton(minutes2Btn, 2);
    m_buttonGroup->addButton(minutes5Btn, 5);
    m_buttonGroup->addButton(minutes10Btn,10);
    m_buttonGroup->addButton(minutes20Btn,20);

    connect(backBtn,SIGNAL(clicked()), this, SIGNAL(hideMe()));
    connect(m_buttonGroup, SIGNAL(buttonClicked(int)),this, SLOT(changeSleepTime(int)));

    setCurrentSleepTime();
}

SettingsSleepTimeMenu::~SettingsSleepTimeMenu()
{
    delete m_buttonGroup;
    m_buttonGroup = NULL;
}

void SettingsSleepTimeMenu::setCurrentSleepTime()
{
    int sleepTime = QBook::settings().value("setting/sleepTimeInSecs", POWERMANAGER_TIME_AUTOSLEEPSECS).toInt();
    sleepTime/=60;
    QAbstractButton *button = m_buttonGroup->button(sleepTime);
    if (button)
    {
        button->setStyleSheet("background-image: url(':/res/check_ico.png');");
        button->setChecked(true);
    }
    else
    {
        minutes5Btn->setChecked(true);
        minutes5Btn->setStyleSheet("background-image: url(':/res/check_ico.png');");
    }
}

void SettingsSleepTimeMenu::changeSleepTime(int time)
{
    qDebug() << Q_FUNC_INFO << ": time selected = " << time;

    QBook::settings().setValue("setting/sleepTimeInSecs", time*60);
    QBook::settings().sync();

    int size = m_buttonGroup->buttons().size();
    for(int i = 0; i < size; i++)
    {
        if(m_buttonGroup->id(m_buttonGroup->buttons().at(i)) == time)
        {
            m_buttonGroup->buttons().at(i)->setStyleSheet("background-image: url(':/res/check_ico.png');");
            m_buttonGroup->buttons().at(i)->setChecked(true);
        }
        else
            m_buttonGroup->buttons().at(i)->setStyleSheet("background-image: none");
    }

    PowerManager::getInstance()->reviewAutoFrequencies();
    QBookApp::instance()->restartWifiSleepTimer();
}

void SettingsSleepTimeMenu::paintEvent (QPaintEvent *)
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }
