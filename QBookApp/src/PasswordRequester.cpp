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

#include <unistd.h>
#include <iostream>
#include <string>
#include <unistd.h>

#include <QObject>
#include <QApplication>
#include <QDBusConnection>

#include <QString>
#include <QDebug>

#include "PasswordRequester.h"
#include "ConnectionManager.h"
#include "MyAgent.h"
#include "Agent.h"

#include "SettingsWiFiPasswordMenu.h"
#include "QBookApp.h"
#include "Screen.h"
#include "SleeperThread.h"


//PasswordRequester::PasswordRequester(ConnectionManager *cm, QBookApp *parent) {
PasswordRequester::PasswordRequester(ConnectionManager *cm, void *parent) : QObject((QObject*)parent)  {
    m_cm = cm;
	m_parent = parent;
	TechnologiesListTypesInit();
	text = NULL;
    MyAgent *agent = new MyAgent;
	agent->setPasswordRequester(this);
    new Agent(agent);

    QDBusConnection connection = QDBusConnection::systemBus();
    bool res = connection.registerService("com.bq.reader.connmanAgent");
    if (!res) {
            qDebug() << Q_FUNC_INFO << "Cannot register the net.connman.Agent service";
            qDebug() << Q_FUNC_INFO << QDBusConnection::systemBus().lastError().message();
            return;
    }

    res = connection.registerObject("/bqlala", agent);
    if (!res) {
            qDebug() << Q_FUNC_INFO << "Cannot register the agent object";
            qDebug() << Q_FUNC_INFO << QDBusConnection::systemBus().lastError().message();
            return;
    }

    registerAgent();
    connect(agent, SIGNAL(wrongPassword(QString)), this, SIGNAL(wrongPassword(QString)), Qt::UniqueConnection);
    connect(agent, SIGNAL(connectionFailed(QString,QString)), this, SIGNAL(connectionFailed(QString,QString)));
    connect(agent, SIGNAL(released()), this, SLOT(agentReleased()));
    connect(QBookApp::instance(), SIGNAL(connmandRestarted()), this, SLOT(registerAgent()));
}

void PasswordRequester::agentReleased(void) {
	qDebug() << "\n\n\n\n\n\nAgent released. That means that connmand has unexpectly exited. Trying to re-register against new instance\n\n\n\n";
}

void PasswordRequester::registerAgent(void) {
	qDebug() << "Registering agent";
	m_cm->registerAgent(QDBusObjectPath("/bqlala"));
	qDebug() << "Agent registered";
}

PasswordRequester::~PasswordRequester() {
}

void PasswordRequester::passwordDialogOk(QString password) {
	text = new QString (password);
	gotPassword = true;
	emit passwordSent();
}

void PasswordRequester::passwordDialogCancel() {
	text = NULL;
	gotPassword = true;
	emit cancelPassword();
}

QString *PasswordRequester::askPassword(QString path) {
	qDebug() << Q_FUNC_INFO << "PasswordRequester::askPassword: Aksing password for path: " << path;
	
	m_path = path;
	emit passwordRequested();
	QList<Network> networks = m_cm->getNetworks();
	bool found = false;
	QString name;
	QString security;
        foreach(Network n, networks) {
        if (n.getPath() == path) {
            name = n.getName();
            security = n.getSecurity();
            found = true;
            break;
        }
    }
	if (!found) {
		qDebug () << "network " << path << " is not avaible, aborting password request";
		return NULL;
	}

    qDebug() << Q_FUNC_INFO << "Secutiry is " << security;

    // If the Wifi list is not present, doesn't show the wifi password request dialog
    if(QBookApp::instance()->getCurrentForm() != QBookApp::instance()->m_wifiSelection)
        return NULL;

    Screen::getInstance()->queueUpdates();
    SettingsWiFiPasswordMenu *dialog = new SettingsWiFiPasswordMenu(((QBookApp*)m_parent)->getCurrentForm(), name, security);
    connect(dialog,SIGNAL(ok(QString)),this,SLOT(passwordDialogOk(QString)));
    connect(dialog,SIGNAL(canceled()),this,SLOT(passwordDialogCancel()));
    dialog->show();
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

	gotPassword = false;
	// FIXME: FER To avoid this ugly loop waiting we can implement a dbus delayed reply
        // See delayed replies at http://doc.trolltech.com/4.2/qdbusdeclaringslots.html
	while (!gotPassword) {
        	QCoreApplication::processEvents();
		SleeperThread::msleep(40);
	}

    delete dialog;
    dialog = NULL;
    qDebug() << "Sending password" << (text ? *text : "(NULL)");

	return text;
}
