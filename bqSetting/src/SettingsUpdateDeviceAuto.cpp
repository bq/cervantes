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
#include <QEventLoop>
#include <QScrollBar>

#include "SettingsUpdateDeviceAuto.h"
#include "QBook.h"
#include "Battery.h"
#include "InfoDialog.h"
#include "QBookApp.h"
#include "ConfirmDialog.h"
#include "SelectionDialog.h"
#include "ADConverter.h"
#include "Screen.h"
#include "QFwUpgradeOTA.h"
#include "ProgressDialog.h"
#include "json.h"

#define BATTERY_LEVEL_FOR_OTA 50
#define MAGIC_UPDATE_CODE 255
#define PERCENT_STEP_VALUE 0.75

SettingsUpdateDeviceAuto::SettingsUpdateDeviceAuto(QWidget * parent) : FullScreenWidget(parent)
    , m_fwCheck(NULL)
    , m_converter(NULL)
{
            qDebug() << Q_FUNC_INFO;
            setupUi(this);
            updateAutoLbl->setText(tr("Wait while searching available updates"));
            updateNumberLbl->hide();
            updateAutoBtn->hide();
            connect(backBtn, SIGNAL(clicked()), this, SIGNAL(hideMe()));
            connect(updateAutoBtn, SIGNAL(clicked()), this, SLOT(updateAutomatic()));
            connect(settingsPagerCont, SIGNAL(previousPageReq()),  this , SLOT(previousPage()));
            connect(settingsPagerCont, SIGNAL(nextPageReq()), this, SLOT(nextPage()));

            updateAutoBtn->hide();
            releaseNotesText->hide();
            settingsPagerCont->hide();
            releaseNotesText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            vbar = releaseNotesText->verticalScrollBar();
            setupPagination();
            m_converter = ADConverter::getInstance();
            m_fwCheck = new QFwUpgradeOTA();
}

void SettingsUpdateDeviceAuto::doCheck() {
    qDebug() << Q_FUNC_INFO;
#ifndef HACKERS_EDITION
    connect(m_fwCheck, SIGNAL(checkOTAAvailableFinished(int)), this, SLOT(gotOTACheckResult(int)));
    if (!QBookApp::instance()->isActivated() && !QBookApp::instance()->isLinked()) {
            disconnect(m_fwCheck, SIGNAL(checkOTAAvailableFinished(int)), this, SLOT(gotOTACheckResult(int)));
            updateAutoLbl->setText(tr("There aren't new updates available while you are not linked."));
            updateNumberLbl->hide();
            updateAutoBtn->hide();
            releaseNotesText->hide();
            return;
    }

    if (!m_fwCheck->checkOTAAvailable()) {
            disconnect(m_fwCheck, SIGNAL(checkOTAAvailableFinished(int)), this, SLOT(gotOTACheckResult(int)));
            updateAutoLbl->setText(tr("There aren't new updates available"));
            updateNumberLbl->hide();
            updateAutoBtn->hide();
            releaseNotesText->hide();
            settingsPagerCont->hide();
	return;
    }
#endif
}

SettingsUpdateDeviceAuto::~SettingsUpdateDeviceAuto()
{
    qDebug() << Q_FUNC_INFO;
    delete m_fwCheck;
    m_fwCheck = NULL;
    m_converter = NULL;
}

void SettingsUpdateDeviceAuto::setupPagination (){

    qDebug() << Q_FUNC_INFO;
    int pageNumber;
    if(vbar->maximum() % vbar->singleStep() == 0)
        pageNumber = vbar->maximum() / vbar->singleStep() + 1;
    else
        pageNumber = vbar->maximum() / vbar->singleStep() + 2;
    settingsPagerCont->setup(pageNumber);
}

void SettingsUpdateDeviceAuto::previousPage()
{
    qDebug() << Q_FUNC_INFO;
    vbar->triggerAction(QAbstractSlider::SliderSingleStepSub);
}

void SettingsUpdateDeviceAuto::nextPage()
{
    qDebug() << Q_FUNC_INFO;
    vbar->triggerAction(QAbstractSlider::SliderSingleStepAdd);
}

void SettingsUpdateDeviceAuto::gotOTACheckResult(int result) {
    qDebug() << Q_FUNC_INFO;
    disconnect(m_fwCheck, SIGNAL(checkOTAAvailableFinished(int)), this, SLOT(gotOTACheckResult(int)));
    Screen::getInstance()->queueUpdates();
    if (result == QFwUpgradeOTA::Error) {
        qDebug() << Q_FUNC_INFO << "Error checking OTA, doing nothing";
        updateAutoLbl->setText(tr("There aren't new updates available"));
        updateNumberLbl->hide();
        updateAutoBtn->hide();
        releaseNotesText->hide();
        settingsPagerCont->hide();
    } else if (result == QFwUpgradeOTA::NotAvailable){
        qDebug() << Q_FUNC_INFO << "No OTA available , doing nothing";
        updateAutoLbl->setText(tr("There aren't new updates available"));
        updateNumberLbl->hide();
        updateAutoBtn->hide();
        releaseNotesText->hide();
        settingsPagerCont->hide();
    } else if (result == QFwUpgradeOTA::Available) {
        qDebug() << Q_FUNC_INFO << "OTA available , asking for download";
        updateAutoLbl->setText(tr("There are a new version available:"));
        updateNumberLbl->setText(m_fwCheck->getOTAVersionString());
        updateNumberLbl->show();
        updateAutoBtn->show();

        QVariantMap releaseNotesMap = m_fwCheck->getOTAReleaseNotes();

        QString releaseNotes;
        if(!releaseNotesMap.empty()){
            releaseNotes = m_fwCheck->getOTAReleaseNotes()[QBook::settings().value("setting/language", QVariant("es")).toString()].toString();
            if(releaseNotes.isEmpty()){
                releaseNotes = m_fwCheck->getOTAReleaseNotes()["es"].toString();
            }
        }

        if (!releaseNotes.isEmpty()) {
            releaseNotesText->setText(releaseNotes);
            releaseNotesText->show();
            settingsPagerCont->show();
        } else {
            releaseNotesText->hide();
            settingsPagerCont->hide();
        }
        //Set step like the 75% of the height to view all text.
        vbar->setSingleStep(releaseNotesText->height()*PERCENT_STEP_VALUE);
        setupPagination();

   }
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void SettingsUpdateDeviceAuto::updateAutomatic()
{
    qDebug() << Q_FUNC_INFO;

    if(!m_fwCheck->checkUpdateInfo())
    {
        m_fwCheck->checkOTAAvailable();
        ConfirmDialog* confirmDialog = new ConfirmDialog(this,tr("Error downloading.\n Please try again."));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        confirmDialog->exec();
        delete confirmDialog;
        return;
    }

    if (!m_fwCheck->enoughMemForUpdate()) // Check memory availability
    {
        ConfirmDialog* confirmDialog = new ConfirmDialog(this,tr("No space available on the device.\nPlease remove some files."));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        confirmDialog->exec();
        delete confirmDialog;
        return;
    }

    // Check the charger
    if(m_converter->getStatus() == ADConverter::ADC_STATUS_NO_WIRE)
    {
        if(Battery::getInstance()->getLevel() < BATTERY_LEVEL_FOR_OTA)
        {
            SelectionDialog* dialogSelect = new SelectionDialog(this,tr("Low battery level.\nPlease connect the charger\nbefore the download."));
            while(m_converter->getStatus() == ADConverter::ADC_STATUS_NO_WIRE)
            {
                Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
                dialogSelect->exec();
                if(!dialogSelect->result())
                {
                    delete dialogSelect;
                    return;
                }
            }
            delete dialogSelect;
        }
    }
    else
    {
        if(Battery::getInstance()->getLevel() < BATTERY_LEVEL_FOR_OTA)
        {
            InfoDialog* infoDialog = new InfoDialog(this,tr("Low battery level.\nPlease, don't disconnect the charger\n during the download."), 3000);
            infoDialog->showForSpecifiedTime();
            delete infoDialog;
        }
    }

    QBookApp::instance()->setOTAUpdating(true);
    ProgressDialog *downloadingDialog = new ProgressDialog(this,tr("Downloading...\nThis process may take several minutes "));
    connect(m_fwCheck, SIGNAL(OTAProgress(int)), downloadingDialog, SLOT(setProgressBar(int)));
    connect(downloadingDialog, SIGNAL(cancel()), m_fwCheck, SLOT(requestCancel()));
   // FIXME: As we do show in here, we should block UI events 
    downloadingDialog->show();
    bool res = m_fwCheck->download();

    if(m_fwCheck->getCancelRequested()){
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        downloadingDialog->accept();
        delete downloadingDialog;
        QBookApp::instance()->setOTAUpdating(false);
        return;
    }

    if (!res) {
        downloadingDialog->accept();
        delete downloadingDialog;
        ConfirmDialog* infoDialog = new ConfirmDialog(this,tr("Error downloading"));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        infoDialog->exec();
        delete infoDialog;
        QBookApp::instance()->setOTAUpdating(false);
        return;
    }

    downloadingDialog->setProgressBar(100);
    downloadingDialog->hide();
    delete downloadingDialog;
    InfoDialog* installationDialog = new InfoDialog(this,tr("Firmware download complete\nRestarting to apply changes."));
    installationDialog->show();
    QCoreApplication::processEvents();

    if (!m_fwCheck->checkFileValidity())
    {
        delete installationDialog;
        ConfirmDialog* infoDialog = new ConfirmDialog(this,tr("Downloaded file is corrupted"));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        infoDialog->exec();
        delete infoDialog;
        QBookApp::instance()->setOTAUpdating(false);
        return;
    }

    updateNumberLbl->hide();
    updateAutoBtn->hide();

    if(!m_fwCheck->doUpdate())
    {
        delete installationDialog;
        ConfirmDialog* infoDialog = new ConfirmDialog(this,tr("Installation failed. Please try again in a few minutes"));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        infoDialog->exec();
        delete infoDialog;
    }
    else{
        Screen::getInstance()->queueUpdates();
        delete installationDialog;
        QBookApp::instance()->showRestoringImage();
        Screen::getInstance()->flushUpdates();
        Screen::getInstance()->lockScreen();
        ::sync();
        QCoreApplication::exit(MAGIC_UPDATE_CODE);
    }

    QBookApp::instance()->setOTAUpdating(false);
}

void SettingsUpdateDeviceAuto::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }
