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


#include "version.h"

#include "BrowserWebPage.h"
#include <QtNetwork/QNetworkRequest>
#include <QtCore/QSettings>
#include "QBook.h"
#include <QDebug>
#include "DeviceInfo.h"

bool
BrowserWebPage::extension(Extension extension, const ExtensionOption* option, ExtensionReturn* output)
{
    qDebug() << Q_FUNC_INFO;
    return QWebPage::extension(extension, option , output);
}

bool
BrowserWebPage::acceptNavigationRequest(QWebFrame * frame, const QNetworkRequest& request, NavigationType type) 
{
    qDebug() << Q_FUNC_INFO;
    QNetworkRequest newRequest = request;
   QUrl url = newRequest.url();
   if(url.toEncoded().right(4) == ".txt"){
        emit tryDownload(url);
        qDebug() << Q_FUNC_INFO << "false";
        return false;
   }
   qDebug() << Q_FUNC_INFO << "true";
   return QWebPage::acceptNavigationRequest(frame, request , type);
}


QString BrowserWebPage::userAgentForUrl ( const QUrl &) const
{
    static bool gotLang = false;
    static QString lang;
    if(!gotLang) {
        lang = QBook::settings().value("setting/language", QVariant("es")).toString().toLower();
        gotLang = true;
    }

    if(!inStore){
        return "Mozilla/5.0(Linux;U;Android 2.1-"+QString(QBOOKAPP_VERSION)+";"+lang+"-"+lang+";) "
               "AppleWebKit/530.17(KHTML,like Gecko) Version/4.0 Mobile Safari/530.17 ";
    }

    static QString serialNumber = NULL;
    if(serialNumber == NULL)
        serialNumber = DeviceInfo::getInstance()->getSerialNumber();

    if(!email.size())
        email = QBook::settings().value("eMail", "").toString();

    QString token = "no_token";

    qDebug() << Q_FUNC_INFO << " email: " << email << " serialNumber: " << serialNumber;

    QString value =  "Mozilla/5.0(Linux;U;Android 2.1-"+QString(QBOOKAPP_VERSION)+";"+lang+"-"+lang+";"+ email+";"+ serialNumber +";" + token +";) "
                     "AppleWebKit/530.17(KHTML,like Gecko) Version/4.0 Mobile Safari/530.17 ";
    return value;

}
