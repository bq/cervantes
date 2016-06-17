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

#include "SettingsReaderProgressBar.h"
#include "QBook.h"

#define WIDTH_LINE_FHD 3
#define WIDTH_LINE_HD 2
#define WIDTH_LINE_SD 1

SettingsReaderProgressBar::SettingsReaderProgressBar(QWidget *parent)  : FullScreenWidget(parent)
{
        setupUi(this);
        readingProgress->setStyleSheet("background:transparent;");

        switch(QBook::getInstance()->getResolution())
        {
            case QBook::RES1072x1448:
                m_chapterLineWidth = WIDTH_LINE_FHD;
                break;
            case QBook::RES758x1024:
                m_chapterLineWidth = WIDTH_LINE_HD;
                break;
            case QBook::RES600x800: default:
                m_chapterLineWidth = WIDTH_LINE_SD;
                break;
        }

        /* Please, keep the same order than interface */

        connect(backBtn,SIGNAL(clicked()), this, SIGNAL(hideMe()));
        connect(withPageBtn, SIGNAL(clicked()), this, SLOT(withPageBtnHandler()));
        connect(withProgressBtn, SIGNAL(clicked()),this, SLOT(withProgressBtnHandler()));
        connect(withProgressBtn_2, SIGNAL(clicked()), this, SLOT(withChapterBtnHandler()));
        connect(withProgressBarBtn, SIGNAL(clicked()), this, SLOT(withProgressBarHandler()));
        connect(withBookTitleBtn, SIGNAL(clicked()), this, SLOT(withBookTitleHandler()));
        connect(withDateTimeBtn, SIGNAL(clicked()), this, SLOT(withDateTimHandler()));
        connect(chapterProgressBtn, SIGNAL(clicked()), this, SLOT(withChapterLinesHandler()));

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

        withPageBtn->setChecked(QBook::settings().value("setting/showPage", QVariant(true)).toBool());
}

void SettingsReaderProgressBar::setWithProgressBtn()
{
        qDebug() << Q_FUNC_INFO;

        withProgressBtn->setChecked(QBook::settings().value("setting/showProgress", QVariant(true)).toBool());
}

void SettingsReaderProgressBar::setWithChapterBtn()
{
        qDebug() << Q_FUNC_INFO;

        withProgressBtn_2->setChecked(QBook::settings().value("setting/showChapterInfo", QVariant(true)).toBool());
}

void SettingsReaderProgressBar::setWithProgressBarBtn()
{
        qDebug() << Q_FUNC_INFO;

        withProgressBarBtn->setChecked(QBook::settings().value("setting/showProgressBar", QVariant(false)).toBool());
        if (withProgressBarBtn->isChecked())
            chapterProgressBtn->setEnabled(true);
        else
            chapterProgressBtn->setEnabled(false);
        setWithChapterLinesBtn();
}

void SettingsReaderProgressBar::setWithBookTitleBtn()
{
        qDebug() << Q_FUNC_INFO;

        withBookTitleBtn->setChecked(QBook::settings().value("setting/showBookTitle", QVariant(false)).toBool());
}

void SettingsReaderProgressBar::setWithDateTimeBtn()
{
        qDebug() << Q_FUNC_INFO;

        withDateTimeBtn->setChecked(QBook::settings().value("setting/showDateTime", QVariant(false)).toBool());
}

void SettingsReaderProgressBar::setWithChapterLinesBtn()
{
        qDebug() << Q_FUNC_INFO;

        bool checked = QBook::settings().value("setting/showChapterLines", QVariant(true)).toBool();
        chapterProgressBtn->setChecked(checked);
        if(checked)
        {
            showChapterLines();
            if(chapterProgressBtn->isEnabled())
                chapterProgressBtn->setStyleSheet("background-image: url(:/res/checked-small.png)");
            else
                chapterProgressBtn->setStyleSheet("background-image: url(:/res/checked-small-disabled.png)");
        }
        else
        {
            hideChapterLines();
            chapterProgressBtn->setStyleSheet("");
        }
}

void SettingsReaderProgressBar::withProgressBtnHandler()
{
        qDebug() << Q_FUNC_INFO;

        bool showProgress = QBook::settings().value("setting/showProgress", QVariant(true)).toBool();
        QBook::settings().setValue("setting/showProgress", QVariant(!showProgress));
        QBook::settings().sync();
        setWithProgressBtn();

}

void SettingsReaderProgressBar::withPageBtnHandler()
{
        qDebug() << Q_FUNC_INFO;

        bool showPage = QBook::settings().value("setting/showPage", QVariant(true)).toBool();
        QBook::settings().setValue("setting/showPage", QVariant(!showPage));
        QBook::settings().sync();
        setWithPageBtn();

}

void SettingsReaderProgressBar::withChapterBtnHandler()
{
        qDebug() << Q_FUNC_INFO;

        bool showChapterInfo = QBook::settings().value("setting/showChapterInfo", QVariant(true)).toBool();
        QBook::settings().setValue("setting/showChapterInfo", QVariant(!showChapterInfo));
        QBook::settings().sync();
        setWithChapterBtn();

}

void SettingsReaderProgressBar::withProgressBarHandler()
{
        qDebug() << Q_FUNC_INFO;

        bool showProgressBar = QBook::settings().value("setting/showProgressBar", QVariant(false)).toBool();
        QBook::settings().setValue("setting/showProgressBar", QVariant(!showProgressBar));
        QBook::settings().sync();
        setWithProgressBarBtn();

}

void SettingsReaderProgressBar::withBookTitleHandler()
{
        qDebug() << Q_FUNC_INFO;

        bool showBookTitle = QBook::settings().value("setting/showBookTitle", QVariant(false)).toBool();
        QBook::settings().setValue("setting/showBookTitle", QVariant(!showBookTitle));
        QBook::settings().sync();
        setWithBookTitleBtn();

}

void SettingsReaderProgressBar::withDateTimHandler()
{
        qDebug() << Q_FUNC_INFO;

        bool showDateTime = QBook::settings().value("setting/showDateTime", QVariant(false)).toBool();
        QBook::settings().setValue("setting/showDateTime", QVariant(!showDateTime));
        QBook::settings().sync();
        setWithDateTimeBtn();

}

void SettingsReaderProgressBar::withChapterLinesHandler()
{
        qDebug() << Q_FUNC_INFO;

        bool showChapterLines = QBook::settings().value("setting/showChapterLines", QVariant(true)).toBool();
        QBook::settings().setValue("setting/showChapterLines", QVariant(!showChapterLines));
        QBook::settings().sync();
        setWithChapterLinesBtn();
}

void SettingsReaderProgressBar::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void SettingsReaderProgressBar::showChapterLines()
{
    //Show all labels child of readinProgressFigureImage.
    qDebug() << Q_FUNC_INFO;
    QList<QLabel*> list = readingProgressFigureImage->findChildren<QLabel *>();
    for(int j = 0; j < list.size();j++)
    {
        QLabel* chapterLine = list[j];
        chapterLine->setFixedSize(m_chapterLineWidth,readingProgress->height());
        int x = readingProgress->rect().left() + readingProgress->rect().width() * j / list.size();
        chapterLine->move(x, readingProgress->rect().top());
        chapterLine->show();
    }
}

void SettingsReaderProgressBar::hideChapterLines()
{
    //Hide all labels child of readinProgressFigureImage.
    qDebug() << Q_FUNC_INFO;
    QList<QLabel*> list = readingProgressFigureImage->findChildren<QLabel *>();
    for(int j = 0; j < list.size();j++)
    {
        QLabel* chapterLine = list[j];
        chapterLine->hide();
    }
}
