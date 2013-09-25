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
#include <QDebug>
#include <QKeyEvent>
#include <QFuture>
#include <QtCore>
#include <QDateTime>

#include "SettingsUnLinkStoreDevice.h"
#include "QBook.h"
#include "QBookApp.h"
#include "SelectionDialog.h"
#include "InfoDialog.h"
#include "Storage.h"
#include "PowerManager.h"
#include "Screen.h"
#include "Model.h"

#ifndef HACKERS_EDITION
#include "bqDeviceServices.h"
#endif

SettingsUnLinkStoreDevice::SettingsUnLinkStoreDevice(QWidget *parent) : FullScreenWidget(parent)
{
        setupUi(this);
        connect(backBtn,SIGNAL(clicked()), this, SIGNAL(hideMe()));
        connect(overrideDeviceBtn, SIGNAL(clicked()), this, SLOT(handleUnlinkBtn()));
        statusLbl->hide();
        statusValLbl->hide();
        warningMsgLbl->hide();
}

SettingsUnLinkStoreDevice::~SettingsUnLinkStoreDevice()
{
        qDebug() << Q_FUNC_INFO;
}

void SettingsUnLinkStoreDevice::setup()
{
    qDebug() << Q_FUNC_INFO;
    QString email = QBook::settings().value("eMail").toString();
    accountEmailLbl->setText(email);
    QString name = QBook::settings().value("name").toString();
    QString surname = QBook::settings().value("surname").toString();
    userValueLbl->setText(name + " " + surname);
    QString shop = QBook::settings().value("shopName").toString().toUpper();
    QString shopUserAccountUrl = QBook::settings().value("shopUserAccountUrl").toString();
    QDateTime lastSyncDate = QDateTime::fromString(QBook::settings().value("setting/library/lastSync").toString(), Qt::ISODate);
    lastSyncValLbl->setText(lastSyncDate.toString("dd/MM/yy hh:mm"));
    sectionTitleLbl->setText(tr("Cuenta %1").arg(shop));
    accountEmailLbl->setText(email);
    updateAccountLbl->setText(tr("If  you need to update your account, enter at\n %1").arg(shopUserAccountUrl));

    if(QBook::settings().value("subscription/subscriptionAllowed").toBool()) {
        subscriptionLbl->setText(tr("Subscribed to %1 PREMIUM.").arg(shop));
        showSubscriptionInfo();
    }
    else{
        subscriptionLbl->setText(tr("This device was linked to %1 ebooks.").arg(shop));
        statusLbl->hide();
        notelbl->hide();
        statusLastSync->hide();
        statusValLbl->hide();
        warningMsgLbl->hide();
    }
}

void SettingsUnLinkStoreDevice::showSubscriptionInfo()
{
    qDebug() << Q_FUNC_INFO;
    QDateTime datePaymentEnd = QDateTime::fromString(QBook::settings().value("subscription/subscriptionEndDate").toString(), Qt::ISODate);
    QDateTime datePeriodEnd = QDateTime::fromString(QBook::settings().value("subscription/canReadSubscriptionBooksEndDate").toString(), Qt::ISODate);
    statusLbl->setText(tr("Status:"));
    statusLbl->show();
    int status = QBook::settings().value("subscription/status").toInt();
    bool cancelled = QBook::settings().value("subscription/cancelled").toBool();
    bool pendingPayment = QBook::settings().value("subscription/pendingPayment").toBool();
    QString shopUserAccountUrl = QBook::settings().value("shopUserAccountUrl").toString();
    switch(status){
        case 0:
        if(QDateTime::currentDateTime() > datePeriodEnd)
        {
            warningMsgLbl->setText(tr("Tu suscripcion ha caducado. Reactiva tu cuenta Premium accediendo a %1").arg(shopUserAccountUrl));
            warningMsgLbl->show();
            break;
        }
        if(cancelled && QDateTime::currentDateTime() < datePaymentEnd)
        {
            statusValLbl->setText(tr("active"));
            statusValLbl->show();
            warningMsgLbl->setText(tr("Tu cuenta estará activa hasta el %1. A partir de entonces, tu cuenta pasará a estar cancelada, disponiendo de un período de cortesía de 30 días para que termines de leer los eBooks activos en tus dispositivos.").arg(datePaymentEnd.toString("dd/MM/yyyy")));
            warningMsgLbl->show();
            break;
        }
        else
        {
            statusValLbl->setText(tr("active"));
            statusValLbl->show();
            if(datePaymentEnd < QDateTime::currentDateTime() && QDateTime::currentDateTime() < datePeriodEnd)
            {
                warningMsgLbl->setText(tr("Te quedan solo %1 dias para seguir leyendo offline en tu dispositivo. Conecta tu dispositivo a Internet y sincroniza tu biblioteca. Transcurrido este plazo sin haber sincronizado tu dispositivo, tus eBooks no seran accesibles hasta nueva sincronizacion").arg(QDateTime::currentDateTime().daysTo(datePeriodEnd)));
                warningMsgLbl->show();
            }
            else
                warningMsgLbl->hide();
            break;
        }
        case 1:
        if(cancelled)
        {
            statusValLbl->setText(tr("cancelled"));
            statusValLbl->show();
            if(QDateTime::currentDateTime() < datePeriodEnd) {
                warningMsgLbl->setText(tr("Hasta el %1 disfrutaras de un periodo de cortesia para que termines de leer los eBooks activos en tu dispositivo, no pudiendo añadir nuevos eBooks ni realizar ninguna otra accion sobre tu biblioteca.").arg(datePeriodEnd.toString("dd/MM/yyyy")));
                warningMsgLbl->show();
            }
            else if(QDateTime::currentDateTime() > datePeriodEnd) {
                warningMsgLbl->setText(tr("Actualmente, tu cuenta esta cancelada. No podras disfrutar de nuestro servicio ni continuar accediendo a tus eBooks desde los dispositivos que tengas asociados a tu cuenta, salvo que  procedas a la reactivacion de tu cuenta Premium. Puedes reactivarla ahora, accediendo a %1").arg(shopUserAccountUrl));
                warningMsgLbl->show();
            }
            break;
        }
        else if(pendingPayment)
        {
            statusValLbl->setText(tr("pending Payment"));
            statusValLbl->show();
            warningMsgLbl->setText(tr("Accede %1 y renueva tu suscripción. Hasta el %2 disfrutaras de un periodo de cortesia para que termines de leer los eBooks activos en tu dispositivo, no pudiendo añadir nuevos eBooks ni realizar ninguna otra accion sobre tu biblioteca.").arg(shopUserAccountUrl).arg(datePeriodEnd.toString("dd/MM/yyyy")));
            warningMsgLbl->show();
            break;
        }
        case 2:
            statusValLbl->setText(tr("Trial"));
            statusValLbl->show();
            if(QDateTime::currentDateTime() < datePeriodEnd)
            {
                warningMsgLbl->setText(tr("Actualmente estás disfrutando del periodo de prueba en Nubico Premium. Podrás seguir disfrutando de la versión de prueba hasta el %1.").arg(datePeriodEnd.toString("dd/MM/yyyy")));
                warningMsgLbl->show();
            }
            else
                warningMsgLbl->hide();
            break;
        case 3:
            statusValLbl->setText(tr("unsubscribed"));
            statusValLbl->show();
            warningMsgLbl->hide();
            break;
    }
}

void SettingsUnLinkStoreDevice::handleUnlinkBtn()
{
        qDebug() << Q_FUNC_INFO;

        if(ConnectionManager::getInstance()->isConnected()) {
                tryToDisassociate();
        } else{
#ifndef FAKE_WIFI
                connectWifiObserverUnlinkBtn();
                connect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disconnectWifiObserverUnlinkBtn()));
                QBookApp::instance()->requestConnection(true);
#endif
        }

}

void SettingsUnLinkStoreDevice::tryToDisassociate()
{
        qDebug() << "--->" << Q_FUNC_INFO;
        disconnectWifiObserverUnlinkBtn();
        Screen::getInstance()->queueUpdates();
        SelectionDialog* confirmDialog = new SelectionDialog(this,tr("Do you want to unlink the device? This will delete all your personal account data"));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        Screen::getInstance()->flushUpdates();
        confirmDialog->exec();
        bool result = confirmDialog->result();
        delete confirmDialog;
        if(!result)
            return;

        disconnect(Storage::getInstance(), SIGNAL(partitionMounted(StoragePartition*)), QBookApp::instance(), SLOT(handlePartitionMounted(StoragePartition*)));
        PowerManagerLock* powerLock = PowerManager::getNewLock(this);
        powerLock->activate();
        QBookApp::instance()->cancelSync();
        //Screen::getInstance()->queueUpdates();
        InfoDialog *dialog = new InfoDialog(this,tr("This process can take a few minutes. At the end, the device will be powered off"));
        dialog->show();
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
        //Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
        //Screen::getInstance()->flushUpdates();

#ifndef HACKERS_EDITION
        bqDeviceServices* services = QBookApp::instance()->getDeviceServices();
        services->unlink();
#endif
        
        if (true)
        {
            QBookApp::instance()->disableUserEvents();
            QBookApp::instance()->getSyncHelper()->SetServerTimestamp(0);
            QBookApp::instance()->syncModel();
            QBookApp::instance()->showRestoringImage();

            Screen::getInstance()->queueUpdates();
            // Remove private books from model
            QBookApp::instance()->getModel()->removeDir(Storage::getInstance()->getPrivatePartition()->getMountPoint());

                QFuture<void> future = QtConcurrent::run(this, &SettingsUnLinkStoreDevice::restoreSettinsAndPartitions, dialog);
                if (!future.isFinished())
                    dialog->exec();
            Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
            PowerManager::powerOffDevice(true);
        }
        else
        {

            InfoDialog *unlinkFailedDialog = new InfoDialog(this,tr("Service unavailable. Please try again in a few minutes"));
            Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
            unlinkFailedDialog->hideSpinner();
            unlinkFailedDialog->showForSpecifiedTime();
            delete unlinkFailedDialog;
            powerLock->release();
            delete powerLock;
        }
}

void SettingsUnLinkStoreDevice::restoreSettinsAndPartitions(QDialog *dialog)
{
        qDebug() << Q_FUNC_INFO <<  ": Format private partition";

    // Clear data partition (except dictionary files)
    QString cmd = "rm -r `find " + Storage::getInstance()->getDataPartition()->getMountPoint() + "/* | grep -v dictionaries`";
    system(cmd.toAscii().constData());

    // Restore connman dir on data partition, as it comes from factory
    QDir(Storage::getInstance()->getDataPartition()->getMountPoint()).mkpath("connman");

    // Format private partition
        Storage *storage = Storage::getInstance();
        storage->formatStoragePartition(storage->getPrivatePartition());
        if (dialog)
            dialog->accept();
}

void SettingsUnLinkStoreDevice::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }

void SettingsUnLinkStoreDevice::showEvent (QShowEvent * )
{
    qDebug() << Q_FUNC_INFO;
    setup();
}


void SettingsUnLinkStoreDevice::connectWifiObserverUnlinkBtn()
{
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(tryToDisassociate()),Qt::UniqueConnection);
}

void SettingsUnLinkStoreDevice::disconnectWifiObserverUnlinkBtn()
{
    disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(tryToDisassociate()));
}
