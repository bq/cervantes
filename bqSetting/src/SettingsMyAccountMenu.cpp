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
#include "SettingsMyAccountMenu.h"
#include "SettingsUnLinkStoreDevice.h"
#include "Settings.h"
#include "QBookApp.h"
#ifndef DISABLE_ADOBE_SDK
#include "AdobeDRM.h"
#include "SettingsAdobeDrmLinkDevice.h"
#include "SettingsAdobeDrmUnLinkDevice.h"
#endif
#include "SettingsGeneralTermsInfo.h"

#ifndef HACKERS_EDITION
#include "bqDeviceServicesClient.h"
#endif

SettingsMyAccountMenu::SettingsMyAccountMenu(QWidget* parent) : FullScreenWidget(parent)
{
        qDebug() << Q_FUNC_INFO;

        setupUi(this);

         /* Please, keep the same order than interface */

        connect(backBtn,SIGNAL(clicked()),getNavigationParent(),SLOT(hideElement()));
        connect(myAccountBtn, SIGNAL(clicked()), this, SLOT(showMyAccount()));
#ifndef DISABLE_ADOBE_SDK
        connect(adobeDrmBtn, SIGNAL(clicked()), this, SLOT(handleAdobeDrmBtn()));
#endif
        connect(generalTermsBtn, SIGNAL(clicked()), this, SLOT(handleGeneralTermsBtn()));

         /**********************************************/
#ifndef DISABLE_ADOBE_SDK
        settingsAdobeDrmLinkDevice = new SettingsAdobeDrmLinkDevice(this);
        settingsAdobeDrmUnLinkDevice = new SettingsAdobeDrmUnLinkDevice(this);
#endif
        settingsUnLinkStoreDevice = new SettingsUnLinkStoreDevice(this);
        detailInformation = new SettingsGeneralTermsInfo(this);

#ifndef DISABLE_ADOBE_SDK
        connect(settingsAdobeDrmLinkDevice, SIGNAL(activatedOK()), this, SLOT(activated()));
        connect(settingsAdobeDrmLinkDevice, SIGNAL(hideMe()), this, SLOT(hideTopElement()));
        connect(settingsAdobeDrmUnLinkDevice, SIGNAL(hideMe()), this, SLOT(hideTopElement()));
        connect(settingsAdobeDrmLinkDevice, SIGNAL(showNewChild(QWidget*)), this, SLOT(showNewChild(QWidget*)));
        connect(settingsAdobeDrmUnLinkDevice, SIGNAL(showNewChild(QWidget*)), this, SLOT(showNewChild(QWidget*)));
        connect(settingsAdobeDrmLinkDevice, SIGNAL(hideChild()), this, SLOT(hideTopElement()));
        connect(settingsAdobeDrmUnLinkDevice, SIGNAL(hideChild()), this, SLOT(hideTopElement()));
#endif
        connect(detailInformation, SIGNAL(hideMe()), this, SLOT(hideTopElement()));
        connect(settingsUnLinkStoreDevice, SIGNAL(hideMe()), this, SLOT(hideTopElement()));
}

SettingsMyAccountMenu::~SettingsMyAccountMenu()
{
        qDebug() << Q_FUNC_INFO;
}

void SettingsMyAccountMenu::setup()
{
    detailInformation->hide();
#ifndef DISABLE_ADOBE_SDK
    settingsAdobeDrmLinkDevice->setup();
    settingsAdobeDrmLinkDevice->hide();
    settingsAdobeDrmUnLinkDevice->setup();
    settingsAdobeDrmUnLinkDevice->hide();
#endif
    settingsUnLinkStoreDevice->setup();
    settingsUnLinkStoreDevice->hide();
}
void SettingsMyAccountMenu::keyReleaseEvent(QKeyEvent *event)
{
        qDebug() << Q_FUNC_INFO;

        if( event->key() == QBook::QKEY_BACK )
        {
            qDebug() << ": Closing...";
            event->accept();
            emit hideMe();
            return;
        }
        QWidget::keyReleaseEvent(event);
}

void SettingsMyAccountMenu::showMyAccount()
{
        qDebug() << Q_FUNC_INFO;
        settingsUnLinkStoreDevice->setup();
        ((Settings*)parent())->showElement(settingsUnLinkStoreDevice);

}

/*TODO: Encapsulate this behavior as soon as possible */
void SettingsMyAccountMenu::handleAdobeDrmBtn()
{
        qDebug() << Q_FUNC_INFO;

        if(ConnectionManager::getInstance()->isConnected()) {
                showAdobeDrm();
        } else{
#ifndef FAKE_WIFI
                QBookApp::instance()->requestConnection(true);
                connectWifiObserverAdobeDrm();
                connect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disconnectWifiObserverAdobeDrm()));
#endif
        };
}

void SettingsMyAccountMenu::showAdobeDrm()
{
   qDebug() << Q_FUNC_INFO;
   disconnectWifiObserverAdobeDrm();
#ifndef DISABLE_ADOBE_SDK
   if(AdobeDRM::getInstance()->isLinked())
   {
       settingsAdobeDrmUnLinkDevice->setup();
       ((Settings *)parent())->showElement(settingsAdobeDrmUnLinkDevice);
   }
   else
       ((Settings *)parent())->showElement(settingsAdobeDrmLinkDevice);
#endif
}

void SettingsMyAccountMenu::connectWifiObserverAdobeDrm()
{
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(showAdobeDrm()),Qt::UniqueConnection);
}

void SettingsMyAccountMenu::disconnectWifiObserverAdobeDrm()
{
    disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(showAdobeDrm()));
}

void SettingsMyAccountMenu::activated(){
        qDebug() << Q_FUNC_INFO;

        ((Settings *)parent())->hideElement();
#ifndef DISABLE_ADOBE_SDK
        settingsAdobeDrmUnLinkDevice->setEmail();
        ((Settings *)parent())->showElement(settingsAdobeDrmUnLinkDevice);
#endif
}

void SettingsMyAccountMenu::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }

void SettingsMyAccountMenu::hideTopElement(){
        qDebug() << Q_FUNC_INFO;
#ifndef DISABLE_ADOBE_SDK
        if(settingsAdobeDrmLinkDevice->keyboard != NULL && settingsAdobeDrmLinkDevice->keyboard->isVisible())
            settingsAdobeDrmLinkDevice->keyboard->hide();
#endif

        ((Settings *)parent())->hideElement();
}

void SettingsMyAccountMenu::handleGeneralTermsBtn(){
        qDebug() << Q_FUNC_INFO;
        Screen::getInstance()->queueUpdates();
        //Need to fill info after show the info to obtain correctly the pages of textBrowser.
        ((Settings*)parent())->showElement(detailInformation);
        detailInformation->fillInfo();
        Screen::getInstance()->flushUpdates();
}

void SettingsMyAccountMenu::showEvent ( QShowEvent * )
{
    qDebug() << Q_FUNC_INFO;
    if (QBook::settings().value("setting/linked").toBool())
        myAccountBtn->show();
    else
        myAccountBtn->hide();
}

void SettingsMyAccountMenu::showNewChild(QWidget * widget)
{
    qDebug() << Q_FUNC_INFO;
    ((Settings*)parent())->showElement(widget);
}
