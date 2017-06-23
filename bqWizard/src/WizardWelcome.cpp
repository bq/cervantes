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

#include "WizardWelcome.h"

#include "QBookApp.h"
#include "QBook.h"

#include "WebWizard.h"
#include "bqDeviceServices.h"
#include "ConfirmDialog.h"

#include "WizardLanguage.h"
#include "WizardDateTime.h"
#include "WizardGeneralTerms.h"
#include "WizardFinish.h"
#include "Screen.h"
#include "DeviceInfo.h"
#include "version.h"

#include <QWidget>
#include <QDebug>
#include <QKeyEvent>
#include <QEventLoop>

WizardWelcome::WizardWelcome(QWidget* parent)
    : QBookForm(parent)
    , current_widget(0)
    , m_WizardLanguage(0)
    , m_WizardDateTime(0)
    , m_elfFinishWizard(0)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);

    connect(nextStepBtn, SIGNAL(clicked()), this, SLOT(requestConnection()));
    connect(skipBtn, SIGNAL(clicked()), this, SLOT(skipWizard()));
    nextStepBtn->setFocus();

    if(QBook::settings().value("setting/initial_lang_selection",true).toBool())
    {
        if(!m_WizardLanguage)
            m_WizardLanguage = new WizardLanguage(this);


        connect(m_WizardLanguage, SIGNAL(languageSelected()), this, SLOT(hideLanguageSettings()));
        showElement(m_WizardLanguage);
    }
    if(QBookApp::instance()->isActivated())
    {
        intro3Lbl->hide();
        welcomeTitleLbl->setText(tr("Enlazar dispositivo"));
    }else
        intro3Lbl->show();

    // Check if: 1. language selection is needed, 2. initial power on for user is set and 3. Device has not been manually deactivated because of OTA
    if(!QBook::settings().value("setting/initial_lang_selection", true).toBool()&& QBook::settings().value("setting/initial",true).toBool()
            && !QBook::settings().value("setting/activated").isValid())
    {
        if(!m_WizardDateTime)
            m_WizardDateTime = new WizardDateTime(this);

        connect(m_WizardDateTime, SIGNAL(hideMe()), this, SLOT(hideWizardDateTime()));
        showElement(m_WizardDateTime);
    }
    QFile fileSpecific(":/res/wizard_styles.qss");
    QFile fileCommons(":/res/wizard_styles_generic.qss");
    fileSpecific.open(QFile::ReadOnly);
    fileCommons.open(QFile::ReadOnly);

    QString styles = QLatin1String(fileSpecific.readAll() + fileCommons.readAll());
    setStyleSheet(styles);
}

WizardWelcome::~WizardWelcome()
{
    qDebug() << Q_FUNC_INFO;
}

void WizardWelcome::hideLanguageSettings()
{
        qDebug() << Q_FUNC_INFO;
        Screen::getInstance()->queueUpdates();
        m_WizardLanguage->hide();
        if(!m_WizardDateTime)
            m_WizardDateTime = new WizardDateTime(this);

        connect(m_WizardDateTime, SIGNAL(hideMe()), this, SLOT(hideWizardDateTime()));
        showElement(m_WizardDateTime);
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
}

void WizardWelcome::hideWizardDateTime()
{
        qDebug() << Q_FUNC_INFO;
        Screen::getInstance()->queueUpdates();
        m_WizardDateTime->hide();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
}

void WizardWelcome::activateForm()
{
    qDebug() << Q_FUNC_INFO;
    QBookApp::instance()->getStatusBar()->hide();
}

void WizardWelcome::deactivateForm()
{}

void WizardWelcome::requestConnection()
{
        qDebug() << Q_FUNC_INFO;

        if(ConnectionManager::getInstance()->isConnected()) {
            loadWelcomePage();
        } else{
#ifndef FAKE_WIFI
            bool inWizard = QBook::settings().value("setting/initial",true).toBool();
            QBookApp::instance()->requestConnection(true, inWizard);
            connectWifiObserver();
            connect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disconnectWifiObserver()));
#endif
        }
}

void WizardWelcome::connectWifiObserver()
{
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(loadWelcomePage()),Qt::UniqueConnection);
}

void WizardWelcome::disconnectWifiObserver()
{
    disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(loadWelcomePage()));
}

void WizardWelcome::showElement(FullScreenWidget * widget)
{
    qDebug() << Q_FUNC_INFO;
    if(current_widget)
        current_widget->hide();

    current_widget = widget;
    widget->show();
    widget->raise();
//    widget->setFocus();
}

void WizardWelcome::hideTerms()
{
    qDebug() << Q_FUNC_INFO;
}

void WizardWelcome::goWizardFinished()
{
    qDebug() << Q_FUNC_INFO;

    disconnectWifiObserver();

    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->popForm(QBookApp::instance()->browserWizard());

    if(!m_elfFinishWizard)
    {
        m_elfFinishWizard = new WizardFinish(this);
        connect(m_elfFinishWizard, SIGNAL(closeWizard()), this, SLOT(goToCloseWizard()));
    }

    showElement(m_elfFinishWizard);
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void WizardWelcome::showRegisterFailed()
{
    qDebug() << Q_FUNC_INFO;

    ConfirmDialog* dialog = new ConfirmDialog(this,tr("Credit Card Register failed."));
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
    dialog->exec();
    delete dialog;

    QBookApp::instance()->popForm(QBookApp::instance()->browserWizard());

    if(!m_elfFinishWizard)
    {
        m_elfFinishWizard = new WizardFinish(this);
        connect(m_elfFinishWizard, SIGNAL(closeWizard()), this, SLOT(goToCloseWizard()));
    }
    showElement(m_elfFinishWizard);
}

void WizardWelcome::connectivityProblem()
{
    qDebug() << Q_FUNC_INFO;

    ConfirmDialog* dialog = new ConfirmDialog(this,tr("Connection problem. Please, select another WiFi."));
    QBookApp::instance()->getStatusBar()->setSpinner(false);
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_IGNORE_QUEUE,Q_FUNC_INFO);
    dialog->exec();
    delete dialog;

    ConnectionManager::getInstance()->removeAll();
    goToWifi();
}

void WizardWelcome::serialInvalidHandle()
{
    qDebug() << Q_FUNC_INFO;
    ConfirmDialog* dialog = new ConfirmDialog(this,tr("There is an error with the Serial Number ") + "(" + DeviceInfo::getInstance()->getSerialNumber() + ")" + tr(". Please contact with support."));
    QBookApp::instance()->getStatusBar()->setSpinner(false);
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_IGNORE_QUEUE,Q_FUNC_INFO);
    dialog->exec();
    delete dialog;
    QBookApp::instance()->goToHome();
}

void WizardWelcome::headersProblem()
{
    qDebug() << Q_FUNC_INFO;

    ConfirmDialog* dialog = new ConfirmDialog(this,tr("There is a problem in wizard. Please contact with support."));
    QBookApp::instance()->getStatusBar()->setSpinner(false);
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_IGNORE_QUEUE,Q_FUNC_INFO);
    dialog->exec();
    delete dialog;

    ConnectionManager::getInstance()->removeAll();
    goToWifi();
}

void WizardWelcome::goToWifi()
{
    qDebug() << Q_FUNC_INFO;    
    requestConnection();
}


void WizardWelcome::goToCloseWizard()
{
    qDebug() << Q_FUNC_INFO;
    emit closeWizard();
}

void WizardWelcome::loadWelcomePage()
{
    qDebug() << Q_FUNC_INFO;

    disconnectWifiObserver();

    WebWizard* webWizard = QBookApp::instance()->browserWizard();

    connect(webWizard, SIGNAL(registerOK()), this, SLOT(goWizardFinished()));
    connect(webWizard, SIGNAL(registerFailed()), this, SLOT(showRegisterFailed()));
    connect(webWizard, SIGNAL(goToWifi()), this, SLOT(goToWifi()));
    connect(webWizard, SIGNAL(connectionProblem()), this, SLOT(connectivityProblem()), Qt::UniqueConnection);
    connect(webWizard, SIGNAL(serialInvalid()), this, SLOT(serialInvalidHandle()), Qt::UniqueConnection);
    connect(webWizard, SIGNAL(processHeadersFailed()), this, SLOT(headersProblem()), Qt::UniqueConnection);

    QString landingUrl = QBook::settings().value("serviceURLs/landing",QVariant("http://landing.mundoreader.com/?")).toString();
    QString version = QBOOKAPP_VERSION;
    QString lang = QBook::settings().value("setting/language",QVariant("es")).toString();
    QString email = QBook::settings().value("eMail").toString();
    QString activated;
    if(QBook::settings().value("setting/activated", false).toBool())
        activated = "1";
    else
        activated = "0";
    qDebug() << Q_FUNC_INFO << "activated:"  << activated;

    QString url = QBookApp::instance()->getDeviceServices()->generateWelcomePageUrl(DeviceInfo::getInstance()->getSerialNumber(), landingUrl, version, lang, email, activated);

    qDebug() << "Loading url: " << url;
    webWizard->forceUrl(url);
    QBookApp::instance()->pushForm(webWizard);
}

void WizardWelcome::skipWizard()
{
    QBook::settings().setValue("setting/initial",false);
    QBookApp::instance()->goToHome();
}

void WizardWelcome::paintEvent (QPaintEvent *)
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
