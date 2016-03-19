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

#include <QVariant>
#include <QSettings>
#include <QtDebug>
#include "Storage.h"
#include "QBook.h"
#include "BrowserNetworkCookieJar.h"

#define COOKIES_INI_FILE	"browser/cookies.ini"
#define TF_COOKIES_INI_FILE	"/rwdata/.store/cookies.ini"

static const unsigned int JAR_VERSION = 23;

QT_BEGIN_NAMESPACE
QDataStream &operator<<(QDataStream &stream, const QList<QNetworkCookie> &list)
{
	stream << JAR_VERSION;
	stream << quint32(list.size());
	for (int i = 0; i < list.size(); ++i)
		stream << list.at(i).toRawForm();
	return stream;
}

QDataStream &operator>>(QDataStream &stream, QList<QNetworkCookie> &list)
{
	list.clear();

	quint32 version;
	stream >> version;

	if (version != JAR_VERSION)
		return stream;

	quint32 count;
	stream >> count;
	for (quint32 i = 0; i < count; ++i) {
		QByteArray value;
		stream >> value;
		QList<QNetworkCookie> newCookies = QNetworkCookie::parseCookies(value);
		if (newCookies.count() == 0 && value.length() != 0) {
			qDebug() << "CookieJar: Unable to parse saved cookie:" << value;
		}
		for (int j = 0; j < newCookies.count(); ++j)
			list.append(newCookies.at(j));
		if (stream.atEnd())
			break;
	}
	return stream;
}
QT_END_NAMESPACE

BrowserNetworkCookieJar::BrowserNetworkCookieJar(QObject * parent)
:m_loaded(false),
m_isPrivate(false)
{}

BrowserNetworkCookieJar::~BrowserNetworkCookieJar()
{
	save();
}

void BrowserNetworkCookieJar::clear()
{
	if (!m_loaded)
		load();
	setAllCookies(QList<QNetworkCookie>());
//	m_saveTimer->changeOccurred();
	save();
	emit cookiesChanged();
}

void BrowserNetworkCookieJar::load()
{
 QString cookie_file="";

	if(QBook::settings().value("setting/TFStore", false).toBool()){
		qDebug()<<"has true value in TFStore";
		cookie_file = TF_COOKIES_INI_FILE;
	}else{
		qDebug()<<"has false value in TFStore";
                cookie_file = ( Storage::getInstance()->getDataPartition()->getMountPoint()+"/"+COOKIES_INI_FILE );
	}
	if (m_loaded)
		return;
	// load cookies and exceptions
	qRegisterMetaTypeStreamOperators<QList<QNetworkCookie> >("QList<QNetworkCookie>");
	QSettings cookieSettings(cookie_file, QSettings::IniFormat);
	qDebug()<<"Load cookie path="+cookie_file;
	if (!m_isPrivate) {
		setAllCookies(qvariant_cast<QList<QNetworkCookie> >(cookieSettings.value(QLatin1String("cookies"))));
	}

	loadSettings();
}

void BrowserNetworkCookieJar::loadSettings()
{
	m_loaded = true;
}

void BrowserNetworkCookieJar::save()
{
	if (!m_loaded || m_isPrivate)
		return;
	purgeOldCookies();

	QList<QNetworkCookie> cookies = allCookies();
	for (int i = cookies.count() - 1; i >= 0; --i) {
		if (cookies.at(i).isSessionCookie())
			cookies.removeAt(i);
	}

    if(QBook::settings().value("setting/webkit/cookiesJarToDisk", true).toBool())
	{
		qDebug () << "++ BrowserNetworkCookieJar:: cookiesjartodisk enabled...";

		QString cookie_file="";

		if(QBook::settings().value("setting/TFStore", false).toBool()){
			qDebug()<<"has true value in TFStore";
			cookie_file = TF_COOKIES_INI_FILE;
		}else{
			qDebug()<<"has false value in TFStore";
                        cookie_file = ( Storage::getInstance()->getDataPartition()->getMountPoint()+"/"+COOKIES_INI_FILE );
		}

		QSettings cookieSettings(cookie_file, QSettings::IniFormat);
		cookieSettings.setValue(QLatin1String("cookies"), qVariantFromValue<QList<QNetworkCookie> >(cookies));
		qDebug()<<"Save cookie path="+cookie_file;
	}
}

void BrowserNetworkCookieJar::purgeOldCookies()
{
	QList<QNetworkCookie> cookies = allCookies();
	if (cookies.isEmpty())
		return;
	int oldCount = cookies.count();
	QDateTime now = QDateTime::currentDateTime();
	for (int i = cookies.count() - 1; i >= 0; --i) {
		if (!cookies.at(i).isSessionCookie() && cookies.at(i).expirationDate() < now)
			cookies.removeAt(i);
	}
	if (oldCount == cookies.count())
		return;
	setAllCookies(cookies);
	emit cookiesChanged();
}

QList<QNetworkCookie> BrowserNetworkCookieJar::cookiesForUrl(const QUrl &url) const
{
	BrowserNetworkCookieJar *that = const_cast<BrowserNetworkCookieJar*>(this);
	if (!m_loaded)
		that->load();

	return QNetworkCookieJar::cookiesForUrl(url);
}

bool BrowserNetworkCookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
	if (!m_loaded)
		load();

	return QNetworkCookieJar::setCookiesFromUrl(cookieList, url);
}

QList<QNetworkCookie> BrowserNetworkCookieJar::cookies() const
{
	BrowserNetworkCookieJar *that = const_cast<BrowserNetworkCookieJar*>(this);
	if (!m_loaded)
		that->load();

	return allCookies();
}

void BrowserNetworkCookieJar::setCookies(const QList<QNetworkCookie> &cookies)
{
	if (!m_loaded)
		load();
	setAllCookies(cookies);
	//m_saveTimer->changeOccurred();
	save();
	emit cookiesChanged();
}
