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

#include <QPainter>
#include <QList>
#include <QDebug>
#include "HomeBooksListOptions.h"
#include "Home.h"
#include "QBook.h"

QString SELECTED("color:#000000;background:url(':/res/check_ico.png') no-repeat center right;");
QString ENABLED("color:#000000;");
QString DISABLED("color:#606060;");


HomeBooksListOptions::HomeBooksListOptions(QWidget *parent) : QWidget(parent){
        setupUi(this);

        QStringList categories = QBook::settings().value("setting/home/categories").toString().split(";");

        if(categories.size() > 0 && !categories[0].trimmed().isEmpty())
            category1Btn->setText(categories[0]);
        else
            category1Btn->setText(tr("Recomendados para ti"));

        if(categories.size() > 1 &&  !categories[1].trimmed().isEmpty())
            category2Btn->setText(categories[1]);
        else
            category2Btn->setText(tr("Novedades"));

        if(categories.size() > 2 && !categories[2].trimmed().isEmpty())
            category3Btn->setText(categories[2]);
        else
            category3Btn->setText(tr("Top Ventas"));

        category1Btn->setEnabled(false);
        category2Btn->setEnabled(false);
        category3Btn->setEnabled(false);
        category1Btn->setStyleSheet(DISABLED);
        category2Btn->setStyleSheet(DISABLED);
        category3Btn->setStyleSheet(DISABLED);

        connect(category1Btn, SIGNAL(clicked()), SLOT(selectedCategory1()));
        connect(category2Btn, SIGNAL(clicked()), SLOT(selectedCategory2()));
        connect(category3Btn, SIGNAL(clicked()), SLOT(selectedCategory3()));
        connect(recentBtn,    SIGNAL(clicked()), SLOT(selectedRecents()));
}

HomeBooksListOptions::~HomeBooksListOptions(){
    // do nothing
}

void HomeBooksListOptions::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }


void HomeBooksListOptions::setupOptions(bool recent, bool hasCategory1, const QString& category1Name, bool hasCategory2, const QString& category2Name, bool hasCategory3, const QString& category3Name, Home::VIEW view){
        qDebug() << Q_FUNC_INFO;
        recentBtn->setEnabled(recent);
        category1Btn->setEnabled(hasCategory1);
        category2Btn->setEnabled(hasCategory2);
        category3Btn->setEnabled(hasCategory3);

        if(recent)
            if(view == Home::RECENT)
                recentBtn->setStyleSheet(SELECTED);
            else
                recentBtn->setStyleSheet(ENABLED);
        else
            recentBtn->setStyleSheet(DISABLED);

        if(hasCategory1)
        {
            category1Btn->show();
            category1Btn->setText(category1Name);
            if(view == Home::CATEGORY1)
                category1Btn->setStyleSheet(SELECTED);
            else
                category1Btn->setStyleSheet(ENABLED);
        }else if(!category1Name.isEmpty())
            category1Btn->setStyleSheet(DISABLED);


        if(hasCategory2)
        {
            category1Btn->show();
            category2Btn->setText(category2Name);
            if(view == Home::CATEGORY2)
                category2Btn->setStyleSheet(SELECTED);
            else
                category2Btn->setStyleSheet(ENABLED);
        }else if(!category2Name.isEmpty())
            category2Btn->setStyleSheet(DISABLED);

        if(hasCategory3)
        {
            category3Btn->show();
            category3Btn->setText(category3Name);
            if(view == Home::CATEGORY3)
                category3Btn->setStyleSheet(SELECTED);
            else
                category3Btn->setStyleSheet(ENABLED);
        }else if(!category3Name.isEmpty())
            category3Btn->setStyleSheet(DISABLED);

        // NOTE: These lines allow us to ensure the window has the proper width before showing
        show();
        layout()->invalidate();
        hide();
}

void HomeBooksListOptions::selectedCategory1 (){
        qDebug() << Q_FUNC_INFO;
        emit viewSelected(Home::CATEGORY1);
}

void HomeBooksListOptions::selectedCategory2(){
        qDebug() << Q_FUNC_INFO;
        emit viewSelected(Home::CATEGORY2);
}

void HomeBooksListOptions::selectedCategory3(){
        qDebug() << Q_FUNC_INFO;
        emit viewSelected(Home::CATEGORY3);
}

void HomeBooksListOptions::selectedRecents(){
        qDebug() << Q_FUNC_INFO;
        emit viewSelected(Home::RECENT);
}

void HomeBooksListOptions::keyReleaseEvent(QKeyEvent* event)
{
        qDebug() << Q_FUNC_INFO;
        emit hideMe();
        QWidget::keyReleaseEvent(event);
}
void HomeBooksListOptions::showEvent ( QShowEvent * )
{
        qDebug() << Q_FUNC_INFO;
        setFocus(Qt::ActiveWindowFocusReason);
}

