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

#ifndef __MYAGENT_H__
#define __MYAGENT_H__

#include <QtCore/QObject>
#include <QtDBus/QtDBus>

#include "PasswordRequester.h"

class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;

/*
 * Adaptor class for interface net.connman.Agent
 */
class MyAgent: public QObject
{
    Q_OBJECT
public:
    explicit MyAgent(QObject *parent = 0);
    void setPasswordRequester(PasswordRequester*);
;

public Q_SLOTS: // METHODS
    void Cancel();
    void Release();
    void ReportError(const QDBusObjectPath &path, const QString &error, const QDBusMessage &message);
    void RequestBrowser(const QDBusObjectPath &path, const QString &url);
    QVariantMap RequestInput(const QDBusObjectPath &path, const QVariantMap &fields, const QDBusMessage &message);
Q_SIGNALS: // SIGNALS
    void wrongPassword(QString);
    void connectionFailed(QString, QString);
    void released();
private:
    PasswordRequester *passwordRequester;
    int tries;
    bool b_wrongPassSignalSent;

};

#endif
