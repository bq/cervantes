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

#include <QDebug>
#include <QObject>
#include <QApplication>

#include <Offline.h>


Offline::Offline(QObject *parent, ConnectionManager *cm) : QObject(parent) { 
	this->cm = cm;
};

void Offline::doWork() {
	qDebug() << "going offline";
        cm->setOffline();
        cm->powerWifi(false);
	QApplication::quit();
}
