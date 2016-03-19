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

#include <iostream>
#include <string>

#define READLINE_CALLBACKS 1

#include <readline/readline.h>
#include <readline/history.h>

#include <QDebug>
#include <QTextStream>
#include <QApplication>
#include <Test.h>

#include <ConnectionManager.h>

static ConnectionManager *cm;

Test::Test(QObject *parent) : QObject(parent) {
	app = (QApplication*)parent;
}

void listNetworks() {
	//cm->setOnline();
	QList<Network> networks = cm->getNetworks();
	foreach (Network n, networks) {
		qDebug() << "network " << n.getPath();
		qDebug() << " name:" << n.getName();
		qDebug() << " security:" << n.getSecurity();
		qDebug() << " favorite:" << n.getFavorite();
		qDebug() << " level:" << n.getLevel();
		qDebug() << " autoconnect:" << n.getAutoConnect();
		qDebug() << " state:" << n.getState();
	}
}

void usage() {
	std::cout << "Availabe commands:\n";
	std::cout << " list: list networks\n";
	std::cout << " quit: finish the application\n";
	std::cout << " connect <path>: connects to network with path <path>\n";
	std::cout << " disconnect <path>: disconnects from network with path <path>\n";
	std::cout << " remove <path>: forgets network with path <path> and disconnect if connected to it\n";
	std::cout << " offine: Go to offline mode\n";
	std::cout << " online: Go to online mode\n";
	std::cout << " status: Show onffline mode status\n";
	std::cout << " scan: Request an scan (on wifi technology)\n";
	std::cout << " help: Display this help\n";
}

bool processLine(QString line);

void my_rlhandler(char* line){
	if(line==NULL){
		QApplication::quit();
	} else {
    		if(*line!=0){
          		// If line wasn't empty, store it so that uparrow retrieves it
      			//add_history(line);
    		}
		if (processLine(line))
			add_history(line);
    		free(line);
  	}
}



bool processLine(QString line) {
	if (line == "list") {
		listNetworks();
	} else if (line == "quit") {
		delete cm;
		QApplication::quit();
		exit(0);
	} else if (line.startsWith("offline")) {
		cm->setOffline();
	} else if (line.startsWith("online")) {
		cm->setOnline();
	} else if (line.startsWith("scan")) {
		cm->scan();
	} else if (line.startsWith("status")) {
		qDebug() << "Offline mode: " << cm->isOffline();
	} else if (line.startsWith("connect ")) {
		cm->connectTo(line.split(" ")[1]);
	} else if (line.startsWith("disconnect ")) {
		cm->disconnectFrom(line.split(" ")[1]);
	} else if (line.startsWith("remove ")) {
		cm->remove(line.split(" ")[1]);
	} else if (line.startsWith("help")) {
		usage();
	} else if (line.isEmpty()) {
		return false;
	} else {
		qDebug() << line << ": unknown command\n";
		usage();
	}
	return true;
}

void Test::doWork() {
	cm = ConnectionManager::getInstance();
	if (!cm) {
		qDebug() << "Cannot initialize ConnectionManager. Aborting";
		QApplication::quit();
		return;
	}

	rl_callback_handler_install("command:", (rl_vcpfunc_t*) &my_rlhandler);

	while (1) {
		rl_callback_read_char();
		QCoreApplication::processEvents();
	}

}


