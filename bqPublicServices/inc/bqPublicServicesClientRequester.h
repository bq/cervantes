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

#ifndef BQPUBLICSERVICESCLIENTREQUESTER_H
#define BQPUBLICSERVICESCLIENTREQUESTER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QTimer>
#include <QList>
#include <QSslError>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QMap>

// Predeclaration
class QNetworkReply;
class QNetworkRequest;

class bqPublicServicesClientRequester : public QObject
{
   Q_OBJECT

public:
    bqPublicServicesClientRequester();
    virtual ~bqPublicServicesClientRequester();

    // Device requests
    QNetworkReply*                          getWikipediaInfo                ( QString lang, QString wordToSearch, int section );
    QNetworkReply*                          getWikipediaHeaders             ( QString lang, QString wordToSearch );
    QNetworkReply*                          getWikipediaSearch              ( QString lang, QString wordToSearch );

    // Generic requests
    QNetworkReply*                          getRequest                      ( const QNetworkRequest& request );
    QNetworkReply*                          postRequest                     ( QNetworkRequest& request, const QString & key, const QByteArray& value);
    QNetworkReply*                          doRequest                       ( const QNetworkRequest& request );
    QNetworkReply*                          doPostRequest                   ( QNetworkRequest& request, const QString & key, const QByteArray& value);

    bool                                    b_isDownloadCanceled;
    bool                                    b_failed;

public slots:
    void                                    finished                        ();
    void                                    timeOut                         ();
    void                                    checkCancel                     ();
    void                                    sslErrors                       ( QList<QSslError> );
    void                                    checkDownload                   ( qint64, qint64 );
    void                                    networkError                    ( QNetworkReply::NetworkError );

    void                                    authenticationRequired          ( QNetworkReply * reply, QAuthenticator * authenticator );
    void                                    networkAccessibleChanged        ( QNetworkAccessManager::NetworkAccessibility accessible );
    void                                    proxyAuthenticationRequired     ( const QNetworkProxy & proxy, QAuthenticator * authenticator );

signals:
    void syncCanceled();

private:
    QTimer                                  socketsTimer;
    QTimer                                  cancelationTimer;
    QNetworkAccessManager                   m_manager;
};

#endif // BQPUBLICSERVICESCLIENTREQUESTER_H
