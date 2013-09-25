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


#include "NetworkService.h"

NetworkService::NetworkService(Service *service) : QObject(NULL) {
	this->service = service;
	connect(service, SIGNAL(PropertyChanged(const QString&, const QDBusVariant&)),
                this, SLOT(serviceChanged(const QString&, const QDBusVariant&)));
}

NetworkService::~NetworkService() {
	delete service;
}

void NetworkService::serviceChanged(const QString &name, const QDBusVariant &v) {
	if (name == "State") {
		QString value = v.variant().toString();
		if (value == "ready") {
			emit connected();
        	} else if (value == "failure" || value == "idle") {
			emit error();
        	}
	} else if (name == "Strength") {
		int value = v.variant().toInt();
		qDebug() << "Strength changed to " << value << ". Emiting strenthChanged signal";
		emit strengthChanged(value);
	}
}

int NetworkService::getStrength() {
	QDBusReply<QVariantMap> reply = service->GetProperties();
        QVariantMap properties = reply;
        return  properties["Strength"].toInt();
}
