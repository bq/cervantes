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

#include "SettingsDeviceOptionsMenu.h"

#include <QDebug>
#include <QKeyEvent>
#include <QPainter>

#ifndef HACKERS_EDITION
#include "SettingsDictionariesMenu.h"
#endif
#include "Settings.h"
#include "SettingsLanguageMenu.h"
#include "SettingsDateTimeMenu.h"
#include "SettingsEnergySaving.h"
#include "SettingsReaderMenu.h"
#include "SettingsPageScreenSaver.h"
#include "QBook.h"
#include "QBookApp.h"
#include "ConnectionManager.h"
#include "InfoDialog.h"

SettingsDeviceOptionsMenu::SettingsDeviceOptionsMenu(QWidget *parent) : FullScreenWidget(parent)
{
        setupUi(this);

        connect(backBtn, SIGNAL(clicked()), this, SIGNAL(hideMe()));
        connect(languageBtn, SIGNAL(clicked()), this, SLOT(handleLanguage()));
        connect(dateTimeBtn, SIGNAL(clicked()), this, SLOT(handleTimeZone()));
        connect(energySavingBtn,SIGNAL(clicked()), this, SLOT(handleEnergySaving()));
        connect(readerBtn, SIGNAL(clicked()), this, SLOT(handleReadMode()));
        connect(dictionariesBtn, SIGNAL(clicked()), this, SLOT(showDictionaryMenu()));
        connect(screenSaverBtn, SIGNAL(clicked()), this, SLOT(handleScreenSaver()));

        language = new SettingsLanguageMenu(this);
        timeZone = new SettingsDateTimeMenu(this);
        energySaving = new SettingsEnergySaving(this);
        readMode = new SettingsReaderMenu(this);
#ifndef HACKERS_EDITION
        dictionary = new SettingsDictionariesMenu(this);
#endif
        screenSaver = new SettingsPageScreenSaver(this);
        screenSaver->hide();

        connect(language,       SIGNAL(hideMe()), this, SLOT(hideTopElement()));
        connect(timeZone,       SIGNAL(hideMe()), this, SLOT(hideTopElement()));
        connect(energySaving,   SIGNAL(hideMe()), this, SLOT(hideTopElement()));
        connect(readMode,       SIGNAL(hideMe()), this, SLOT(hideTopElement()));
#ifndef HACKERS_EDITION
        connect(dictionary,     SIGNAL(hideMe()), this, SLOT(hideTopElement()));
#endif
        connect(screenSaver,    SIGNAL(hideMe()), this, SLOT(hideTopElement()));
}

SettingsDeviceOptionsMenu::~SettingsDeviceOptionsMenu()
{
}

void SettingsDeviceOptionsMenu::setup()
{
    language->hide();
    timeZone->hide();
    energySaving->hide();
    readMode->hide();
#ifndef HACKERS_EDITION
    dictionary->hide();
#endif
}

void SettingsDeviceOptionsMenu::handleLanguage(){
        qDebug() << Q_FUNC_INFO << "Calling LanguageMenu";
        ((Settings*)parent())->showElement(language);
}

void SettingsDeviceOptionsMenu::handleTimeZone(){

        qDebug() << Q_FUNC_INFO << "Calling TimeZone";
        ((Settings*)parent())->showElement(timeZone);
}

void SettingsDeviceOptionsMenu::handleEnergySaving()
{
    qDebug() << Q_FUNC_INFO << ": Calling EnergySaving";
    ((Settings*)parent())->showElement(energySaving);
}

void SettingsDeviceOptionsMenu::handleReadMode(){
        qDebug() << Q_FUNC_INFO << ": Calling readMode";
        ((Settings*)parent())->showElement (readMode);
}

#ifndef HACKERS_EDITION
void SettingsDeviceOptionsMenu::showDictionaryMenu()
{
    qDebug() << Q_FUNC_INFO;
    if(QBookApp::instance()->isActivated() || QBookApp::instance()->isLinked())
        ((Settings*)parent())->showElement(dictionary);
    else{
        Screen::getInstance()->queueUpdates();
        InfoDialog *infoDialog = new InfoDialog(this, tr("Recuerda activar tu dispositivo para disfrutar de los diccionarios."), 3000);
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
        infoDialog->hideSpinner();
        infoDialog->showForSpecifiedTime();
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
        delete infoDialog;
    }
}
#endif

void SettingsDeviceOptionsMenu::handleScreenSaver()
{
    qDebug() << Q_FUNC_INFO;
    ((Settings*)parent())->showElement(screenSaver);
}

void SettingsDeviceOptionsMenu::keyReleaseEvent(QKeyEvent *event) {
        qDebug() << Q_FUNC_INFO;
        if( event->key() == QBook::QKEY_BACK )
        {
            qDebug() << ": Closing...";
            emit hideMe();
            event->accept();
            return;
        }
        QWidget::keyReleaseEvent(event);
}

void SettingsDeviceOptionsMenu::hideTopElement(){
        qDebug() << Q_FUNC_INFO;
        ((Settings *)parent())->hideElement();
}

void SettingsDeviceOptionsMenu::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

