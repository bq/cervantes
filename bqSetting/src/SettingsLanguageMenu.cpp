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

#include "SettingsLanguageMenu.h"
#include "QBook.h"
#include "SelectionDialog.h"
#include "InfoDialog.h"
#include "QBookApp.h"
#include "Screen.h"
#include "PowerManager.h"

#include <QDebug>
#include <QTimer>
#include <QKeyEvent>
#include <QPainter>

SettingsLanguageMenu::SettingsLanguageMenu(QWidget *parent) : FullScreenWidget(parent)
  , infoDialog(NULL)
{
        setupUi(this);
        m_buttonGroup = new QButtonGroup(this);
        m_buttonGroup->addButton(englishBtn, LANG_EN);
        m_buttonGroup->addButton(espanolBtn,LANG_ES);
        m_buttonGroup->addButton(portuguesBtn,LANG_PT);
        m_buttonGroup->addButton(catalanBtn,LANG_CA);
        m_buttonGroup->addButton(euskeraBtn, LANG_EU);
        m_buttonGroup->addButton(galicianBtn, LANG_GL);

        connect(backBtn,SIGNAL(clicked()), this, SIGNAL(hideMe()));
        connect(m_buttonGroup, SIGNAL(buttonClicked(int)),this, SLOT(changeLanguage(int)));

        setCurrentLanguage();
}

SettingsLanguageMenu::~SettingsLanguageMenu()
{
    delete m_buttonGroup;
    m_buttonGroup = NULL;
    delete infoDialog;
    infoDialog = NULL;
}

void SettingsLanguageMenu::changeLanguage(int langInt)
{
    qDebug() << Q_FUNC_INFO << langInt;

    int currentLang = getIntByLangCode(QBook::settings().value("setting/language", "es").toString());

    if(langInt == currentLang)
        return;

    SelectionDialog* confirmDialog = new SelectionDialog(this,tr("Do you want to change language?"));
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
    confirmDialog->exec();
    bool result = confirmDialog->result();
    delete confirmDialog;

    if(result) {

        PowerManagerLock* lock = PowerManager::getNewLock(this);
        lock->activate();
        QBookApp::instance()->cancelSync();
        QBookApp::instance()->disableUserEvents();
        QString lang(getLangByInt(langInt));
        qDebug() << Q_FUNC_INFO << "Changing lang to " << lang;
        QBook::settings().setValue("setting/language", lang);
        QBookApp::instance()->showRestoringImage();
        Screen::getInstance()->lockScreen();

        if(langInt == LANG_EN)
            QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedKingdom));
        else if (langInt == LANG_ES )
            QLocale::setDefault(QLocale(QLocale::Spanish, QLocale::Spain));
        else if (langInt == LANG_PT)
            QLocale::setDefault(QLocale(QLocale::Portuguese, QLocale::Portugal));
        else if (langInt == LANG_CA)
            QLocale::setDefault(QLocale(QLocale::Catalan, QLocale::Spain));
        else if (langInt == LANG_EU)
            QLocale::setDefault(QLocale(QLocale::Basque, QLocale::Spain));
        else if (langInt == LANG_GL)
            QLocale::setDefault(QLocale(QLocale::Galician, QLocale::Spain));


        QBook::settings().setValue("setting/changeLanguage", true);
        QBookApp::instance()->syncModel();
        QBook::settings().sync();

        QTimer::singleShot(100, this, SLOT(quitApp()));
    }
    else
        setCurrentLanguage();
}

void SettingsLanguageMenu::quitApp()
{    
        QApplication::quit();
}

void SettingsLanguageMenu::setCurrentLanguage(){
        qDebug() << "--->" << Q_FUNC_INFO;
        QString langCodeSelected = QBook::settings().value("setting/language", QVariant("es")).toString();
        int langId = getIntByLangCode(langCodeSelected);
        qDebug() << m_buttonGroup->buttons().size();
        for(int i=0; i<m_buttonGroup->buttons().size(); i++){
            if(i == langId){
                m_buttonGroup->button(i)->setStyleSheet("background-image: url(':/res/check_ico.png');");
                m_buttonGroup->button(i)->setChecked(true);
            }else
                m_buttonGroup->button(i)->setStyleSheet("background-image: none");
        }
}

const char* SettingsLanguageMenu::getLangByInt(int langInt) const
{
    switch(langInt)
    {
        case LANG_EN:
            return "en";
        case LANG_ES:
            return "es";
        case LANG_PT:
            return "pt";
        case LANG_CA:
            return "ca";
        case LANG_EU:
            return "eu";
        case LANG_GL:
            return "gl";
        default:
            return "es";
    }
}

int SettingsLanguageMenu::getIntByLangCode(const QString& code) const
{
    for(int enumVal = LANG_EN; enumVal <= LANG_NUMBER; ++enumVal) {
        if( code == getLangByInt(enumVal) )
        {
            return enumVal;
        }
    }
}

void SettingsLanguageMenu::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }
