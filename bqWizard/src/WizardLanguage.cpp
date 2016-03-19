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
#include "QButtonGroup"
#include "unistd.h"

WizardLanguage::WizardLanguage(QWidget* parent): FullScreenWidget(parent), settingsLanguage(0), choosenLanguage(0)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);
    settingsLanguage = getLanguageFromSettings();
    choosenLanguage = settingsLanguage;

    m_languageButtons.addButton(engBtn, ENGLISH);
    m_languageButtons.addButton(spaBtn, SPANISH);
    m_languageButtons.addButton(porBtn, PORTUGUES);
    m_languageButtons.addButton(catBtn, CATALAN);
    m_languageButtons.addButton(eusBtn, BASQUE);
    m_languageButtons.addButton(galBtn, GALICIAN);
    m_languageButtons.addButton(freBtn, FRENCH);
    m_languageButtons.addButton(gerBtn, GERMAN);
    m_languageButtons.addButton(itaBtn, ITALIAN);
    connect(&m_languageButtons, SIGNAL(buttonPressed(int)), this, SLOT(btnClicked(int)));

    connect(deviceInfo, SIGNAL(clicked()), this, SLOT(handleDeviceInfo()));
}

WizardLanguage::~WizardLanguage()
{
    qDebug() << Q_FUNC_INFO;
}

void WizardLanguage::btnClicked(int langIndex)
{
    qDebug() << Q_FUNC_INFO << langIndex;
    choosenLanguage = langIndex;
    languageAccepted();
}

int WizardLanguage::getLanguageFromSettings()
{
    qDebug() << Q_FUNC_INFO;

    QString language = QBook::settings().value("setting/language",QVariant("es")).toString();
    int value;
    qDebug() << Q_FUNC_INFO << "language" << language;

    if(language == "es")
        value = SPANISH;
    else if(language == "pt")
        value = PORTUGUES;
    else if(language == "en")
        value = ENGLISH;
    else if(language == "eu")
        value = BASQUE;
    else if(language == "gl")
        value = GALICIAN;
    else if(language == "ca")
        value = CATALAN;
    else if(language == "fr")
        value = FRENCH;
    else if(language == "de")
        value = GERMAN;
    else if(language == "it")
        value = ITALIAN;

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
    else if (choosenLanguage == FRENCH)
        QBook::settings().setValue("setting/language", "fr");
    else if (choosenLanguage == GERMAN)
        QBook::settings().setValue("setting/language", "de");
    else if (choosenLanguage == ITALIAN)
        QBook::settings().setValue("setting/language", "it");

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
