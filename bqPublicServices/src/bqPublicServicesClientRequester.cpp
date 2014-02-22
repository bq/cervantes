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

#include "bqPublicServicesClientRequester.h"
#include "bqPublicServicesClient.h"

#include <QUrl>
#include <QSsl>
#include <QSslConfiguration>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>
#include <QDateTime>

bqPublicServicesClientRequester::bqPublicServicesClientRequester() :
    b_isDownloadCanceled(false),
    socketsTimer(NULL),
    cancelationTimer(NULL),
    m_manager(this)
{
    qDebug() << Q_FUNC_INFO;
    connect(&m_manager, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)), this, SLOT(authenticationRequired(QNetworkReply*, QAuthenticator*)));
    connect(&m_manager, SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), this, SLOT(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)));
    connect(&m_manager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)), this, SLOT(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));
}

bqPublicServicesClientRequester::~bqPublicServicesClientRequester(){
    qDebug() << Q_FUNC_INFO;
}

QNetworkReply* bqPublicServicesClientRequester::doRequest( const QNetworkRequest& request )
{
    qDebug() << "--->" << Q_FUNC_INFO;
    b_failed = false;
    QNetworkReply* reply = getRequest(request);

    // ------------ Asynchronous -------------------

    QEventLoop waitingLoop;

    reply->ignoreSslErrors();

    connect(reply,                  SIGNAL(downloadProgress(qint64,qint64)),    this,               SLOT(checkDownload(qint64,qint64)));
    connect(reply,                  SIGNAL(error(QNetworkReply::NetworkError)), &waitingLoop,       SLOT(quit()));
    connect(reply,                  SIGNAL(error(QNetworkReply::NetworkError)), this,               SLOT(networkError(QNetworkReply::NetworkError)));
    connect(reply,                  SIGNAL(finished()),                         &waitingLoop,       SLOT(quit()));
    connect(reply,                  SIGNAL(finished()),                         this,               SLOT(finished()));
    connect(reply,                  SIGNAL(sslErrors(QList<QSslError>)),        this,               SLOT(sslErrors(QList<QSslError>)));
    connect(reply,                  SIGNAL(finished()),                         &socketsTimer,      SLOT(stop()));
    connect(reply,                  SIGNAL(finished()),                         &cancelationTimer,  SLOT(stop()));
    connect(reply,                  SIGNAL(error(QNetworkReply::NetworkError)), &socketsTimer,      SLOT(stop()));
    connect(reply,                  SIGNAL(error(QNetworkReply::NetworkError)), &cancelationTimer,  SLOT(stop()));

    connect(&socketsTimer,          SIGNAL(timeout()),                          &waitingLoop,       SLOT(quit()));
    connect(&socketsTimer,          SIGNAL(timeout()),                          &socketsTimer,      SLOT(stop()));
    connect(&socketsTimer,          SIGNAL(timeout()),                          &cancelationTimer,  SLOT(stop()));
    connect(&socketsTimer,          SIGNAL(timeout()),                          this,               SLOT(timeOut()));
    connect(&cancelationTimer,      SIGNAL(timeout()),                          this,               SLOT(checkCancel()));
    connect(this,                   SIGNAL(syncCanceled()),                     &waitingLoop,       SLOT(quit()));
    connect(this,                   SIGNAL(syncCanceled()),                     &socketsTimer,      SLOT(stop()));

    int seconds = 15;
    socketsTimer.start(1000 * seconds);
    cancelationTimer.start(300);

    waitingLoop.exec();

    return reply;
}

QNetworkReply* bqPublicServicesClientRequester::doPostRequest( QNetworkRequest& request, const QString & key, const QByteArray& value )
{
    qDebug() << "--->" << Q_FUNC_INFO;
    b_failed =false;
    QNetworkReply *reply = postRequest(request, key, value);

    // ------------ Asynchronous -------------------

    QEventLoop waitingLoop;

    connect(reply,              SIGNAL(downloadProgress(qint64,qint64)),    this,               SLOT(checkDownload(qint64,qint64)));
    connect(reply,              SIGNAL(error(QNetworkReply::NetworkError)), &waitingLoop,       SLOT(quit()));
    connect(reply,              SIGNAL(error(QNetworkReply::NetworkError)), this,               SLOT(networkError(QNetworkReply::NetworkError)));
    connect(reply,              SIGNAL(finished()),                         &waitingLoop,       SLOT(quit()));
    connect(reply,              SIGNAL(finished()),                         this,               SLOT(finished()));
    connect(reply,              SIGNAL(sslErrors(QList<QSslError>)),        this,               SLOT(sslErrors(QList<QSslError>)));
    connect(reply,              SIGNAL(metaDataChanged()),                  this,               SLOT(metaDataChanged()));
    connect(reply,              SIGNAL(finished()),                         &socketsTimer,      SLOT(stop()));
    connect(reply,              SIGNAL(finished()),                         &cancelationTimer,  SLOT(stop()));
    connect(reply,              SIGNAL(error(QNetworkReply::NetworkError)), &socketsTimer,      SLOT(stop()));
    connect(reply,              SIGNAL(error(QNetworkReply::NetworkError)), &cancelationTimer,  SLOT(stop()));

    connect(&socketsTimer,      SIGNAL(timeout()),                          &waitingLoop,       SLOT(quit()));
    connect(&socketsTimer,      SIGNAL(timeout()),                          &socketsTimer,      SLOT(stop()));
    connect(&socketsTimer,      SIGNAL(timeout()),                          &cancelationTimer,  SLOT(stop()));
    connect(&socketsTimer,      SIGNAL(timeout()),                          this,               SLOT(timeOut()));
    connect(&cancelationTimer,  SIGNAL(timeout()),                          this,               SLOT(checkCancel()));
    connect(this,               SIGNAL(syncCanceled()),                     &waitingLoop,       SLOT(quit()));

    int seconds = 15;
    socketsTimer.start(1000 * seconds);
    cancelationTimer.start(100);
    waitingLoop.exec();

    return reply;
}

#include <QNetworkConfiguration>

QNetworkReply* bqPublicServicesClientRequester::getRequest( const QNetworkRequest& request )
{
    qDebug() << Q_FUNC_INFO;

    qDebug() << "URL: " << request.url();
    QList<QByteArray> headers = request.rawHeaderList();
    qDebug() << "Request Headers: " << headers;
    foreach(QByteArray a, headers) {
        qDebug() << "Header: " << request.rawHeader(a);
    }

    QNetworkReply *reply = m_manager.get(request);
    qDebug() << "reply: " << reply;
    return reply;
}

QNetworkReply* bqPublicServicesClientRequester::postRequest( QNetworkRequest& request, const QString & key, const QByteArray& value)
{
    qDebug() << Q_FUNC_INFO;

    qDebug() << "URL: " << request.url();
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QUrl params;
    params.addQueryItem(key, value);

    QNetworkReply *reply = m_manager.post(request, params.encodedQuery());

    return reply;
}

void bqPublicServicesClientRequester::finished()
{
    qDebug() << Q_FUNC_INFO;
}

void bqPublicServicesClientRequester::timeOut()
{
    qDebug() << Q_FUNC_INFO;
    b_failed = true;
}

void bqPublicServicesClientRequester::checkCancel()
{
    if(socketsTimer.isActive()){
        if(b_isDownloadCanceled){
            socketsTimer.stop();
            cancelationTimer.stop();
            emit syncCanceled();
        }else{
            cancelationTimer.start(500);
        }
    }
}

void bqPublicServicesClientRequester::networkError( QNetworkReply::NetworkError error )
{
    qDebug() << Q_FUNC_INFO << "Error: " << error;
}

void bqPublicServicesClientRequester::sslErrors(QList<QSslError> errors)
{
    foreach(QSslError error, errors)
    {
        qDebug() << Q_FUNC_INFO << "Error in ssl comunication:" << error.errorString();
    }
}

void bqPublicServicesClientRequester::checkDownload(qint64 downloaded, qint64 total)
{
    qDebug() << Q_FUNC_INFO << "downloaded: " << downloaded  << " total: " << total;
    socketsTimer.start(1000 * 15);
}

void bqPublicServicesClientRequester::authenticationRequired( QNetworkReply *, QAuthenticator* )
{
    qDebug() << Q_FUNC_INFO;
}

void bqPublicServicesClientRequester::networkAccessibleChanged( QNetworkAccessManager::NetworkAccessibility )
{
    qDebug() << Q_FUNC_INFO;
}

void bqPublicServicesClientRequester::proxyAuthenticationRequired( const QNetworkProxy &, QAuthenticator* )
{
    qDebug() << Q_FUNC_INFO;
}

QNetworkReply* bqPublicServicesClientRequester::getWikipediaHeaders(QString lang, QString wordToSearch)
{
    qDebug() << Q_FUNC_INFO;
    QString host = "http://" + lang + ".wikipedia.org/w/api.php?";

    QString action = "action=mobileview&";
    QString images = "noimages&";
    QString format = "format=xml&";
    QString sections = "sections=0&";
    QString search = QString("page=%1").arg(wordToSearch);

    QString urlPath = QString("%1%2%3%4%5%6").arg(host, action, images, format, sections, search);
    qDebug() << Q_FUNC_INFO << "urlPath: " << urlPath;

    QNetworkRequest* request = new QNetworkRequest(urlPath);
    request->setHeader(QNetworkRequest::ContentTypeHeader, "text/xml");

    QNetworkReply *reply = doRequest(*request);

    delete request;
    return reply;
}

QNetworkReply* bqPublicServicesClientRequester::getWikipediaInfo(QString lang, QString wordToSearch, int section)
{
    qDebug() << Q_FUNC_INFO;

    QString host = "http://" + lang + ".wikipedia.org/w/api.php?";

    QString action = "action=mobileview&";
    QString images = "noimages&";
    QString format = "format=xml&";
    QString sections = QString("sections=%1&").arg(section);
    QString search = QString("page=%1").arg(wordToSearch);

    QString urlPath = QString("%1%2%3%4%5%6").arg(host, action, images, format, sections, search);
    qDebug() << "urlPath: " << urlPath;

    QNetworkRequest* request = new QNetworkRequest(urlPath);
    request->setHeader(QNetworkRequest::ContentTypeHeader, "text/xml");

    QNetworkReply *reply = doRequest(*request);

    delete request;
    return reply;
}

QNetworkReply* bqPublicServicesClientRequester::getWikipediaSearch(QString lang, QString wordToSearch)
{
    qDebug() << Q_FUNC_INFO;

    QString host = "http://" + lang + ".wikipedia.org/w/api.php?";

    QString action = "action=opensearch&";
    QString format = "format=xml&";
    QString search = QString("search=%1").arg(wordToSearch);

    QString urlPath = QString("%1%2%3%4").arg(host, action, format, search);
    qDebug() << "urlPath: " << urlPath;

    QNetworkRequest* request = new QNetworkRequest(urlPath);
    request->setHeader(QNetworkRequest::ContentTypeHeader, "text/xml");

    QNetworkReply *reply = doRequest(*request);

    delete request;
    return reply;
}
