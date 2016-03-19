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

#include "SettingsDictionariesMenu.h"

#include "QBook.h"
#include "SettingsGeneralTermsInfo.h"
#include "Settings.h"
#include "Dictionary.h"
#include "Screen.h"
#include "SettingsDictionariesInfo.h"
#include "InfoDialog.h"
#include "ConfirmDialog.h"
#include "SelectionDialog.h"
#include "ConnectionManager.h"
#include "QBookApp.h"
#include "bqDeviceServices.h"
#include "Storage.h"
#include "ProgressDialog.h"
#include "PowerManager.h"

#include <QPainter>
#include <QByteArray>
#include <QDebug>
#include <QKeyEvent>


SettingsDictionariesMenu::SettingsDictionariesMenu(QWidget* parent) : FullScreenWidget(parent)
  , downloadDialog(NULL)
  , m_powerLock(NULL)
{

        qDebug() << Q_FUNC_INFO;
        setupUi(this);

        items.append(Item1);
        items.append(Item2);
        items.append(Item3);
        items.append(Item4);
        items.append(Item5);

        connect(Item1, SIGNAL(selectDictionary(QString)), this, SLOT(changeDictionary(QString)));
        connect(Item2, SIGNAL(selectDictionary(QString)), this, SLOT(changeDictionary(QString)));
        connect(Item3, SIGNAL(selectDictionary(QString)), this, SLOT(changeDictionary(QString)));
        connect(Item4, SIGNAL(selectDictionary(QString)), this, SLOT(changeDictionary(QString)));
        connect(Item5, SIGNAL(selectDictionary(QString)), this, SLOT(changeDictionary(QString)));

        /* Please, keep the same order than interface */

        connect(backBtn,SIGNAL(clicked()), this, SIGNAL(hideMe()));
        connect(seeDictionariesInfoBtn, SIGNAL(clicked()), this, SLOT(showDictioInformation()));
        connect(activateExtraDictionaries, SIGNAL(clicked()), this, SLOT(downloadDictionaries()));
        activateExtraDictionaries->setEnabled(true);
        /**********************************************/

        // Connect page handler
        connect(settingsPagerCont,SIGNAL(previousPageReq()),this,SLOT(back()));
        connect(settingsPagerCont,SIGNAL(nextPageReq()),this,SLOT(forward()));

        dictionaryList = Dictionary::instance()->parseCurrentDictionaries(QString::fromUtf8(Dictionary::instance()->getJsonDictionaries())).values();

        detailInformation = new SettingsDictionariesInfo(this);
        detailInformation->hide();

        connect(detailInformation, SIGNAL(hideMe()), this, SIGNAL(hideChild()));
        m_powerLock = PowerManager::getNewLock(this);
}

SettingsDictionariesMenu::~SettingsDictionariesMenu()
{
        qDebug() << Q_FUNC_INFO;
        delete m_powerLock;
        m_powerLock = NULL;
}

int SettingsDictionariesMenu::getTotalPages()
{
        qDebug() << Q_FUNC_INFO;

        if(dictionaryList.size() == 0)
            return 1;
        if(dictionaryList.size() % items.size() == 0)
            return dictionaryList.size() / items.size();
        else
            return (dictionaryList.size() / items.size()) + 1;
}

void SettingsDictionariesMenu::init()
{
        qDebug() << Q_FUNC_INFO;

        if(dictionaryList.size() == 0)
            return;

        page = 0;
        settingsPagerCont->setup(getTotalPages());

        getCurrentDictionary();

        for (int i = 0; i < dictionaryList.size(); ++i){
            if(dictionaryList[i].id == currentDictio){
                dictionaryList.prepend(dictionaryList.takeAt(i));
            }
            dictionaryList[i].defaultDictio = false;
        }
        dictionaryList.first().defaultDictio = true;

        paintDictios();
}

void SettingsDictionariesMenu::setupDictionaries()
{
        qDebug() << Q_FUNC_INFO;

        if(dictionaryList.size() == 0)
            return;

        page = 0;
        settingsPagerCont->setup(getTotalPages());

        getCurrentDictionary();

        dictionaryList.first().defaultDictio = false;
        for (int i = 0; i < dictionaryList.size(); ++i){
            if(dictionaryList[i].id == currentDictio){
                dictionaryList.prepend(dictionaryList.takeAt(i));
                break;
            }
            dictionaryList[i].defaultDictio = false;
        }
        dictionaryList.first().defaultDictio = true;

        paintDictios();
}

void SettingsDictionariesMenu::getCurrentDictionary()
{
        qDebug() << Q_FUNC_INFO;

        currentDictio = QBook::settings().value("setting/dictionary", "").toString();
        qDebug() << Q_FUNC_INFO << currentDictio;

}

void SettingsDictionariesMenu::forward ()
{
        qDebug() << Q_FUNC_INFO;

        page++;
        paintDictios();

}

void SettingsDictionariesMenu::back()
{
        qDebug() << Q_FUNC_INFO;
        page--;

        if(page < 0)
            page = 0;

        paintDictios();
}

void SettingsDictionariesMenu::changeDictionary(QString id)
{
        qDebug() << Q_FUNC_INFO;

        Screen::getInstance()->queueUpdates();
        QBook::settings().setValue("setting/dictionary", id);
        QBook::settings().sync();

        currentDictio = id;
        setupDictionaries();
        Screen::getInstance()->flushUpdates();
        emit hideMe();
        InfoDialog * dictionaryDialog = new InfoDialog(this, tr("Diccionario cambiado correctamente"), 3000);
        dictionaryDialog->hideSpinner();
        dictionaryDialog->showForSpecifiedTime();
        delete dictionaryDialog;
}

void SettingsDictionariesMenu::showDictioInformation()
{
        qDebug() << Q_FUNC_INFO;
        Screen::getInstance()->queueUpdates();
        emit showNewChild(detailInformation);
        detailInformation->fillInfo();
        Screen::getInstance()->flushUpdates();
}

void SettingsDictionariesMenu::paintDictios()
{
        qDebug() << Q_FUNC_INFO << "size:" << dictionaryList.size();

        const int listSize = dictionaryList.size();
        const int pageOffset = page*items.size();
        const int size = items.size()-1;
        for(int i=0; i <= size; ++i){
            SettingsDictionariesMenuItem* item = items.at(i);
            int pos = pageOffset + i;
            if(pos < listSize){
                qDebug() << Q_FUNC_INFO << "id: " << dictionaryList.at(pos).id;
                item->paint(dictionaryList.at(pos));
                item->show();
            }else
                item->paint(DictionaryParams());
                item->show();
        }
}

void SettingsDictionariesMenu::paintEvent (QPaintEvent *)
{
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void SettingsDictionariesMenu::downloadDictionaries()
{
    qDebug() << Q_FUNC_INFO;

    if(!ConnectionManager::getInstance()->isConnected())
    {
        connectDictionaryWifiObserver();
        connect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disconnectDictionaryWifiObserver()));
        QBookApp::instance()->requestConnection();
        return;
    }

    disconnectDictionaryWifiObserver();

    if(newDictionaryAvailable())
    {
        m_powerLock->activate();
        activateExtraDicts();
        m_powerLock->release();
    }
}

void SettingsDictionariesMenu::activateExtraDicts()
{
    qDebug() << Q_FUNC_INFO;
    bqDeviceServices* services = QBookApp::instance()->getDeviceServices();
    services->processSyncCanceled();
    b_downloadCanceled = false;

    if(!downloadDialog)
    {
        downloadDialog = new ProgressDialog(this,tr("Downloading dictionary, please wait for a few minutes."));
        downloadDialog->setHideBtn(false);
        downloadDialog->setTextValue(false);
        downloadDialog->setModal(true);
        connect(downloadDialog, SIGNAL(cancel()),               this,          SLOT(downloadCanceled()), Qt::UniqueConnection);
        connect(services,       SIGNAL(downloadProgress(int)), downloadDialog, SLOT(setProgressBar(int)), Qt::UniqueConnection);
    }

    downloadDialog->setProgressBar(0);
    bool installingPending = false;
    newDictionaryNames.clear();

    QList<DictionaryParams>::const_iterator it = pendingDictsList.constBegin();
    QList<DictionaryParams>::const_iterator itEnd = pendingDictsList.constEnd();
    while(it != itEnd)
    {
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
        downloadDialog->show();

        if(!(*it).activationState)
        {
            int dixioSize = (*it).dixFileSize * 1024 * 1024;
            int downloadComplementSize = (*it).compFileSize * 1024 * 1024;

            QString downloadPath = Storage::getInstance()->getPrivatePartition()->getMountPoint() + QDir::separator() + "dictionaries" + QDir::separator();
            newDictionaryNames.append(downloadPath + QString((*it).downloadFileName));
            bool downloaded = services->downloadDictionary((*it).downloadUrl, downloadPath + QString((*it).downloadFileName),dixioSize);

            if(downloaded)
            {
                installingPending = true;
                if((*it).downloadUrlComp.size())
                {
                    qDebug() << Q_FUNC_INFO << "downloadUrlComp: " << (*it).downloadUrlComp;
                    downloadDialog->setText(tr("Downloading complements, please wait for a few minutes."));
                    Screen::getInstance()->refreshScreen();
                    newDictionaryNames.append(downloadPath + QString((*it).downloadFileNameComp));
                    if(!services->downloadDictionary((*it).downloadUrlComp, downloadPath + QString((*it).downloadFileNameComp), downloadComplementSize))
                    {
                        downloaded = false;
                        installingPending = false;
                    }
                }
            }

            if(!downloaded && !b_downloadCanceled)
            {
                delete downloadDialog;
                downloadDialog = NULL;
                deleteDictionaryFiles(newDictionaryNames);
                ConfirmDialog* confirmDialog = new ConfirmDialog(this,tr("Download failed. Please try again."));
                Screen::getInstance()->resetQueue();
                Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE|FLAG_IGNORE_QUEUE,Q_FUNC_INFO);
                confirmDialog->exec();
                delete confirmDialog;
                Screen::getInstance()->refreshScreen();
                return;
            }
        }
        it++;
    }

    if(installingPending){
        downloadDialog->setText(tr("Installing..."));
        downloadDialog->hideCancelButton();
        Screen::getInstance()->refreshScreen();
    }else{
        if(downloadDialog){
            delete downloadDialog;
            downloadDialog = NULL;
        }
        return;
    }

    QString dixioKey = Dictionary::instance()->getDixioKey();
    qDebug() << Q_FUNC_INFO << "dixioKey: " << dixioKey;
    QString callUrl;
    QString postParameters;
    QString activationResponse;
    bool installingError = false;

    // Get activation data
    if(Dictionary::instance()->getActivation(dixioKey, callUrl, postParameters))
    {
        QString activationRequest = postParameters.split("activationRequest=")[1];
        if(services->callActivationDictionary(activationResponse, activationRequest))
        {
            qDebug() << Q_FUNC_INFO << "activationResponse: " << activationResponse;
            QString activation = QString::fromStdString(Dictionary::instance()->setActivation(dixioKey, activationResponse));
            qDebug() << Q_FUNC_INFO << "activation: " << activation;
            installingError = activation.contains("error");
        }else
            installingError = true;
    }else
        installingError = true;

    delete downloadDialog;
    downloadDialog = NULL;
    ConfirmDialog* confirmDialog;
    if(!installingError)
    {
        confirmDialog = new ConfirmDialog(this,tr("Installing success. Now the system will be turned off."));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
        confirmDialog->exec();
        delete confirmDialog;
        QBook::settings().setValue("setting/dictionaryAvailable", false);
        QBook::settings().sync();
        Screen::getInstance()->refreshScreen(Screen::MODE_SAFE,true);
        PowerManager::powerOffDevice(false);
    }else
    {
        deleteDictionaryFiles(newDictionaryNames);
        confirmDialog = new ConfirmDialog(this,tr("Install failed. Please try again."));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
        confirmDialog->exec();
        delete confirmDialog;
        Screen::getInstance()->refreshScreen();
    }
}

void SettingsDictionariesMenu::showEvent(QShowEvent *)
{
    init();

    if(!QBook::settings().value("setting/dictionaryAvailable", false).toBool())
        activateExtraDictionaries->hide();
    else
        activateExtraDictionaries->show();
}

void SettingsDictionariesMenu::downloadCanceled()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getDeviceServices()->cancelDownload();
    b_downloadCanceled = true;

    if(downloadDialog)
    {
        deleteDictionaryFiles(newDictionaryNames);
        downloadDialog->hide();
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
        delete downloadDialog;
        downloadDialog = NULL;
    }
    Screen::getInstance()->flushUpdates();
}

void SettingsDictionariesMenu::deleteDictionaryFiles(QStringList dictionaryNames)
{
    qDebug() << Q_FUNC_INFO;
    foreach(QString dictionary, dictionaryNames)
        QFile::remove(dictionary);
}

bool SettingsDictionariesMenu::newDictionaryAvailable()
{
    qDebug() << Q_FUNC_INFO;

    availableDictsList = QBookApp::instance()->getAvailableDicts();
    pendingDictsList.clear();
    bool newDictionaryFound = false;

    QList<DictionaryParams>::iterator it = availableDictsList.begin();
    QList<DictionaryParams>::iterator itEnd = availableDictsList.end();
    while(it != itEnd)
    {
        qDebug() << Q_FUNC_INFO << "id: " << (*it).id << " commercialName: " << (*it).commercialName;
        newDictionaryFound = true;

        for (int i = 0; i < dictionaryList.size(); i++){
            if(dictionaryList[i].id == (*it).id){
                newDictionaryFound = false;
                break;
            }
        }

        (*it).activationState = false;
        if(newDictionaryFound)
            pendingDictsList.append(*it);

        ++it;
    }

    return pendingDictsList.size();
}

void SettingsDictionariesMenu::connectDictionaryWifiObserver()
{
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(downloadDictionaries()), Qt::UniqueConnection);
}

void SettingsDictionariesMenu::disconnectDictionaryWifiObserver()
{
    disconnect(ConnectionManager::getInstance(),SIGNAL(connected()),this,SLOT(downloadDictionaries()));
}
