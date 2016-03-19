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
#include <QPainter>
#include "SettingsAdobeDrmUnLinkDevice.h"
#include "Settings.h"
#include "SettingsGeneralTermsInfoAdobe.h"
#include "Keyboard.h"
#include "AdobeDRM.h"
#include "ConfirmDialog.h"
#include "InfoDialog.h"
#include "SelectionDialog.h"
#include "Screen.h"


SettingsAdobeDrmUnLinkDevice::SettingsAdobeDrmUnLinkDevice(QWidget* parent): FullScreenWidget(parent)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);

    adobeInformation = new SettingsGeneralTermsInfoAdobe(this);
    adobeInformation->hide();
    licenseAdobeBtn->hide();

    connect(overrideDeviceBtn,SIGNAL(clicked()), this, SLOT(handleDeactivate()));
    connect(backBtn,SIGNAL(clicked()),this,SIGNAL(hideMe()));
    connect(licenseAdobeBtn, SIGNAL(clicked()), this, SLOT(handleLicenseBtn()));
    connect(adobeInformation, SIGNAL(hideMe()), this, SIGNAL(hideChild()));

    connect(AdobeDRM::getInstance(), SIGNAL(unlinkOK()), this, SLOT(showDeactivatedOK()));
    connect(AdobeDRM::getInstance(), SIGNAL(unlinkKO()), this, SLOT(showDeactivatedKO()));
}

SettingsAdobeDrmUnLinkDevice::~SettingsAdobeDrmUnLinkDevice()
{
    qDebug() << Q_FUNC_INFO;
}

void SettingsAdobeDrmUnLinkDevice::setup()
{
    qDebug() << Q_FUNC_INFO << AdobeDRM::getInstance()->getAdobeID();
    accountEmailAdobeDrmLbl->setText(AdobeDRM::getInstance()->getAdobeID());
}

void SettingsAdobeDrmUnLinkDevice::handleDeactivate()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    SelectionDialog* confirmDialog = new SelectionDialog(this,tr("Do you want to unlink the device?"));
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
     Screen::getInstance()->flushUpdates();
    confirmDialog->exec();
    bool confirm = confirmDialog->result();
    delete confirmDialog;
    if(!confirm)
        return;

    AdobeDRM::getInstance()->unlink();
}

void SettingsAdobeDrmUnLinkDevice::showDeactivatedOK()
{
    qDebug() << Q_FUNC_INFO;

    if(isVisible())
    {
        Screen::getInstance()->queueUpdates();
        ConfirmDialog* dialog = new ConfirmDialog(this,tr("Deactivated successfully!!"));
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
        dialog->exec();
        delete dialog;
    }
    emit hideMe();
}

void SettingsAdobeDrmUnLinkDevice::showDeactivatedKO()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();
    InfoDialog* dialog = new InfoDialog(this,tr("Deactivation failed!!"),2000);
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
    dialog->hideSpinner();
    dialog->showForSpecifiedTime();
    delete dialog;
}

void SettingsAdobeDrmUnLinkDevice::handleLicenseBtn(){
        qDebug() << Q_FUNC_INFO;

        Screen::getInstance()->queueUpdates();
        emit showNewChild(adobeInformation);
        adobeInformation->fillInfo();
        Screen::getInstance()->flushUpdates();
}

void SettingsAdobeDrmUnLinkDevice::setEmail()
{
    qDebug() << Q_FUNC_INFO << AdobeDRM::getInstance()->getAdobeID();
    accountEmailAdobeDrmLbl->setText(AdobeDRM::getInstance()->getAdobeID());
}

void SettingsAdobeDrmUnLinkDevice::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }
