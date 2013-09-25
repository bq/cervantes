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

#ifndef __NETWORK__
#define __NETWORK__

#include <QString>
#include <QMetaType>

class Network {
public:
        friend class ConnectionManager;
        Network(){};
        Network (const Network &other);
    Network(QString);
	QString getPath();
	QString getName();
	bool getFavorite();
	QString getState();
	QString getSecurity();
	QString getIpv4Method();
	QString getIpv4Address();
	QString getIpv4Netmask();
	QString getIpv4Router();
	QString getIpv4Dns();
	QString getIpv4ConfMethod();
	QString getIpv4ConfAddress();
	QString getIpv4ConfNetmask();
	QString getIpv4ConfRouter();
	QString getIpv4ConfDns();
	int getLevel();
	bool getAutoConnect();
    void setState(QString);
    void setName(QString);
    bool isConnected();
    bool isConnecting();
    bool isFavorite();

    bool operator==(const Network &b) const{
        return this->path == b.path;
    }

protected:

	void setFavorite(bool);
	void setSecurity(QString);
	void setIpv4Method(QString);
	void setIpv4Address(QString);
	void setIpv4Netmask(QString);
	void setIpv4Router(QString);
	void setIpv4Dns(QString);
	void setIpv4ConfMethod(QString);
	void setIpv4ConfAddress(QString);
	void setIpv4ConfNetmask(QString);
	void setIpv4ConfRouter(QString);
	void setIpv4ConfDns(QString);
	void setLevel(int);
	void setAutoConnect(bool);
private:
	QString path;
	QString name;
	QString state;
	QString security;
	QString ipv4Method;
	QString ipv4Address;
	QString ipv4Netmask;
	QString ipv4Router;
	QString ipv4Dns;
	QString ipv4ConfMethod;
	QString ipv4ConfAddress;
	QString ipv4ConfNetmask;
	QString ipv4ConfRouter;
	QString ipv4ConfDns;
	int level;
	bool favorite;
	bool autoConnect;


};

Q_DECLARE_METATYPE(Network);
//Q_DECLARE_METATYPE(Network*);

#endif
