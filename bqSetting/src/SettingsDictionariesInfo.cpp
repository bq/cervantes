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

#include "SettingsDictionariesInfo.h"
#include "FullScreenWidget.h"
#include "Dictionary.h"
#include "Screen.h"

#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>

#define ITEMS_PER_PAGE 4
#define PERCENT_STEP_VALUE 0.75

SettingsDictionariesInfo::SettingsDictionariesInfo(QWidget* parent) : FullScreenWidget(parent),
    page(0), totalPages(0)
{
        qDebug() << "--->" << Q_FUNC_INFO;
        setupUi(this);

        dictionaryList = Dictionary::instance()->parseCurrentDictionaries(QString::fromUtf8(Dictionary::instance()->getJsonDictionaries())).values();
        vbar = listView->verticalScrollBar();
        connect(backBtn,SIGNAL(clicked()),this, SIGNAL(hideMe()));

        // Connect page handler
        connect(verticalPage, SIGNAL(previousPageReq()),this,   SLOT(previousPage()));
        connect(verticalPage, SIGNAL(nextPageReq()),    this,   SLOT(nextPage()));
        verticalPage->hidePages();
}

SettingsDictionariesInfo::~SettingsDictionariesInfo()
{
        qDebug() << Q_FUNC_INFO;
}

void SettingsDictionariesInfo::fillInfo()
{
        qDebug() << Q_FUNC_INFO;
        int itemsInfo = dictionaryList.size() - 1; // -1 because simple english words
        QString copy(QChar::fromAscii('©'));
        QString eTilde(QChar(232));

        QString catalonianDictionary = QString("\n\nDiccionari Manual de la Llengua Catalana\n")
                                     + QString("Enciclop%1dia Catalana\n").arg(eTilde)
                                     + QString("Powered by Dixio - %1 2012 Semantix\n").arg(copy) ;

        QList<DictionaryParams>::iterator it = dictionaryList.begin();
        QList<DictionaryParams>::iterator itEnd = dictionaryList.end();

        bool hasCatalonian = false;
        while(it != itEnd)
        {
            if((*it).id == "224")
            {
                hasCatalonian = true;
                break;
            }

            it++;
        }
        if(hasCatalonian)
            listView->setText(tr("Dictionaries_copyright_info") + catalonianDictionary);
        else
        {
            itemsInfo--;
            listView->setText(tr("Dictionaries_copyright_info"));
        }
        //Define a single step as the 75% of the normal height in the widget.
        vbar->setSingleStep(listView->height()*PERCENT_STEP_VALUE);

        if(vbar->maximum() % vbar->singleStep() == 0)
            totalPages = vbar->maximum() / vbar->singleStep() + 1;
        else
            totalPages = vbar->maximum() / vbar->singleStep() + 2;

        verticalPage->setup(totalPages);
}

void SettingsDictionariesInfo::keyReleaseEvent(QKeyEvent *event)
{
    qDebug() << "--->" << Q_FUNC_INFO;
    if( event->key() == QBook::QKEY_BACK )
    {
        qDebug() << ": Closing...";
        emit hideMe();
        event->accept();
        return;
    }
    QWidget::keyReleaseEvent(event);
}

void SettingsDictionariesInfo::paintEvent (QPaintEvent *)
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void SettingsDictionariesInfo::previousPage()
{
    qDebug() << "--->" << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();

    if (vbar)
        vbar->triggerAction(QAbstractSlider::SliderSingleStepSub);

    Screen::getInstance()->setMode(Screen::MODE_QUICK, true, FLAG_WAITFORCOMPLETION, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void SettingsDictionariesInfo::nextPage()
{
    qDebug() << "--->" << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();

    if (vbar)
        vbar->triggerAction(QAbstractSlider::SliderSingleStepAdd);

    Screen::getInstance()->setMode(Screen::MODE_QUICK, true, FLAG_WAITFORCOMPLETION, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

