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

#include <time.h>
#include <sys/time.h>
#include <unistd.h>


#include <QApplication>
#include <QVariantMap>
#include <QWidget>
#include <QDebug>
#include <QFutureWatcher>

#include "ConnectionManager.h"
#include "Technology.h"
#include "Service.h"
#include "TechnologiesList.h"
#include "NetworkService.h"

#include "HiddenQuery.h"

ConnectionManager* ConnectionManager::instance = NULL;

ConnectionManager* ConnectionManager::getInstance(void) {
    qDebug() << "--->" << Q_FUNC_INFO;
	if (instance == NULL) {
		instance = new ConnectionManager(QCoreApplication::instance());
		if (!instance) {
			qDebug() << "Cannot create ConnectionManager";
			return NULL;
		}
		if (!instance->init(false)) {
			qDebug() << "Cannot initialize ConnectionManager";
			delete instance;
			return NULL;
		}
	}
	return instance;
}

void ConnectionManager::removeInstance()
{
    qDebug() << "--->" << Q_FUNC_INFO;
    if(instance)
    {
        delete instance;
        instance = NULL;
    }
}

ConnectionManager::ConnectionManager(QObject *parent) : QObject(parent) {
	qDebug() << "--->" << Q_FUNC_INFO;
	TechnologiesListTypesInit();
	connectedService = NULL;
	connect(&timeSyncwatcher, SIGNAL(finished()), this, SLOT(handleSyncTimeFinished()));
}

bool ConnectionManager::init(bool online) {
    qDebug() << "--->" << Q_FUNC_INFO;
	if (!QDBusConnection::systemBus().isConnected()) {
		qDebug() << "system bus is not connected";
		return false;
	}

	nm = new Manager("net.connman", "/",
                          QDBusConnection::systemBus(),
                          this);
	if (!nm) {
		qDebug() << "Cannot create the connman Manager dbus proxy";
		qDebug() << QDBusConnection::systemBus().lastError().message();
		return false;
	}
	// FIXME: FER Add notify if we got dissconected (connmand diying?)

	QCoreApplication::processEvents();
	connect(nm, SIGNAL(PropertyChanged(const QString&, const QDBusVariant&)),
                this, SLOT(propertyChanged(const QString&, const QDBusVariant&)));
        connect(nm, SIGNAL(ServicesChanged(TechnologiesList, const QList<QDBusObjectPath>&)),
        	this, SLOT(servicesChanged(TechnologiesList, const QList<QDBusObjectPath>&)));

	setTimeConfiguration();

	QDBusReply<QVariantMap> reply = nm->GetProperties();
	QVariantMap properties = reply;
	state = properties["State"].toString();

	if (online)
		setOnline();

	return true;
}

void ConnectionManager::setTimeConfiguration(void) {
    qDebug() << "--->" << Q_FUNC_INFO;
	Clock *clock = new Clock("net.connman", "/",
                          QDBusConnection::systemBus(),
                          this);
	if (!clock) {
		qDebug() << "Cannot create the connman Clock dbus proxy";
		qDebug() << QDBusConnection::systemBus().lastError().message();
		return;
	}

	clock->SetProperty("TimezoneUpdates", QDBusVariant("manual"));
	clock->SetProperty("TimeUpdates", QDBusVariant("manual"));
	clock->SetProperty("Timeservers", QDBusVariant(QStringList("")));

	delete clock;
}

void ConnectionManager::powerWifi(bool on, bool retry) {
    QDBusReply<TechnologiesList> reply = nm->GetTechnologies();
    TechnologiesList technologies = reply;
    foreach(TechnologyStruct t, technologies) {
        if (t.properties["Name"] == "WiFi") {
            qDebug() << "Enabling wifi on path: " << t.path.path();
            Technology *tech = new Technology("net.connman", t.path.path(), QDBusConnection::systemBus(), this);
            if (!tech) {
                qDebug() << "Cannot get technology object for path " << t.path.path();
                return;
            }
            tech->SetProperty("Powered", QDBusVariant(on));
            QVariantMap map;
            QElapsedTimer timer;
            timer.start();
            do{
                QDBusReply<QVariantMap> reply = tech->GetProperties();
                map = reply;
                qDebug() << map;

            }while (map["Powered"].toBool() != on && timer.elapsed() < 5000);
            if(timer.elapsed() >= 5000)
            {
                QString cmd = "killall connmand";
                int ret = system(cmd.toAscii().constData());

                qDebug() << Q_FUNC_INFO << "Killing command. Ret: " << ret;
                sleep(2);
                if(retry){
                    qDebug() << Q_FUNC_INFO << "Retrying....";
                    powerWifi(on, false);
                }else{
                    emit wifiProblem();
                    qDebug() << Q_FUNC_INFO << "no retrying. WARNING We have killed command previously and it was not fixed";
                }
            }
        break;
        }
    }
}


void ConnectionManager::setOnline(bool waitUntilOnlineMode) {
    qDebug() << "--->" << Q_FUNC_INFO;

    /** Signal to activate lock waiting to connman autoconnect **/
    emit onLineWifi(true);

	QDBusReply<QVariantMap> reply = nm->GetProperties();
	QVariantMap properties = reply;

	if (properties["OfflineMode"].toBool() == true) {
		nm->SetProperty("OfflineMode", QDBusVariant(false));

        if(!waitUntilOnlineMode)
            return;

        // Wait loop
		QElapsedTimer timer;
		timer.start();
		QVariantMap properties = reply;
		do {
			QDBusReply<QVariantMap> reply = nm->GetProperties();
            QVariantMap properties = reply;
			QCoreApplication::processEvents();
		} while (properties["OfflineMode"].toBool() == true && timer.elapsed() < 3000);
	} else {
		qDebug() << "Called ConnectionManager::setOnline but we are already online";
	}
}

void ConnectionManager::setOffline() {
    qDebug() << "--->" << Q_FUNC_INFO;

    /** Signal to release lock waiting to connman autoconnect **/
    emit onLineWifi(false);

	QDBusReply<QVariantMap> reply = nm->GetProperties();
	QVariantMap properties = reply;
	if (properties["OfflineMode"].toBool() == false || properties["State"].toString() != "offline") {
		nm->SetProperty("OfflineMode", QDBusVariant(true));
	} else {
		qDebug() << "Called ConnectionManager::setOffline but we are already offline";
	}
}

bool ConnectionManager::isOffline() {
    qDebug() << "--->" << Q_FUNC_INFO;
#ifndef Q_WS_QWS
#ifdef FAKE_WIFI
    return false;
#endif
#endif
	QDBusReply<QVariantMap> reply = nm->GetProperties();
	QVariantMap properties = reply;
    qDebug() << Q_FUNC_INFO << properties["OfflineMode"].toBool() << " - " << properties["State"].toString();
	return (properties["OfflineMode"].toBool() && properties["State"] == "offline");
}

bool ConnectionManager::isConnected() {
    qDebug() << "--->" << Q_FUNC_INFO;
#ifndef Q_WS_QWS
#ifdef FAKE_WIFI
	return true;
#endif
#endif
	QDBusReply<QVariantMap> reply = nm->GetProperties();
	QVariantMap properties = reply;
	qDebug() << properties;
	return (properties["State"] == "online" || (properties["State"] == "ready"));
}

QString ConnectionManager::getConnectingNetwork() {
    qDebug() << "--->" << Q_FUNC_INFO;
	QDBusReply<TechnologiesList> reply = nm->GetServices();
	TechnologiesList services = reply;
	foreach(TechnologyStruct s, services) {
		QString state = s.properties["State"].toString();
		if (state == "configuration" || state == "association") {
            return s.path.path();
		}
	}
    return "";
}

QString ConnectionManager::getConnectedNetwork() {
    qDebug() << "--->" << Q_FUNC_INFO;
	QDBusReply<TechnologiesList> reply = nm->GetServices();
	TechnologiesList services = reply;
	foreach(TechnologyStruct s, services) {
		QString state = s.properties["State"].toString();
		if (state == "ready" || state == "online") {
            return s.path.path();
		}
	}
    return "";
}



NetworkService *ConnectionManager::getNetworkService(QString path) {
    qDebug() << "--->" << Q_FUNC_INFO;
	Service *service = new Service("net.connman", path , QDBusConnection::systemBus(), this);
	if (!service) {
		qDebug() << Q_FUNC_INFO << "Cannot get service from connman";
		return NULL;
	}
	NetworkService *n = new NetworkService(service);
	return n;
}

QString ConnectionManager::getState() {
    qDebug() << "--->" << Q_FUNC_INFO;
	QDBusReply<QVariantMap> reply = nm->GetProperties();
	QVariantMap properties = reply;
	qDebug() << properties;
	return properties["State"].toString();
}

void ConnectionManager::scan() {
    qDebug() << "--->" << Q_FUNC_INFO;
	QDBusReply<TechnologiesList> reply = nm->GetTechnologies();
	TechnologiesList technologies = reply;
	foreach(TechnologyStruct t, technologies) {
		if (t.properties["Name"] == "WiFi") {
			Technology *tech = new Technology("net.connman", t.path.path(), QDBusConnection::systemBus(), this);
			if (!tech) {
				qDebug() << "Cannot get technology object for path " << t.path.path();
				return;
			}
			if (!getConnectingNetwork().isEmpty()) {
				qDebug() << "Aborting scan because we are associating or connecting and dhd.ko could hang";
				return;
			}
			qDebug() << "Doing scan on " << t.path.path();
			QDBusReply<void> scanReply = tech->Scan();
			qDebug() << "Scan finished";
            emit scanFinished();
		}
	}
}

void ConnectionManager::registerAgent(QDBusObjectPath path) {
    qDebug() << "--->" << Q_FUNC_INFO;
	nm->RegisterAgent(path);
}

QList<Network> ConnectionManager::getNetworks() {
    qDebug() << "--->" << Q_FUNC_INFO;
	QList<Network> networks;
	// FIXME: We should construct this from the cached *services we have
	QDBusReply<TechnologiesList> reply = nm->GetServices();
	TechnologiesList services = reply;
    foreach(TechnologyStruct s, services) {
		Network n(s.path.path());
		n.setName(s.properties["Name"].toString());
		n.setFavorite(s.properties["Favorite"].toBool());
		n.setState(s.properties["State"].toString());
		n.setSecurity(s.properties["Security"].toString());
		n.setLevel(s.properties["Strength"].toInt());
		n.setAutoConnect(s.properties["AutoConnect"].toBool());
		QVariantMap ipv4;
		s.properties["IPv4"].value<QDBusArgument>() >> ipv4;
		n.setIpv4Method(ipv4["Method"].toString());
		n.setIpv4Address(ipv4["Address"].toString());
		n.setIpv4Netmask(ipv4["Netmask"].toString());
		n.setIpv4Router(ipv4["Gateway"].toString());
		QVariantMap ipv4Conf;
		s.properties["IPv4.Configuration"].value<QDBusArgument>() >> ipv4Conf;
		n.setIpv4ConfMethod(ipv4Conf["Method"].toString());
		n.setIpv4ConfAddress(ipv4Conf["Address"].toString());
		n.setIpv4ConfNetmask(ipv4Conf["Netmask"].toString());
		n.setIpv4ConfRouter(ipv4Conf["Gateway"].toString());

		//FIXME: We are exporting only the first one!
		if (s.properties["Nameservers"].toList().size() > 0) {
			QString dns = s.properties["Nameservers"].toList()[0].toString();
			n.setIpv4Dns(dns);
		}
		if (s.properties["Nameservers.Configuration"].toList().size() > 0) {
			QString dns = s.properties["Nameservers.Configuration"].toList()[0].toString();
			n.setIpv4ConfDns(dns);
		}

		networks.append(n);
	}
	return networks;
}

Network ConnectionManager::getNetwork(QString path) {
    qDebug() << "--->" << Q_FUNC_INFO;
    Network network = Network(path);;
    QDBusReply<TechnologiesList> reply = nm->GetServices();
    TechnologiesList services = reply;
    foreach(TechnologyStruct s, services) {
		if (s.path.path() == path) {
			network.setName(s.properties["Name"].toString());
			network.setFavorite(s.properties["Favorite"].toBool());
			network.setState(s.properties["State"].toString());
			network.setSecurity(s.properties["Security"].toString());
			network.setLevel(s.properties["Strength"].toInt());
			network.setAutoConnect(s.properties["AutoConnect"].toBool());
			QVariantMap ipv4;
			s.properties["IPv4"].value<QDBusArgument>() >> ipv4;
			network.setIpv4Method(ipv4["Method"].toString());
			network.setIpv4Address(ipv4["Address"].toString());
			network.setIpv4Netmask(ipv4["Netmask"].toString());
			network.setIpv4Router(ipv4["Gateway"].toString());
			QVariantMap ipv4Conf;
			s.properties["IPv4.Configuration"].value<QDBusArgument>() >> ipv4Conf;
			network.setIpv4ConfMethod(ipv4Conf["Method"].toString());
			network.setIpv4ConfAddress(ipv4Conf["Address"].toString());
			network.setIpv4ConfNetmask(ipv4Conf["Netmask"].toString());
			network.setIpv4ConfRouter(ipv4Conf["Gateway"].toString());

			//FIXME: We are exporting only the first one!
			if (s.properties["Nameservers"].toList().size() > 0) {
				QString dns = s.properties["Nameservers"].toList()[0].toString();
				network.setIpv4Dns(dns);
			} else
				network.setIpv4Dns(QString(""));
			if (s.properties["Nameservers.Configuration"].toList().size() > 0) {
				QString dns = s.properties["Nameservers.Configuration"].toList()[0].toString();
				network.setIpv4ConfDns(dns);
			} else
				network.setIpv4ConfDns(QString(""));

			break;
		}
	}
	return network;
}

void ConnectionManager::connectTo(QString path) {
    qDebug() << "--->" << Q_FUNC_INFO;
	Service *service = new Service("net.connman", path, QDBusConnection::systemBus(), this);
	if (!service) {
		qDebug() << "cannot get network " << path;
		return;
	}
	// This is a blocking call, so we usually spawn it from a separate helper.
	// We want to have a very high timeout. Setting timeouts it's only supported
	// on Qt > 4.8, so right now only for the device
#ifdef Q_WS_QWS
	service->setTimeout(300);
#endif

	qDebug() << "Trying to connect to " << path;
	// FIXME: this is sync, and we don't want to block here
	QDBusReply<void> reply = service->Connect();
	//service->SetProperty("Autoconnect", QDBusVariant(true));
}

void ConnectionManager::disconnectFrom(QString path) {
    qDebug() << "--->" << Q_FUNC_INFO;
	Service *service = new Service("net.connman", path, QDBusConnection::systemBus(), this);
	if (!service) {
		qDebug() << "cannot get network " << path;
		return;
	}

	// FIXME: Check if it is connected
	qDebug() << "disconnecting from " << path;
	QDBusReply<void> reply = service->Disconnect();
}

void ConnectionManager::removeAll() {
    qDebug() << "--->" << Q_FUNC_INFO;
    QDBusReply<TechnologiesList> reply = nm->GetServices();
    TechnologiesList services = reply;
    foreach(TechnologyStruct s, services)
        remove(s.path.path());
}


void ConnectionManager::remove(QString path) {
    qDebug() << "--->" << Q_FUNC_INFO;
	Service *service = new Service("net.connman", path, QDBusConnection::systemBus(), this);
    QDBusReply<TechnologiesList> reply = nm->GetServices();
    TechnologiesList services = reply;
    foreach(TechnologyStruct s, services) {
        if(s.path.path() == path &&  s.properties["State"].toString() == "ready") {
            qDebug() << "Removed connected network and change state to idle";
            qDebug() << "#################### EMITING DISCONNECTED ###########################";
                emit disconnected();
            state = "idle";
        }
    }
	if (!service) {
		qDebug() << "cannot get network " << path;
		return;
	}

	qDebug() << "Removing netowrk (and maybe disconnecting) " << path;
    reply = service->Remove();
}

void ConnectionManager::propertyChanged(const QString &name, const QDBusVariant &value) {
    qDebug() << "--->" << Q_FUNC_INFO;
    if (name != "State")
        return;

    QString v = value.variant().toString();
    qDebug() << "State changed value to: "<< v << " (old state was " << state << ")";
    if ((state == "idle" || state == "offline") && v == "ready") {
        qDebug() << "#################### EMITING CONNECTED ##########################";
        syncTime();
        emit connected();
        QString path = getConnectedNetwork();
        if (!path.isEmpty()) {
            if (connectedService != NULL) {
                disconnect(connectedService, 0, 0, 0);
                delete connectedService;
            }
            connectedService = getNetworkService(path);
            if (connectedService != NULL) {
                qDebug() << "connected network is " << path << ". connecting its strengthChanged signal";
                connect(connectedService, SIGNAL(strengthChanged(int)), this, SIGNAL(currentNetworkLevelChanged(int)));
                emit currentNetworkLevelChanged(connectedService->getStrength());
            }
        }
    } else if ((state == "ready" && (v == "idle" || v == "offline")) || (state == "idle" && v == "offline")) {
        qDebug() << "#################### EMITING DISCONNECTED ###########################";
        emit disconnected();
        if (connectedService != NULL) {
            disconnect(connectedService, 0, 0, 0);
            delete connectedService;
            connectedService = NULL;
        }
    }
    qDebug() << "************************ last state = " << state << " new state = " << v;
    state = v;
}

void ConnectionManager::servicesChanged(TechnologiesList added, const QList<QDBusObjectPath> &removed) {
    qDebug() << "--->" << Q_FUNC_INFO;
    emit someThingHasChanged();
	if (added.size() > 0 || removed.size() > 0)
		emit servicesChanged();
}

int doSyncTime() {
	QProcess process;

        process.start("htpdate -s -t google.com");
	
	if(!process.waitForStarted()) {
		qDebug() << Q_FUNC_INFO << "Cannot exec htpdate. Aborting.";
		return -1;
	}

	process.waitForFinished();
	if (process.state() == QProcess::Running) {
		qDebug() << Q_FUNC_INFO << "htpdate is taking more than 30 secs. Aborting.";
		process.kill();
		return -1;
	}

	if (process.exitStatus() != QProcess::NormalExit) {
		qDebug() << Q_FUNC_INFO << "htpdate crashed. Aborting";
		return -1;
	}

	if (process.exitCode() != 0) {
		qDebug() << Q_FUNC_INFO << "htpdate finished with error. Aborting";
		return -1;
	}

	qDebug() << Q_FUNC_INFO << "htpdate run sucessfully:" << process.readAll();

	return 0;
}

void ConnectionManager::handleSyncTimeFinished() {
	int result = timeSyncwatcher.result();
	qDebug() << Q_FUNC_INFO << result;
	if (result == 0) {
		qDebug() << "timeSync future finished with good result, emiting timeUpdated";
		emit timeUpdated();
	} else {
		qDebug() << "*** Warning *** timeSync future finished with error. Not emitting timeUpdated";
	}
}

void ConnectionManager::syncTime() {

	QFuture<int> future = QtConcurrent::run(doSyncTime);
        timeSyncwatcher.setFuture(future);
}

void ConnectionManager::setNetworkDHCPConfig(QString path) {
	qDebug() << "--->" << Q_FUNC_INFO;
	Service *service = new Service("net.connman", path , QDBusConnection::systemBus(), this);
	if (!service) {
		qDebug() << Q_FUNC_INFO << "Cannot get service from connman";
		return;
	}
	QVariantMap config;
	config["Method"] = "dhcp";
	config["Address"] = "";
	config["Netmask"] = "";
	config["Gateway"] = "";
	service->SetProperty("Nameservers.Configuration", QDBusVariant(QStringList()));
	service->SetProperty("IPv4.Configuration", QDBusVariant(config));
}

void ConnectionManager::setNetworkManualConfig(QString path, QString ip, QString mask, QString gw, QStringList nameservers) {
	qDebug() << "--->" << Q_FUNC_INFO;
	Service *service = new Service("net.connman", path , QDBusConnection::systemBus(), this);
	if (!service) {
		qDebug() << Q_FUNC_INFO << "Cannot get service from connman";
		return;
	}
	QVariantMap config;
	config["Method"] = "manual";
	config["Address"] = ip;
	config["Netmask"] = mask;
	config["Gateway"] = gw;
	service->SetProperty("Nameservers.Configuration", QDBusVariant(nameservers));
	service->SetProperty("IPv4.Configuration", QDBusVariant(config));
}

bool ConnectionManager::scanForESSID(QString essid) {

    QTimer timer;
    QEventLoop loop;

    HiddenQuery *query = new HiddenQuery(essid);
    loop.connect(query, SIGNAL(Done()), SLOT(quit()));
    loop.connect(&timer, SIGNAL(timeout()), SLOT(quit()));
    timer.start(10 * 1000);
    query->start();
    loop.exec();

    bool result = query->getResult();
    delete query;
    return result;
}


// FIXME: implement destructor releasing agent and connections
