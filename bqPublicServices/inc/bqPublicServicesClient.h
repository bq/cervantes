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

#ifndef BQPUBLICSERVICESCLIENT_H_
#define BQPUBLICSERVICESCLIENT_H_

#include "bqPublicServicesClientRequester.h"

#include <QString>
#include <QObject>
#include <QStringList>
#include <QDate>
#include <QFile>
#include <QElapsedTimer>

class bqPublicServicesClient : public QObject
{
       Q_OBJECT

public:
    bqPublicServicesClient();
    virtual ~bqPublicServicesClient();

    void                                    setDownloadCanceled             (bool _downloadCanceled);

    QString                                 getWikipediaInfo                ( QString, QString, int section );
    QString                                 getWikipediaHeaders             ( QString, QString, int& );
    QString                                 getWikipediaSearch              ( QString, QString );

public slots:
    void                                    processSyncCanceled             ();

signals:
    void                                    downloadProgress                ( int );
    void                                    hideProgressBar                 ( );

private:
    bqPublicServicesClientRequester         m_requester;
    int                                     getNetworkReplyResponse         ( QNetworkReply* reply );
    bool                                    downloadCanceled;

};

#endif // BQPUBLICSERVICESCLIENT_H_
