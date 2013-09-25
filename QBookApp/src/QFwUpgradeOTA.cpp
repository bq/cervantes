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

/*  QFwUpgradeOTA.cpp

    This class defines handles download of firmware file
    from the server in parallel with normal execution. When
    done fw upgrade is prepared to be done ready after restart.

*/

#include <unistd.h>

#include "version.h"

#include "QFwUpgradeOTA.h"
#include <QDebug>
#include <QTimer>
#include "QBook.h"
#include "QBookForm.h"
#include <QFileInfo>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QSettings>
#include <QDir>
#include <QCryptographicHash>

#include "PowerManager.h"
#include "DeviceInfo.h"
#include "Storage.h"
#include "QBookApp.h"
#include "json.h"
#include "Screen.h"


#define DOWNLOAD_TIMEOUT 15000
#define DOWNLOAD_BUFFER_SIZE 10 * 1024

#define KEYRING_PATH "/etc/bqkeyring"

QFwUpgradeOTA::QFwUpgradeOTA()
    : m_qnam(NULL)
    , downloaded(false)
    , checkReply(NULL)
    , updateInfo(NULL)
{
    m_qnam = new QNetworkAccessManager(this);
    lock = PowerManager::getNewLock(this);
    QString downloadPath = Storage::getInstance()->getPrivatePartition()->getMountPoint();
    downloadFile = downloadPath + "/" + DOWNLOAD_FILENAME;
    OTAFile = downloadPath + "/" + OTA_FILENAME;
    if (QFile::exists(OTAFile)) {
	qDebug() << Q_FUNC_INFO << "Removing previous OTA file: " << OTAFile;
	QFile::remove(OTAFile);
    }
}

QFwUpgradeOTA::~QFwUpgradeOTA()
{
    if (m_qnam) {
        delete m_qnam;
        m_qnam = NULL;
    }

    if (updateInfo) {
        delete updateInfo;
        updateInfo = NULL;
    }

    /** No need of checkReply deletion, this destructor is only called
    when turning off. Calling checkReply deletion before network reply
    causes segmentation fault due to previous deletion of object */

    if (lock) {
        lock->release();
        delete lock;
        lock = NULL;
    }
}

bool QFwUpgradeOTA::checkOTAAvailable(){
    qDebug() << Q_FUNC_INFO;

    if (checkReply)
        delete checkReply;

    if (updateInfo) {
        delete updateInfo;
        updateInfo = NULL;
    }

    QString host = QBook::settings().value("wsServicesURL", "").toString();
    if (host.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "wsServicesURL is empty, probably device not linked, aborting OTA check";
        return false;
    }

    QString model = QBook::settings().value("deviceModelName").toString();
    QString sn = DeviceInfo::getInstance()->getSerialNumber();

    if (model.isEmpty() || sn.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "Cannot retrieve model, or SN. Aborting checkUpdate";
        return false;
    }

    lock->activate();

    QUrl url = QUrl (host + "/checkUpdate/reader/" + model + "/" + sn + "/" + QBOOKAPP_VERSION);
    qDebug() << Q_FUNC_INFO << url.toString();
    QNetworkRequest* request = new QNetworkRequest(url);
    checkReply = m_qnam->get(*request);
    connect(checkReply, SIGNAL(sslErrors(QList<QSslError>)), checkReply, SLOT(ignoreSslErrors()));
    connect(checkReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(checkReplyError(QNetworkReply::NetworkError)));
    connect(checkReply, SIGNAL(finished()), this, SLOT(checkReplyFinished()));

    delete request;
    return true;
}

void QFwUpgradeOTA::checkReplyError(QNetworkReply::NetworkError error) {
    qDebug() << Q_FUNC_INFO << error;
    lock->release();
    emit checkOTAAvailableFinished(QFwUpgradeOTA::Error);
}

void QFwUpgradeOTA::checkReplyFinished() {
    QString content = QString::fromUtf8 (checkReply->readAll());
    lock->release();
    qDebug() << Q_FUNC_INFO << ": Reply content: " << content;

    checkReply->deleteLater();
    checkReply = NULL;

    // {"available":true,"ota":{"version":"2","type":"full","url":"https://readersupgrades.s3.amazonaws.com/s602m/elf_OTA_179caec13518.tar.gz?AWSAccessKeyId=AKIAJZCM2S2K54A3SCMA&Expires=1341403094&Signature=mXar8iCg8rkjD%2BovggE6R%2Fthv1A%3D","md5":"123123123123","dateVersion":"2012/06/25","downloadSize":"100Mb","releaseNotes":"New features:\r\n- Bla bla blah","signature":null}}
    // Parse reply
    bool parseOk;
    QVariantMap val = Json::parse(content, parseOk).toMap();

    if(!parseOk) {
        emit checkOTAAvailableFinished(QFwUpgradeOTA::Error);
        return;
    }

    if (val.contains("error")) {
        qDebug() << Q_FUNC_INFO << "Error calling checkUpdate: " << val["error"].toString();
        emit checkOTAAvailableFinished(QFwUpgradeOTA::Error);
        return;
    }

    updateInfo = new UpdateInfo();
    updateInfo->available = val["available"].toBool();
    if (updateInfo->available == false) {
        emit checkOTAAvailableFinished(QFwUpgradeOTA::NotAvailable);
        return;
    }

    QVariantMap ota = val["ota"].toMap();
    updateInfo->version = ota["version"].toString();
    QString type = ota["type"].toString();

    if (type == "partial")
        updateInfo->type = UpdateInfo::UpdatePartial;
    else if (type == "full")
        updateInfo->type = UpdateInfo::UpdateFull;
    else
        updateInfo->type = UpdateInfo::UpdateUnknown;

    updateInfo->url = ota["url"].toString();
    updateInfo->md5 = ota["md5"].toString();
    updateInfo->dateVersion = QDate::fromString(ota["dateVersion"].toString(), "yyyy/mm/dd");
    updateInfo->downloadSize = ota["downloadSize"].toLongLong();
    updateInfo->releaseNotes = ota["releaseNotesI18N"].toMap();
    updateInfo->signature = ota["signature"].toString();

    emit checkOTAAvailableFinished(QFwUpgradeOTA::Available);
}

QString QFwUpgradeOTA::getOTAVersionString() {
    if (!updateInfo && !updateInfo->available)
	return "";

    return updateInfo->version;
}

QVariantMap QFwUpgradeOTA::getOTAReleaseNotes() {
    if (!updateInfo && !updateInfo->available)
        return QVariantMap();

    return updateInfo->releaseNotes;
}



bool QFwUpgradeOTA::enoughMemForUpdate(){
    if (!updateInfo || !updateInfo->available)
	return false;

    unsigned long long freeSpace = Storage::getInstance()->getPrivatePartition()->getFreeSpace();
    qDebug() << Q_FUNC_INFO << "freeSpace: " << freeSpace << "OTA size: " << updateInfo->downloadSize;

    if (updateInfo->downloadSize > 0 && freeSpace > updateInfo->downloadSize) {
	qDebug() << Q_FUNC_INFO << "There is enough space for OTA";
	return true;
    }

    qDebug() << Q_FUNC_INFO << "There is not enough space for OTA";
    return false;
}

void QFwUpgradeOTA::requestCancel() {
    qDebug() << Q_FUNC_INFO;
    cancelRequested = true;
}

/// Url from server download the new firmware version.
bool QFwUpgradeOTA::download()
{
    qDebug() << Q_FUNC_INFO;
    if (!updateInfo || !updateInfo->available || updateInfo->url.isEmpty())
	return false;

    lock->activate();
    qDebug() << Q_FUNC_INFO << " starting download";

    cancelRequested = false;

    qDebug() << Q_FUNC_INFO << "FW Download URL = " << updateInfo->url;
    qDebug() << Q_FUNC_INFO << "FW Downloading to = " << downloadFile;

    if (QFile::exists(downloadFile)) {
        qDebug() << Q_FUNC_INFO << "Removing previous file";
        QFile::remove(downloadFile);
    }

    QUrl url = QUrl::fromEncoded(updateInfo->url.toLatin1());
    QNetworkRequest request(url);
    QNetworkReply *reply = m_qnam->get(request);

    // Assign buffer size
    reply->setReadBufferSize(DOWNLOAD_BUFFER_SIZE);
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), reply, SLOT(ignoreSslErrors()));

    QFile file(downloadFile);

    if (!file.open(QIODevice::WriteOnly))
    {
        delete reply;
    	lock->release();
        return false;
    }

    int oldAv = 0;
    int totalBytes = 0;
    QElapsedTimer timer;
    timer.start();
    while (!reply->isFinished())
    {
        int av = reply->bytesAvailable();
        if (cancelRequested) {
            qDebug() << Q_FUNC_INFO << "Canceling download per user request and removing file";
            file.close();
            file.remove();
            delete reply;
    	    lock->release();
            return false;
        }
	
        if (av >= DOWNLOAD_BUFFER_SIZE) {
            qDebug() << "OTA date available, saving it" << av;
            QByteArray data = reply->read(DOWNLOAD_BUFFER_SIZE);
            totalBytes += file.write(data);
            qDebug() << "acumulated" << totalBytes << " total: " << updateInfo->downloadSize;
            qDebug() << "Progress: " << int(100 * ( float(totalBytes)/float(updateInfo->downloadSize) ));
            emit OTAProgress(int(100 * ( float(totalBytes)/float(updateInfo->downloadSize) )));
            timer.restart();
        }
        QApplication::processEvents();

        /* Network error or no new bytes after DOWNLOAD_TIMEOUT */
        if (reply->error() || (timer.elapsed() > DOWNLOAD_TIMEOUT && oldAv == av)) {
            qDebug() << "Error downloading file, aborting: " << downloadFile;
            file.close();
            file.remove();
            delete reply;
    	    lock->release();
            return false;
        }
        oldAv = av;
    }

    /* Write all remaining data */
    file.write(reply->readAll());
    file.close();
    delete reply;

    lock->release();
    downloaded = true;
    return true;
}

bool QFwUpgradeOTA::checkFileValidity()
{
    if (!downloaded)
	return false;

    qDebug() << Q_FUNC_INFO << "File name:" << downloadFile;

    if (updateInfo->md5.isEmpty()) {
	qDebug() << Q_FUNC_INFO << "No hash received in checkUpdate, then file is not ok";
	qDebug() << Q_FUNC_INFO << "removing file: " << downloadFile;
	QFile::remove(downloadFile);
        return false;
    }

    if (!checkMd5()) {
	qDebug() << Q_FUNC_INFO << "md5 check failed, then file is not ok";
	qDebug() << Q_FUNC_INFO << "removing file: " << downloadFile;
	QFile::remove(downloadFile);
        return false;
    }

    if (updateInfo->signature.isEmpty()) {
	qDebug() << Q_FUNC_INFO << "No signature received in checkUpdate, then file is not ok";
	qDebug() << Q_FUNC_INFO << "removing file: " << downloadFile;
	QFile::remove(downloadFile);
        return false;
    }

    if (!checkSignature()) {
	qDebug() << Q_FUNC_INFO << "signature check failed, then file is not ok";
	qDebug() << Q_FUNC_INFO << "removing file: " << downloadFile;
	QFile::remove(downloadFile);
        return false;
    }

    return true;
}

bool QFwUpgradeOTA::checkMd5()
{
    QCryptographicHash crypto(QCryptographicHash::Md5);

    QFile file(downloadFile);
    if (!file.open(QFile::ReadOnly)) {
	qDebug() << Q_FUNC_INFO << "Cannot open file";
	return false;
    }
	   
    lock->activate();
    while(!file.atEnd()){
    	crypto.addData(file.read(8192));
	QApplication::processEvents();
    }

    QByteArray hash = crypto.result();
    lock->release();
    if (hash.toHex() != updateInfo->md5) {
	qDebug() << Q_FUNC_INFO << "File hash: " << hash.toHex() << " expeceted hash: " << updateInfo->md5;
	return false;
    }

    return true;
}

bool QFwUpgradeOTA::checkSignature()
{
    QTemporaryFile signatureFile;
    if (!signatureFile.open()) { 
        qDebug() << Q_FUNC_INFO << "Cannot create signature file " << signatureFile.fileName() << " for gpg checking. Aborting";
	return false;
    }

    lock->activate();
    QTextStream out(&signatureFile);
    out << updateInfo->signature;
    signatureFile.close();

    QProcess gpg;
    QStringList arguments;
    arguments << "--homedir="KEYRING_PATH;
    arguments << "--ignore-time-conflict";
    arguments << "--verify";
    arguments << signatureFile.fileName();
    arguments << downloadFile;

    gpg.start("/usr/bin/gpg", arguments);
    gpg.waitForFinished(60000);
    if (gpg.exitStatus() != QProcess::NormalExit || gpg.exitCode() != 0) {
        qDebug() << Q_FUNC_INFO << "gpg exited with error. Check failed";
        qDebug() << gpg.readAllStandardOutput();
        qDebug() << gpg.readAllStandardError();
        return false;
    }
    
    return true;
}

bool QFwUpgradeOTA::doUpdate()
{
    qDebug() << Q_FUNC_INFO;
    if (!QFile::exists(downloadFile))
    {
        qDebug() << Q_FUNC_INFO << "There is no downloaded file for update: " << downloadFile;
        return false;
    }
   
    if (QFile::exists(OTAFile))
    {
        qDebug() << Q_FUNC_INFO << "Removing previous OTA file: " << OTAFile;
        if (!QFile::remove(OTAFile))
        {
            qDebug() << Q_FUNC_INFO << "Cannot remove OTA file from " << OTAFile;
            return false;
        }
    }

    if (!QFile::rename(downloadFile, OTAFile))
    {
        qDebug() << Q_FUNC_INFO << "Cannot rename downloaded file from " << downloadFile << " to " << OTAFile;
        return false;
    }

    if (QFile::exists(SYSTEM_OTA_PATH))
    {
        qDebug() << Q_FUNC_INFO << "Removing previous OTA symlink: " << SYSTEM_OTA_PATH;
        if (!QFile::remove(SYSTEM_OTA_PATH))
        {
            qDebug() << Q_FUNC_INFO << "Cannot remove OTA symlink from " << SYSTEM_OTA_PATH;
            return false;
        }
    }

    if (!QFile::link(OTAFile, SYSTEM_OTA_PATH))
    {
        qDebug() << Q_FUNC_INFO << "Cannot create symlink for system OTA installing from " << OTAFile << " to " << SYSTEM_OTA_PATH;
        return false;
    }

    return true;
}
