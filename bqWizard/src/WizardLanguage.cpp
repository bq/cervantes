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

#include "WizardLanguage.h"
#include "FullScreenWidget.h"
#include "PowerManager.h"
#include "Screen.h"
#include "QBookApp.h"
#include "QBook.h"
#include "DeviceInfoDialog.h"
#include <QTimer>
#include <QWidget>
#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include "unistd.h"

WizardLanguage::WizardLanguage(QWidget* parent): FullScreenWidget(parent), settingsLanguage(0), choosenLanguage(0)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);
    settingsLanguage = getLangugeFromSettings();
    choosenLanguage = settingsLanguage;
    connect(engBtn, SIGNAL(clicked()), this,SLOT(eng_BtnClicked()));
    connect(spaBtn, SIGNAL(clicked()), this,SLOT(spa_BtnClicked()));
    connect(porBtn, SIGNAL(clicked()), this,SLOT(por_BtnClicked()));
    connect(catBtn, SIGNAL(clicked()), this,SLOT(cat_BtnClicked()));
    connect(eusBtn, SIGNAL(clicked()), this,SLOT(bas_BtnClicked()));
    connect(galBtn, SIGNAL(clicked()), this,SLOT(gal_BtnClicked()));

    connect(deviceInfo, SIGNAL(clicked()), this, SLOT(handleDeviceInfo()));
}

WizardLanguage::~WizardLanguage()
{
    qDebug() << Q_FUNC_INFO;
}

void WizardLanguage::eng_BtnClicked()
{
    qDebug() << Q_FUNC_INFO;
    choosenLanguage = ENGLISH;
    languageAccepted();
}

void WizardLanguage::spa_BtnClicked()
{
    qDebug() << Q_FUNC_INFO;
    choosenLanguage = SPANISH;
    languageAccepted();
}

void WizardLanguage::por_BtnClicked()
{
    qDebug() << Q_FUNC_INFO;
    choosenLanguage = PORTUGUES;

    languageAccepted();
}

void WizardLanguage::cat_BtnClicked()
{
    qDebug() << Q_FUNC_INFO;
    choosenLanguage = CATALAN;

    languageAccepted();
}

void WizardLanguage::bas_BtnClicked()
{
    qDebug() << Q_FUNC_INFO;
    choosenLanguage = BASQUE;

    languageAccepted();
}

void WizardLanguage::gal_BtnClicked()
{
    qDebug() << Q_FUNC_INFO;
    choosenLanguage = GALICIAN;

    languageAccepted();
}

int WizardLanguage::getLangugeFromSettings()
{
    qDebug() << Q_FUNC_INFO;

    QString language = QBook::settings().value("setting/language",QVariant("es")).toString();
    int value;
    qDebug() << Q_FUNC_INFO << "language" << language;

    if(language == "es")
        value = 1;
    else if(language == "pt")
        value = 2;
    else if(language == "en")
        value = 0;

    return value;
}

void WizardLanguage::languageAccepted()
{
    qDebug() << Q_FUNC_INFO << "settingsLanguage: " << settingsLanguage << "choosenLanguage: " << choosenLanguage;

    if(choosenLanguage == SPANISH)
        QBook::settings().setValue("setting/language", "es");
    else if (choosenLanguage == ENGLISH)
        QBook::settings().setValue("setting/language", "en");
    else if (choosenLanguage == PORTUGUES)
        QBook::settings().setValue("setting/language", "pt");
    else if (choosenLanguage == CATALAN)
        QBook::settings().setValue("setting/language", "ca");
    else if (choosenLanguage == BASQUE)
        QBook::settings().setValue("setting/language", "eu");
    else if (choosenLanguage == GALICIAN)
        QBook::settings().setValue("setting/language", "gl");

    QBook::settings().setValue("setting/initial_lang_selection", false);

    if(settingsLanguage != choosenLanguage){

        PowerManagerLock* lock = PowerManager::getNewLock(this);
        lock->activate();
        QBookApp::instance()->disableUserEvents();
        QBookApp::instance()->showRestoringImage();
        Screen::getInstance()->lockScreen();

        QBook::settings().sync();
        ::sync();
        QTimer::singleShot(100, this, SLOT(quitApp()));
    }else
        emit languageSelected();
}

void WizardLanguage::quitApp()
{
    QApplication::quit();
}

void WizardLanguage::handleDeviceInfo()
{
        qDebug() << Q_FUNC_INFO;
        QBookApp::instance()->showDeviceInfoDialog();
}

void WizardLanguage::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }
