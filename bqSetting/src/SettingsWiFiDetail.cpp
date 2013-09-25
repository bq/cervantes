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
#include "SettingsWiFiDetail.h"
#include "SettingsConfigureNetwork.h"
#include "Network.h"
#include "Screen.h"
#include "QBookApp.h"

#include <QPainter>

SettingsWiFiDetail::SettingsWiFiDetail(QString path, QBookForm *parent)
        : QBookForm(parent)
        , m_path(path)
{
        qDebug() << Q_FUNC_INFO << path;
        setupUi(this);

        connect(backBtn, SIGNAL(clicked()),this, SLOT(closeDialog()));
        connect(forgetBtn, SIGNAL(clicked()),this, SLOT(forgetWifi()));
        connect(disconnectBtn, SIGNAL(clicked()),this, SLOT(disconnectWifi()));
        connect(edit_Btn, SIGNAL(clicked()),this, SLOT(editWifi()));
	
        connect(this, SIGNAL(hideMe()), this, SLOT(closeDialog()));

	// FIXME: 
	// On Network configuration change we get a online->ready + ready->online state change
	// Currently ConnectionManager is not exposing that. It will once the captive_networks branch is merges
	// So by now we update the info on every service change, but we should connect the real connected signal to update this.
	connect(ConnectionManager::getInstance(), SIGNAL(servicesChanged()), this, SLOT(update()));

        QFile fileSpecific(":/res/settings_styles.qss");
        QFile fileCommons(":/res/settings_styles_generic.qss");
        fileSpecific.open(QFile::ReadOnly);
        fileCommons.open(QFile::ReadOnly);

        QString styles = QLatin1String(fileSpecific.readAll() + fileCommons.readAll());
        setStyleSheet(styles);

}

SettingsWiFiDetail::~SettingsWiFiDetail()
{
        qDebug() << Q_FUNC_INFO;
}

void SettingsWiFiDetail::closeDialog()
{
        qDebug() << Q_FUNC_INFO;
        Screen::getInstance()->queueUpdates();
        QBookApp::instance()->popForm(this);
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE,false);
        Screen::getInstance()->flushUpdates();
}

void SettingsWiFiDetail::activateForm()
{
    qDebug() << Q_FUNC_INFO;
	update();

}

void SettingsWiFiDetail::deactivateForm ()
{
    qDebug() << Q_FUNC_INFO;

}

void SettingsWiFiDetail::update()
{
	Network network = ConnectionManager::getInstance()->getNetwork(m_path);

        populateProperties(network);

        if(network.isConnected())
            disconnectBtn->setText(tr("Disconnect"));
        else
            disconnectBtn->setText(tr("Connect"));

        if(network.isFavorite())
            forgetBtn->show();
        else
            forgetBtn->hide();
}

void SettingsWiFiDetail::disconnectWifi ()
{
        qDebug() << Q_FUNC_INFO;

        Screen::getInstance()->queueUpdates();
	Network network = ConnectionManager::getInstance()->getNetwork(m_path);

        if(network.isConnected())
            emit disconnectWifiSignal(m_path);
        else
            emit connectWifiSignal(m_path);

        QBookApp::instance()->popForm(this);

        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
}

void SettingsWiFiDetail::forgetWifi ()
{
        qDebug() << Q_FUNC_INFO;

        emit forgetWifiSignal(m_path);
        Screen::getInstance()->queueUpdates();
        QBookApp::instance()->popForm(this);
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
}

void SettingsWiFiDetail::populateProperties(Network network)
{
        qDebug() << Q_FUNC_INFO;

        if(!network.isConnected()){
            ipAddress->hide();
            ipAddressEdit->hide();
            subnetMask->hide();
            subnetEdit->hide();
            router->hide();
            routerEdit->hide();
            dns->hide();
            dnsEdit->hide();
        } else {
            ipAddress->show();
            ipAddressEdit->show();
            subnetMask->show();
            subnetEdit->show();
            router->show();
            routerEdit->show();
            dns->show();
            dnsEdit->show();
	}

        security_data->setText(tr(network.getSecurity().toUtf8()));

        if(network.getLevel() < 33)
            signal_data->setText(tr("Low"));
        else if(network.getLevel() < 66)
            signal_data->setText(tr("Medium"));
        else if(network.getLevel() < 100)
            signal_data->setText(tr("High"));

        wifiNameLabel->setText(network.getName());
        ipAddressEdit->setText(network.getIpv4Address());
        subnetEdit->setText(network.getIpv4Netmask());
        routerEdit->setText(network.getIpv4Router());
	qDebug() << network.getIpv4Dns();
        dnsEdit->setText(network.getIpv4Dns());
}

void SettingsWiFiDetail::editWifi()
{
	qDebug() << Q_FUNC_INFO << m_path;

	SettingsConfigureNetwork *configureNetwork = new SettingsConfigureNetwork(m_path, (QBookForm*) parent());
	Screen::getInstance()->queueUpdates();
       	configureNetwork->move(0, QBookApp::instance()->getStatusBar()->height());
       	QBookApp::instance()->pushForm(configureNetwork);
       	Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
       	Screen::getInstance()->flushUpdates();
}

void SettingsWiFiDetail::paintEvent (QPaintEvent *)
{
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
