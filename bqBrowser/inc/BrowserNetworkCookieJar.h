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

#ifndef QBOOKNETWORKCOOKIEJAR_H
#define QBOOKNETWORKCOOKIEJAR_H
#include <QNetworkCookieJar>
class BrowserNetworkCookieJar:public QNetworkCookieJar
{
	Q_OBJECT
public:
    BrowserNetworkCookieJar(QObject * parent = 0);
    ~BrowserNetworkCookieJar();

	QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const;
	bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url);

	QList<QNetworkCookie> cookies() const;
	void setCookies(const QList<QNetworkCookie> &cookies);

signals:
	void cookiesChanged();

public slots:
	void clear();
	void loadSettings();

private slots:
	void save();

private:
	void purgeOldCookies();
	void load();
	bool m_loaded;
	bool m_isPrivate;
};

#endif
