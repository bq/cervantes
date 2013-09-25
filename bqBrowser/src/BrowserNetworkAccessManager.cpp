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

#define _LARGEFILE64_SOURCE
#include <QNetworkProxy>
#include <QNetworkProxyFactory>

#include "BrowserNetworkAccessManager.h"
#include "BrowserNetworkCookieJar.h"
#include "BrowserNetworkProxyFactory.h"
#include "BrowserNetworkDiskCache.h"
#include "QBook.h"

#define DEFAULT_PROXY_PORT 1080

#include <fcntl.h>
#include <unistd.h>

#define BLKGETSIZE _IO(0x12,96)	/* return device size /512 (long *arg) */
const int envSize = 32;
const int bl1Size = 16;
const int reserve = 2;

BrowserNetworkAccessManager::BrowserNetworkAccessManager(QObject * parent): QNetworkAccessManager(parent)
{
    setCookieJar(new BrowserNetworkCookieJar);
    loadSettings();
}

BrowserNetworkAccessManager::~BrowserNetworkAccessManager()
{

}

void BrowserNetworkAccessManager::loadSettings()
{
    QNetworkProxy proxy;
    if (QBook::settings().value(QLatin1String("proxy/enabled"), false).toBool() == true) {
		qDebug ("++ BrowserNetworkAccessManager:proxy enabled...\n");

        int proxyType = QBook::settings().value(QLatin1String("proxy/type"), 0).toInt();
        if (proxyType == 0)
            proxy = QNetworkProxy::Socks5Proxy;
        else if (proxyType == 1)
            proxy = QNetworkProxy::HttpProxy;
        else { // 2
            proxy.setType(QNetworkProxy::HttpCachingProxy);
            proxy.setCapabilities(QNetworkProxy::CachingCapability | QNetworkProxy::HostNameLookupCapability);
        }

		qDebug ("++ [proxy] type = %d", proxyType);

        proxy.setHostName(QBook::settings().value(QLatin1String("proxy/hostName")).toString());
        proxy.setPort(QBook::settings().value(QLatin1String("proxy/port"), DEFAULT_PROXY_PORT).toInt());
        proxy.setUser(QBook::settings().value(QLatin1String("proxy/userName")).toString());
        proxy.setPassword(QBook::settings().value(QLatin1String("proxy/password")).toString());

		qDebug() << "++ [proxy] hostname = " << proxy.hostName();
		qDebug ("++ [proxy] port = %d", proxy.port());
		qDebug() << "++ [proxy] user = " << proxy.user();
		qDebug() << "++ [proxy] password = " << proxy.password();
    }
    BrowserNetworkProxyFactory *proxyFactory = new BrowserNetworkProxyFactory;
	proxyFactory->setProxy(proxy);
    setProxyFactory(proxyFactory);

	bool cacheEnabled = QBook::settings().value(QLatin1String("webkit/cacheEnabled"), true).toBool();

	if (cacheEnabled) {
		BrowserNetworkDiskCache *diskCache;
		if (cache())
			diskCache = qobject_cast<BrowserNetworkDiskCache*>(cache());
		else
			diskCache = new BrowserNetworkDiskCache(this);
		setCache(diskCache);
		diskCache->loadSettings();
	} else {
		setCache(0);
	}
}

QNetworkReply * BrowserNetworkAccessManager::createRequest ( Operation op, const QNetworkRequest & req, QIODevice * outgoingData )
{
    return QNetworkAccessManager::createRequest(op,req,outgoingData);
}
bool BrowserNetworkAccessManager::isDnURL(const QUrl & dnUrl)
{
    qDebug( "isDnURL" );
    QString ipAddrRegExp("([0-9]{1,3}[.][0-9]{1,3}[.][0-9]{1,3}[.][0-9]{1,3})");
    QRegExp  ip(ipAddrRegExp );
    QString sURL = dnUrl.toString();
    int nIPos = ip.indexIn(sURL) ;
    if( nIPos == -1){
        return true;
    }
    return false;

}
void BrowserNetworkAccessManager::dnURLToIPURL(const QUrl & dnUrl,QUrl & ipUrl)
{
    qDebug( "dnURLToIPURL,ShowHost" );
    QString sHost = dnUrl.host();   
    //QString sOriginalUrl = dnUrl.toString();
    QString sOriginalUrl = dnUrl.toEncoded(); 

    int nHostHeadPos = sOriginalUrl.indexOf(sHost);
    int nHostTailPos = nHostHeadPos + sHost.length();
    QString sNewURL = sOriginalUrl.mid(0,nHostHeadPos) + dnToIP(sHost);
    QString sURLTail = sOriginalUrl.right(sOriginalUrl.length() - nHostTailPos);
    sNewURL += sURLTail;
    //ipUrl = QUrl(sNewURL);
    ipUrl = QUrl::fromEncoded(sNewURL.toAscii());
    ipUrl = ipUrl;
}

QString BrowserNetworkAccessManager::dnToIP(const QString & sDomainName)
{
    qDebug( "dnToIP" );
    QString sIp = sDomainName;
    QString showIP = "showDomainName" ;
    qDebug() << showIP;
    qDebug() << sDomainName;

    QString sCommand = "/usr/bin/nslookup " + sDomainName;
    qDebug("/usr/bin/nslookup ");
    qDebug() << sDomainName;

     qDebug() <<  "DO:dnToIP" ;
    QProcess nslookup;
    nslookup.start( sCommand );
    QString ipAddrRegExp("([0-9]{1,3}[.][0-9]{1,3}[.][0-9]{1,3}[.][0-9]{1,3})");
    if(!nslookup.waitForStarted() ) {
        qDebug() <<  " error starting 'nslookup'" ;
       return sIp;
    } 
    else
    {
        if(!nslookup.waitForFinished()) {
             qDebug ( "nslookup exited with error" );
            return sIp;
        } else {
            qDebug( "else" );
            QString array = nslookup.readAll();
            qDebug("ShowAll");
			qDebug() << array;
            QString sName = "Name:";
            int nNameTailPos = array.indexOf(sName) ;
            if( nNameTailPos == -1){
                qDebug( "No Name:" );
                return sIp;
            }
            nNameTailPos += sName.length();
            QRegExp  ip(ipAddrRegExp );
            int ipPos = ip.indexIn( array ,nNameTailPos);
            if (ipPos == -1){
                qDebug( "No IP:" );
                return sIp;
            }
            sIp = array.mid(ipPos,ip.matchedLength());
             qDebug( "Get IP SUccess!!" );
             qDebug() <<  sIp ;
            return sIp;
        }
    }
    
    return sIp;
}
