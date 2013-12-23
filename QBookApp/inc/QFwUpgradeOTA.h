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

/*  QFwUpgradeOTA.h

    This class defines a thread that download firmware file
    from the server in parallel with normal execution. When
    done parent process is notified and user is asked about
    firmware update.

    This class inherits from QThread
*/
#ifndef __QFWUPGRADEOTA_H__
#define __QFWUPGRADEOTA_H__

#include <QString>
#include <qglobal.h>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QDate>
#include "PowerManagerLock.h"


#define OTA_FILENAME "OTA.gpg"
#define DOWNLOAD_FILENAME "OTA-tmp.gpg"
#define SYSTEM_OTA_PATH "/OTA.gpg"

class UpdateInfo
{
    public:
        enum UpdateType { UpdateUnknown = -1, UpdatePartial = 0 , UpdateFull = 1 };

        bool            available;
        QString         version;
        UpdateType      type;
        QString         url;
        QString         md5;
        QDate           dateVersion;
        long long       downloadSize;
        QVariantMap     releaseNotes;
        QString         signature;
};

class QFwUpgradeOTA: public QObject
{
    Q_OBJECT

public:

    enum CheckResult { Error = -1, NotAvailable = 0 , Available = 1 };

    QFwUpgradeOTA();
    virtual ~QFwUpgradeOTA();

    bool checkOTAAvailable();
    bool enoughMemForUpdate();
    bool download();
    bool checkFileValidity();
    bool doUpdate();
    bool getCancelRequested(){ return cancelRequested;}
    QString getOTAVersionString();
    QVariantMap getOTAReleaseNotes();
    bool checkUpdateInfo();

signals:
    void OTAProgress(int);
    void checkOTAAvailableFinished(int);

public slots:
    void requestCancel();

private slots:
    void checkReplyError(QNetworkReply::NetworkError);
    void checkReplyFinished();

private:
    bool checkMd5();
    bool checkSignature();
    PowerManagerLock *lock;
    QNetworkAccessManager *m_qnam;
    bool downloaded;
    QNetworkReply *checkReply;
    UpdateInfo *updateInfo;
    QString downloadFile;
    QString OTAFile;
    bool cancelRequested;
};

#endif // __QFWUPGRADEOTA_H__
