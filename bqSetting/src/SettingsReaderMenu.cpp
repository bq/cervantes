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

#include <QDebug>
#include <QKeyEvent>
#include <QPainter>

#include "SettingsReaderMenu.h"
#include "Settings.h"
#include "SettingsReaderPageTurning.h"
#include "SettingsReaderProgressBar.h"
#include "QBook.h"

SettingsReaderMenu::SettingsReaderMenu(QWidget *parent)  : FullScreenWidget(parent)
{
        setupUi(this);

        /* Please, keep the same order than interface */

        connect(backBtn,SIGNAL(clicked()), this, SIGNAL(hideMe()));
        connect(pageTurningBtn, SIGNAL(clicked()), this, SLOT(showReaderPageTurning()));
        connect(progressBarBtn, SIGNAL(clicked()), this, SLOT(showReaderProgressBar()));

        /**********************************************/

        settingsReaderPageTurning = new SettingsReaderPageTurning(this);
        settingsReaderProgressBar = new SettingsReaderProgressBar(this);

        settingsReaderPageTurning->hide();
        settingsReaderProgressBar->hide();

        connect(settingsReaderPageTurning, SIGNAL(hideMe()), this, SLOT(hideTopElement()));
        connect(settingsReaderProgressBar, SIGNAL(hideMe()), this, SLOT(hideTopElement()));
    }

SettingsReaderMenu::~SettingsReaderMenu()
{

}

void SettingsReaderMenu::showReaderPageTurning()
{
    qDebug() << Q_FUNC_INFO;
    ((Settings*)parent())->showElement(settingsReaderPageTurning);
}

void SettingsReaderMenu::showReaderProgressBar()
{
    qDebug() << Q_FUNC_INFO;
    ((Settings*)parent())->showElement(settingsReaderProgressBar);
}

void SettingsReaderMenu::hideTopElement(){
        qDebug() << Q_FUNC_INFO;

        ((Settings *)parent())->hideElement();
}

void SettingsReaderMenu::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }
