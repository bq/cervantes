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

#ifndef QBOOKWEBPAGE_H
#define QBOOKWEBPAGE_H
#include <QWebPage>
#include <QWebFrame>
class BrowserWebPage : public QWebPage
{
	Q_OBJECT
public:
    BrowserWebPage(QWidget* parent = 0, bool inStore = false):QWebPage(parent) { this->inStore = inStore; }
    ~BrowserWebPage(){}
public:
    virtual bool                extension                   ( Extension extension, const ExtensionOption * option , ExtensionReturn * output);
    void                        setEmail                    (const QString& _email){ email = _email;}

protected:
    virtual bool acceptNavigationRequest ( QWebFrame * frame, const QNetworkRequest & request, NavigationType type ) ;
    virtual QString  userAgentForUrl ( const QUrl & url ) const;

signals:
    void tryDownload(const QUrl & url);
private:
    bool                            inStore;
    mutable QString                 email;
};
#endif
