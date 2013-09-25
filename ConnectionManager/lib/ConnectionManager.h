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

#ifndef __CONNECTIONMANAGER__
#define __CONNECTIONMANAGER__

#include <QObject>

#include "Manager.h"
#include "Service.h"
#include "Network.h"
#include "NetworkService.h"
#include "Clock.h"
#include "WpaInterface.h"
#include "Supplicant.h"

#define HELPER_EXIT_OK 0
#define HELPER_EXIT_TERMINATE 1
#define HELPER_EXIT_KILL 2
#define HELPER_EXIT_ERROR_REMOVED -1
#define HELPER_EXIT_ERROR_GENERIC -2

class PowerManagerLock;

class ConnectionManager : public QObject
{
    Q_OBJECT


public:
    typedef enum WifiTechnologyStatus {
        WIFI_TECHNOLOGY_STATUS_ENABLED,
        WIFI_TECHNOLOGY_STATUS_DISSABLED,
        WIFI_TECHNOLOGY_STATUS_MISSING,
    } WifiTechnologyStatus;

    void setOnline(bool waitUntilOnlineMode = true);
    void setOffline();
    bool isOffline();
    bool isConnected();
    QString getState();
    void scan();
    bool scanForESSID(QString essid);
    QList<Network> getNetworks();
    Network getNetwork(QString);
    QString getConnectingNetwork();
    NetworkService *getNetworkService(QString);
    void connectTo(QString);
    void disconnectFrom(QString);
    void removeAll();
    void remove(QString);
    void registerAgent(QDBusObjectPath);
    static ConnectionManager* getInstance();
    static void removeInstance();
    void powerWifi(bool, bool retry = true);
    void setNetworkManualConfig(QString path, QString ip, QString mask, QString gw, QStringList nameservers);
    void setNetworkDHCPConfig(QString path);

signals:
    void connected();
    void disconnected();
    void currentNetworkLevelChanged(int);
    void servicesChanged();
    void someThingHasChanged();
    void scanFinished();
    void timeUpdated();
    void onLineWifi(bool);
    void wifiProblem();

private slots:
    void propertyChanged(const QString&, const QDBusVariant&);
    void servicesChanged(TechnologiesList, const QList<QDBusObjectPath>&);
    void handleSyncTimeFinished();

private:
    ConnectionManager(QObject *parent = 0);
    bool init(bool);

    void setTimeConfiguration();
    void syncTime();
    QString getConnectedNetwork();
    WpaInterface *getWpaInterface();

    static ConnectionManager *instance;
    Manager *nm;
    QString state;
    NetworkService *connectedService;
    QFutureWatcher<int> timeSyncwatcher;
};

#endif
