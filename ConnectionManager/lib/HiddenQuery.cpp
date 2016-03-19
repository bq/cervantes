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

#include <QList>
#include <QVariant>
#include <QByteArray>

#include "ArrayArrayBytes.h"
#include "Supplicant.h"
#include "HiddenQuery.h"

HiddenQuery::HiddenQuery(QString essid) {
    this->essid = essid;
    result = false;
    interface = getWpaInterface();
    if (!interface) {
        qDebug() << "Cannot get WpaInterface object";
        return;
    }

}

bool HiddenQuery::getResult() {
    return result;
}

void HiddenQuery::scanForESSIDDone(bool result){
    qDebug() << Q_FUNC_INFO << result;
    emit Done();
}

WpaInterface *HiddenQuery::getWpaInterface() {
    Supplicant *supplicant = new Supplicant("fi.w1.wpa_supplicant1", "/fi/w1/wpa_supplicant1", QDBusConnection::systemBus(), this);
    if (!supplicant) {
	qDebug() << "Cannot get supplicant object";
	return NULL;
    }
    if (supplicant->interfaces().size() <= 0) {
	qDebug() << "Cannot get wpa_supplicant interface path";
	delete supplicant;
	return NULL;
    }
    QString path = supplicant->interfaces()[0].path();
    qDebug() << "getting wpa interface on path: " << path;
    delete supplicant;

    WpaInterface *interface = new WpaInterface("fi.w1.wpa_supplicant1", path, QDBusConnection::systemBus(), this);
    if (!interface) {
	qDebug() << "Cannot get WpaInterface object";
	return NULL;
    }

    return interface;
}


void HiddenQuery::scanForESSIDBSSAdded(const QDBusObjectPath &path, const QVariantMap &properties) {
    qDebug() << Q_FUNC_INFO;
    qDebug() << path.path();
    qDebug() << properties["SSID"].toString();

    if (properties["SSID"].toString() == essid) {
	qDebug() << "SSID found, setting result to true";
	result = true;
    }
}

void HiddenQuery::start() {

    qDebug() << "Scanning for specific essid: " << essid;

    QByteArray bytes;
    int i;
    for (i=0; i<essid.size(); i++) {
        // FIXME: Need to re-check this conversion from QString to proper SSID byte format
        bytes.append((uchar)essid.at(i).toAscii());
    }

    QList<QByteArray> list;
    list.append(bytes);
    QVariant var;
    var.setValue(list);
    QVariantMap query;
    query.insert("Type", "active");
    query.insert("SSIDs", var);

    connect(interface, SIGNAL(ScanDone(bool)), this, SLOT(scanForESSIDDone(bool)));
    connect(interface, SIGNAL(BSSAdded(const QDBusObjectPath &, const QVariantMap &)), this, SLOT(scanForESSIDBSSAdded(const QDBusObjectPath &, const QVariantMap &)));
    interface->Scan(query);
}

HiddenQuery::~HiddenQuery() {
    delete interface;
}
