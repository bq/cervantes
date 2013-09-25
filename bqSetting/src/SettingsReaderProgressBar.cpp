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

#include "SettingsReaderProgressBar.h"
#include "QBook.h"

SettingsReaderProgressBar::SettingsReaderProgressBar(QWidget *parent)  : FullScreenWidget(parent)
{
        setupUi(this);

        /* Please, keep the same order than interface */

        connect(backBtn,SIGNAL(clicked()), this, SIGNAL(hideMe()));
        connect(withPageBtn, SIGNAL(clicked()), this, SLOT(withPageBtnHandler()));
        connect(withProgressBtn, SIGNAL(clicked()),this, SLOT(withProgressBtnHandler()));
        connect(withProgressBtn_2, SIGNAL(clicked()), this, SLOT(withChapterBtnHandler()));
        connect(withProgressBarBtn, SIGNAL(clicked()), this, SLOT(withProgressBarHandler()));
        connect(withBookTitleBtn, SIGNAL(clicked()), this, SLOT(withBookTitleHandler()));
        connect(withDateTimeBtn, SIGNAL(clicked()), this, SLOT(withDateTimHandler()));

        /**********************************************/

        setWithPageBtn();
        setWithProgressBtn();
        setWithChapterBtn();
        setWithProgressBarBtn();
        setWithBookTitleBtn();
        setWithDateTimeBtn();
    }

SettingsReaderProgressBar::~SettingsReaderProgressBar()
{

}


void SettingsReaderProgressBar::setWithPageBtn()
{
        qDebug() << Q_FUNC_INFO;

        withPageBtn->setChecked(QBook::settings().value("setting/showPage", true).toBool());
}

void SettingsReaderProgressBar::setWithProgressBtn()
{
        qDebug() << Q_FUNC_INFO;

        withProgressBtn->setChecked(QBook::settings().value("setting/showProgress", true).toBool());
}

void SettingsReaderProgressBar::setWithChapterBtn()
{
        qDebug() << Q_FUNC_INFO;

        withProgressBtn_2->setChecked(QBook::settings().value("setting/showChapterInfo", true).toBool());
}

void SettingsReaderProgressBar::setWithProgressBarBtn()
{
        qDebug() << Q_FUNC_INFO;

        withProgressBarBtn->setChecked(QBook::settings().value("setting/showProgressBar", false).toBool());
}

void SettingsReaderProgressBar::setWithBookTitleBtn()
{
        qDebug() << Q_FUNC_INFO;

        withBookTitleBtn->setChecked(QBook::settings().value("setting/showBookTitle", false).toBool());
}

void SettingsReaderProgressBar::setWithDateTimeBtn()
{
        qDebug() << Q_FUNC_INFO;

        withDateTimeBtn->setChecked(QBook::settings().value("setting/showDateTime", false).toBool());
}

void SettingsReaderProgressBar::withProgressBtnHandler()
{
        qDebug() << Q_FUNC_INFO;

        bool showProgress = QBook::settings().value("setting/showProgress", true).toBool();
        QBook::settings().setValue("setting/showProgress", !showProgress);
        QBook::settings().sync();
        setWithProgressBtn();

}

void SettingsReaderProgressBar::withPageBtnHandler()
{
        qDebug() << Q_FUNC_INFO;

        bool showPage = QBook::settings().value("setting/showPage", true).toBool();
        QBook::settings().setValue("setting/showPage", !showPage);
        QBook::settings().sync();
        setWithPageBtn();

}

void SettingsReaderProgressBar::withChapterBtnHandler()
{
        qDebug() << Q_FUNC_INFO;

        bool showChapterInfo = QBook::settings().value("setting/showChapterInfo", true).toBool();
        QBook::settings().setValue("setting/showChapterInfo", !showChapterInfo);
        QBook::settings().sync();
        setWithChapterBtn();

}

void SettingsReaderProgressBar::withProgressBarHandler()
{
        qDebug() << Q_FUNC_INFO;

        bool showProgressBar = QBook::settings().value("setting/showProgressBar", false).toBool();
        QBook::settings().setValue("setting/showProgressBar", !showProgressBar);
        QBook::settings().sync();
        setWithProgressBarBtn();

}

void SettingsReaderProgressBar::withBookTitleHandler()
{
        qDebug() << Q_FUNC_INFO;

        bool showBookTitle = QBook::settings().value("setting/showBookTitle", false).toBool();
        QBook::settings().setValue("setting/showBookTitle", !showBookTitle);
        QBook::settings().sync();
        setWithBookTitleBtn();

}

void SettingsReaderProgressBar::withDateTimHandler()
{
        qDebug() << Q_FUNC_INFO;

        bool showDateTime = QBook::settings().value("setting/showDateTime", false).toBool();
        QBook::settings().setValue("setting/showDateTime", !showDateTime);
        QBook::settings().sync();
        setWithDateTimeBtn();

}

void SettingsReaderProgressBar::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }
