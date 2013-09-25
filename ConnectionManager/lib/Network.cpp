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

#include <QString>
#include <QDebug>

#include "Network.h"

Network::Network (const Network &other)
{
    path = other.path;
    name = other.name;
    state = other.state;
    security = other.security;
    ipv4Method = other.ipv4Method;
    ipv4Address = other.ipv4Address;
    ipv4Netmask = other.ipv4Netmask;
    ipv4Router = other.ipv4Router;
    ipv4Dns = other.ipv4Dns;
    level = other.level;
    favorite = other.favorite;
    autoConnect = other.autoConnect;
}

Network::Network(QString path) {
	this->path = path;
}
QString Network::getPath() {
	return path;
}
QString Network::getName() {
	return name;
}
bool Network::getFavorite() {
	return favorite;
}
QString Network::getState() {
	return state;
}
QString Network::getSecurity() {
	return security;
}
QString Network::getIpv4Method() {
	return ipv4Method;
}
QString Network::getIpv4Address() {
	return ipv4Address;
}
QString Network::getIpv4Netmask() {
	return ipv4Netmask;
}
QString Network::getIpv4Router() {
	return ipv4Router;
}
QString Network::getIpv4Dns() {
	return ipv4Dns;
}
QString Network::getIpv4ConfMethod() {
	return ipv4ConfMethod;
}
QString Network::getIpv4ConfAddress() {
	return ipv4ConfAddress;
}
QString Network::getIpv4ConfNetmask() {
	return ipv4ConfNetmask;
}
QString Network::getIpv4ConfRouter() {
	return ipv4ConfRouter;
}
QString Network::getIpv4ConfDns() {
	return ipv4ConfDns;
}
int Network::getLevel() {
	return level;
}
bool Network::getAutoConnect() {
	return autoConnect;
}
void Network::setName(QString name) {
	this->name = name;
}
void Network::setFavorite(bool favorite) {
	this->favorite = favorite;
}
void Network::setSecurity(QString security) {
	this->security = security;
}
void Network::setState(QString state) {
	this->state = state;
}
void Network::setIpv4Method(QString method) {
	this->ipv4Method = method;
}
void Network::setIpv4Address(QString address) {
	this->ipv4Address = address;
}
void Network::setIpv4Netmask(QString netmask) {
	this->ipv4Netmask = netmask;
}
void Network::setIpv4Router(QString router) {
	this->ipv4Router = router;
}
void Network::setIpv4Dns(QString dns) {
	this->ipv4Dns = dns;
}
void Network::setIpv4ConfMethod(QString method) {
	this->ipv4ConfMethod = method;
}
void Network::setIpv4ConfAddress(QString address) {
	this->ipv4ConfAddress = address;
}
void Network::setIpv4ConfNetmask(QString netmask) {
	this->ipv4ConfNetmask = netmask;
}
void Network::setIpv4ConfRouter(QString router) {
	this->ipv4ConfRouter = router;
}
void Network::setIpv4ConfDns(QString dns) {
	this->ipv4ConfDns = dns;
}
void Network::setLevel(int level) {
	this->level = level;
}
void Network::setAutoConnect(bool autoConnect) {
	this->autoConnect = autoConnect;
}
bool Network::isConnected() {
    return (this->state == "ready" || this->state == "online");
}
bool Network::isConnecting() {
    return (this->state == "association" || this->state == "configuration");
}
bool Network::isFavorite(){
    return favorite;
}
