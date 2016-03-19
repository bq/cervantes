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
#include <QObject>
#include <QPainter>
#include "SettingsAdobeDrmLinkDevice.h"
#include "Settings.h"
#include "SettingsGeneralTermsInfoAdobe.h"
#include "Keyboard.h"
#include "AdobeDRM.h"
#include "QBookApp.h"
#include "ConfirmDialog.h"
#include "InfoDialog.h"
#include "Storage.h"
#include <QFile>
#include <QDir>
#define ERR_ACTIVATION 2
#define ERR_MAX_ACTIVATIONS 16


SettingsAdobeDrmLinkDevice::SettingsAdobeDrmLinkDevice(QWidget* parent): FullScreenWidget(parent)
        , keyboard(NULL)
        , m_waitingDialog(NULL)
        , m_userIDTyped(false)
        , m_userPWTyped(false) {

        qDebug() << Q_FUNC_INFO;
        setupUi(this);

        adobeInformation = new SettingsGeneralTermsInfoAdobe(this);
        adobeInformation->hide();
        licenseAdobeBtn->hide();

        connect(backBtn,SIGNAL(clicked()),this,SIGNAL(hideMe()));
        connect(idAdobeLineEdit, SIGNAL(clicked()), this, SLOT(handleID()));
        connect(passAdobeLineEdit, SIGNAL(clicked()), this, SLOT(handlePW()));
        connect(licenseAdobeBtn, SIGNAL(clicked()), this, SLOT(handleLicenseBtn()));
        connect(adobeInformation, SIGNAL(hideMe()), this, SIGNAL(hideChild()));

        connect(AdobeDRM::getInstance(), SIGNAL(linkOK()),      this, SLOT(showActivatedOK()));
        connect(AdobeDRM::getInstance(), SIGNAL(linkKO(int)),   this, SLOT(showActivatedKO(int)));
}

SettingsAdobeDrmLinkDevice::~SettingsAdobeDrmLinkDevice(){
        qDebug() << Q_FUNC_INFO;
}

void SettingsAdobeDrmLinkDevice::setup()
{
    qDebug() << Q_FUNC_INFO;
    m_initialIDString = idAdobeLineEdit->text();
    m_initialPWString = passAdobeLineEdit->text();
}

void SettingsAdobeDrmLinkDevice::hideEvent ( QHideEvent * )
{
        qDebug() << Q_FUNC_INFO;
        QBookApp::instance()->hideKeyboard();
}

void SettingsAdobeDrmLinkDevice::handleLicenseBtn(){
        qDebug() << Q_FUNC_INFO;

        Screen::getInstance()->queueUpdates();
        emit showNewChild(adobeInformation);
        adobeInformation->fillInfo();
        Screen::getInstance()->flushUpdates();
}

void SettingsAdobeDrmLinkDevice::handleID(){
        qDebug() << Q_FUNC_INFO << ": show Adobe ID Keyboard";
        keyboard = QBookApp::instance()->showKeyboard(tr("Enviar"), false, Keyboard::EMAIL);
        connect(keyboard,SIGNAL(actionRequested()),this, SLOT(handleOK()), Qt::UniqueConnection);
        if(!m_userIDTyped) {
            idAdobeLineEdit->clear();
            idAdobeLineEdit->setFocus();
            m_userIDTyped = true;
        }
        keyboard->handleMyQLineEdit(idAdobeLineEdit);
}

void SettingsAdobeDrmLinkDevice::handlePW (){
        qDebug() << Q_FUNC_INFO << ": show Adobe PW Keyboard";
        keyboard = QBookApp::instance()->showKeyboard(tr("Enviar"), false, Keyboard::EMAIL);
        connect(keyboard,SIGNAL(actionRequested()),this, SLOT(handleOK()), Qt::UniqueConnection);
        connect(keyboard,SIGNAL(newLinePressed()),this, SLOT(handleOK()), Qt::UniqueConnection);
        if(!m_userPWTyped) {
            passAdobeLineEdit->clear();
            passAdobeLineEdit->setFocus();
            m_userPWTyped = true;
        }
        keyboard->handleMyQLineEdit(passAdobeLineEdit);
}

void SettingsAdobeDrmLinkDevice::handleOK (){
        qDebug() << Q_FUNC_INFO;

        if(idAdobeLineEdit->text().length()<=0 || !m_userIDTyped){
            ConfirmDialog* dialog = new ConfirmDialog(this,tr("Adobe ID is empty."));
            dialog->exec();
            delete dialog;
            handleID();
            return;
        }

        qDebug() << Q_FUNC_INFO << ": checking password ";
        if(passAdobeLineEdit->text().length() <= 0 || !m_userPWTyped){
            ConfirmDialog* dialog = new ConfirmDialog(this,tr("Password is empty."));
            dialog->exec();
            delete dialog;
            handlePW();
            return;
        }

        Screen::getInstance()->lockScreen();
        keyboard->hide();

        if (!m_waitingDialog)
            m_waitingDialog = new InfoDialog(this,tr("Please wait, activation in progress ..."));
        m_waitingDialog->show();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->releaseScreen();

        AdobeDRM::getInstance()->link(idAdobeLineEdit->text(), passAdobeLineEdit->text());
}

void SettingsAdobeDrmLinkDevice::showActivatedOK ()
{
    qDebug() << Q_FUNC_INFO;
    if(isVisible())
    {
        if(m_waitingDialog != NULL){
              m_waitingDialog->close();
              delete m_waitingDialog;
              m_waitingDialog = NULL;
        }

        if(keyboard->isVisible())
            keyboard->hide();

        Screen::getInstance()->queueUpdates();
        ConfirmDialog* dialog = new ConfirmDialog(this,tr("Device successfully activated!"));
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();

        dialog->exec();
        delete dialog;

        idAdobeLineEdit->setText(m_initialIDString);
        m_userIDTyped = false;
        passAdobeLineEdit->setText(m_initialPWString);
        m_userPWTyped = false;
        emit activatedOK();
    }
}

void SettingsAdobeDrmLinkDevice::showActivatedKO (int err)
{
        qDebug() << Q_FUNC_INFO << err;
        if(m_waitingDialog != NULL){
            m_waitingDialog->close();
            delete m_waitingDialog;
            m_waitingDialog = NULL;
        }
        if (err == ERR_ACTIVATION) {
            Screen::getInstance()->queueUpdates();
            ConfirmDialog* dialog = new ConfirmDialog(this,tr("Error during activation, please try again"));
            Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
            Screen::getInstance()->flushUpdates();
            dialog->exec();
            delete dialog;
        }
        if(err == ERR_MAX_ACTIVATIONS) {
            Screen::getInstance()->queueUpdates();
            ConfirmDialog* dialog = new ConfirmDialog(this,tr("The account reached the maximum activations."));
            Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
            Screen::getInstance()->flushUpdates();
            dialog->exec();
            delete dialog;
        }
        idAdobeLineEdit->setText(m_initialIDString);
        m_userIDTyped = false;
        passAdobeLineEdit->setText(m_initialPWString);
        m_userPWTyped = false;
}

void SettingsAdobeDrmLinkDevice::paintEvent (QPaintEvent *)
{
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void SettingsAdobeDrmLinkDevice::showEvent (QShowEvent * )
{
    qDebug() << Q_FUNC_INFO;

    if(AdobeDRM::getInstance()->isLinked())
        emit activatedOK();
}
