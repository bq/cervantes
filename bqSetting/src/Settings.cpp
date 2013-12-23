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
#include <QFile>
#include <QPainter>
#include <QStackedWidget>
#include "Settings.h"
#include "StatusBar.h"

#include "SettingsMyAccountMenu.h"
#ifndef HACKERS_EDITION
#include "SettingsDictionariesMenu.h"
#endif
#include "SettingsDeviceOptionsMenu.h"
#include "SettingsBrightness.h"
#include "SettingsDeviceInfo.h"
#include "SettingsDevelopmentOptions.h"
#include "QBookApp.h"
#include "Screen.h"
#include "QBook.h"
#include "DeviceInfo.h"
#include "SettingsAboutUs.h"

#include <QKeyEvent>

Settings::Settings(QWidget* parent) : QBookForm(parent)
        ,m_currentForm(0)
        ,widgetStack(0)
        ,m_myAccount(NULL)
        ,m_deviceOptions(NULL)
        ,m_light(NULL)
        ,m_deviceInfo(NULL)
        ,m_aboutUs(NULL)
        ,isChildSetting(0)
{

        qDebug() << Q_FUNC_INFO;

        setupUi(this);
        widgetStack = new QStackedWidget(this);

        /* Please, keep the same order than interface */

#ifndef HACKERS_EDITION
        connect(userDataBtn,SIGNAL(clicked()), this, SLOT(handleMyAccountBtn()));
#else
	userDataBtn->setText("Development options");
        connect(userDataBtn,SIGNAL(clicked()), this, SLOT(handleDevelopmentBtn()));
#endif
        connect(deviceInfoBtn,      SIGNAL(clicked()), this, SLOT(handleInformationBtn()));
        connect(deviceOptionsBtn,   SIGNAL(clicked()), this, SLOT(handleDevicesBtn()));
        connect(wifiConnexionBtn,   SIGNAL(clicked()), this, SLOT(handleWifiBtn()));
        connect(lightSettingsBtn,   SIGNAL(clicked()), this, SLOT(handleLightBtn()));
        connect(webBrowserBtn,      SIGNAL(clicked()), this, SLOT(handleBrowserBtn()));
        connect(activateDeviceBtn,  SIGNAL(clicked()), this, SLOT(activateDevice()));
        connect(aboutUsBtn,         SIGNAL(clicked()), this, SLOT(handleAboutUsBtn()));
        /**********************************************/

        QFile fileSpecific(":/res/settings_styles.qss");
        QFile fileCommons(":/res/settings_styles_generic.qss");
        fileSpecific.open(QFile::ReadOnly);
        fileCommons.open(QFile::ReadOnly);

        QString styles = QLatin1String(fileSpecific.readAll() + fileCommons.readAll());
        setStyleSheet(styles);

        if(!DeviceInfo::getInstance()->hasFrontLight()){
            lightSettingsBtn->hide();
        }
}

Settings::~Settings()
{
        qDebug() << Q_FUNC_INFO;
}

void Settings::activateForm()
{    
        qDebug() << Q_FUNC_INFO ;

        QBookApp::instance()->getStatusBar()->setMenuTitle(tr("Settings"));
}

void Settings::deactivateForm()
{
    qDebug() << Q_FUNC_INFO;
    if(m_deviceOptions && m_deviceOptions->fromViewer())
        m_deviceOptions->resetFromViewer();
}

void Settings::handleDevicesBtn()
{
        qDebug() << Q_FUNC_INFO;

        if(!m_deviceOptions){
            m_deviceOptions = new SettingsDeviceOptionsMenu(this);
            connect(m_deviceOptions,SIGNAL(hideMe()), this, SLOT(hideElement()));
        }

        m_deviceOptions->setup();
        showElement(m_deviceOptions);
}

SettingsDeviceOptionsMenu* Settings::getDeviceOptions()
{
        qDebug() << Q_FUNC_INFO;

        if(!m_deviceOptions){
            m_deviceOptions = new SettingsDeviceOptionsMenu(this);
            connect(m_deviceOptions,SIGNAL(hideMe()), this, SLOT(hideElement()));
        }

        showElement(m_deviceOptions);
        return m_deviceOptions;
}

void Settings::handleWifiBtn()
{
        qDebug() << Q_FUNC_INFO;

        QBookApp::instance()->showWifiSelection(SettingsWiFiList::BACK_BEHAVIOR);
}

void Settings::handleMyAccountBtn()
{
        qDebug() << Q_FUNC_INFO;

        if(!m_myAccount){
            m_myAccount = new SettingsMyAccountMenu(this);
            connect(m_myAccount, SIGNAL(hideMe()), this, SLOT(hideElement()));
        }

        m_myAccount->setup();
        showElement(m_myAccount);
}

void Settings::handleDevelopmentBtn()
{
        qDebug() << Q_FUNC_INFO;

        SettingsDevelopmentOptions* devel = new SettingsDevelopmentOptions(this);
        connect(devel, SIGNAL(hideMe()), this, SLOT(hideElement()));
        showElement(devel);
}

void Settings::handleLightBtn()
{
        qDebug() << Q_FUNC_INFO;

        if(!m_light){
            m_light = new SettingsBrightness(this);
            connect(m_light, SIGNAL(hideMe()), this, SLOT(hideElement()));
        }

        showElement(m_light);
}

void Settings::handleBrowserBtn()
{
        qDebug() << Q_FUNC_INFO;
        if(ConnectionManager::getInstance()->isConnected())
            goToBrowser();
        else {
#ifndef FAKE_WIFI
                QBookApp::instance()->requestConnection(true);
                connectWifiObserverBrowser();
                connect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disconnectWifiObserverBrowser()));
#endif
        }
}

void Settings::connectWifiObserverBrowser()
{
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(goToBrowser()),Qt::UniqueConnection);
}

void Settings::disconnectWifiObserverBrowser()
{
    disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(goToBrowser()));
}

void Settings::connectWifiObserverWizard()
{
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(activateDevice()),Qt::UniqueConnection);
}

void Settings::disconnectWifiObserverWizard()
{
    disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(activateDevice()));
}

void Settings::goToBrowser() {
    disconnectWifiObserverBrowser();
    QBookApp::instance()->goToBrowser();
}

void Settings::handleInformationBtn()
{
        qDebug() << Q_FUNC_INFO;

        if(!m_deviceInfo){
            m_deviceInfo = new SettingsDeviceInfo(this);
            connect(m_deviceInfo, SIGNAL(hideMe()), this, SLOT(hideElement()));
        }

        showElement(m_deviceInfo);
}

SettingsDeviceInfo* Settings::showInformation()
{
        qDebug() << Q_FUNC_INFO;

        if(!m_deviceInfo){
            m_deviceInfo = new SettingsDeviceInfo(this);
            connect(m_deviceInfo, SIGNAL(hideMe()), this, SLOT(hideElement()));
        }

        showElement(m_deviceInfo);
        return m_deviceInfo;
}

void Settings::showElement(QWidget * widget)
{
        qDebug() << Q_FUNC_INFO;

        Screen::getInstance()->queueUpdates();

        ++isChildSetting; //TODO: Take a look, i hate this kind of variables (Mikel)
        if(widget->inherits("ViewerMenu"))
        {
            // Show
            widget->show();
            //getStatusBar()->show();
            // Order
            widget->raise();
        }
        else if(widget->inherits("FullScreenWidget")) // Full Screen
        {
            // disconnects previous form
            FullScreenWidget* newForm = (FullScreenWidget*) widget;
            newForm->setNavigationPrevious(m_currentForm);// NOTE: Don't care if it's NULL. It'll be evaluated later (hideElement)
            m_currentForm = newForm;

            // Show widgets
            widget->show();
            widget->raise();
        }
        else if(widget->inherits("PopUp")){
            // Show widgets
            widget->show();
            widget->raise();
        }
        else if(widget->inherits("ViewerTextActionsPopup")){
            // Store current popUp
            widget->show();

        } else { // Unknown widget type
            qDebug() << Q_FUNC_INFO << "Not recognised element";
        }
        widget->setFocus();

        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
}


void Settings::hideElement()
{
        qDebug() << Q_FUNC_INFO;

        Screen::getInstance()->queueUpdates();

        --isChildSetting;
        if( m_currentForm )
        {
            m_currentForm->hide();

            FullScreenWidget* prevForm = m_currentForm->getNavigationPrevious();
            if( prevForm )
            {
                qDebug() << Q_FUNC_INFO << ": " << prevForm;
                prevForm->setFocus();
            }
            else
            {
                setFocus();
            }
            qDebug() << Q_FUNC_INFO << ": " << m_currentForm;
            m_currentForm = prevForm;
        }

        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
}

void Settings::keyReleaseEvent(QKeyEvent *event)
{
        qDebug() << Q_FUNC_INFO;

        if( event->key()==QBook::QKEY_BACK )
        {
            if( isChildSetting > 0 )
            {
                qDebug() << "--->CURRENT NUMBER OF CHILDS IS: " << isChildSetting;
                hideElement();
                event->accept();
            }
            else
            {
                QBookApp::instance()->goToHome();
                qDebug() << Q_FUNC_INFO;
            }
            return;
        } else
            QWidget::keyReleaseEvent(event);

}

void Settings::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }


void Settings::showEvent (QShowEvent * )
{
    qDebug() << Q_FUNC_INFO;
#ifndef HACKERS_EDITION
    if(!QBookApp::instance()->isActivated())

        activateDeviceBtn->show();
    else
#endif
        activateDeviceBtn->hide();
}

void Settings::activateDevice()
{
#ifndef HACKERS_EDITION
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    if(ConnectionManager::getInstance()->isConnected())
        QBookApp::instance()->goToWizard();
    else
    {
        connectWifiObserverWizard();
        connect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disconnectWifiObserverWizard()));
        QBookApp::instance()->requestConnection(true);
    }
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
#endif
}

void Settings::handleAboutUsBtn()
{
        qDebug() << Q_FUNC_INFO;

        if(!m_aboutUs){
            m_aboutUs = new SettingsAboutUs(this);
            connect(m_aboutUs, SIGNAL(hideMe()), this, SLOT(hideElement()));
        }
        showElement(m_aboutUs);
}


void Settings::goToViewerMenu()
{
    while ( isChildSetting > 0 )
    {
        hideElement();
    }
    handleDevicesBtn();
    m_deviceOptions->showReaderMenu();
    connect(m_deviceOptions, SIGNAL(goToViewer()), this, SLOT(handleGoToViewer()));
}

void Settings::handleGoToViewer()
{
    qDebug() << Q_FUNC_INFO;
    disconnect(m_deviceOptions, SIGNAL(goToViewer()), this, SLOT(handleGoToViewer()));
    emit goToViewer();
}
