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
#include <QHostAddress>
#include "SettingsConfigureNetwork.h"
#include "SettingConfigureNetworkListActions.h"
#include "Network.h"
#include "Screen.h"
#include "QBookApp.h"

#include <QPainter>

SettingsConfigureNetwork::SettingsConfigureNetwork(QString path, QBookForm *parent)
        : QBookForm(parent),
	m_path(path)
  , fields_ok(false)
{
        qDebug() << Q_FUNC_INFO;

        setupUi(this);

        editWidgets << ipAddressLineEdit << subnettingLineEdit << gatewayLineEdit << dnsListLineEdit;

        connect(backBtn, SIGNAL(clicked()),this, SLOT(closeDialog()));
        connect(cancelBtn, SIGNAL(clicked()),this, SLOT(closeDialog()));
        connect(saveBtn, SIGNAL(clicked()),this, SLOT(saveConfig()));
        foreach(MyQLineEdit *e, editWidgets) {
        	connect(e, SIGNAL(clicked()),this, SLOT(onEditClicked()));
        }
        connect(confTypeBtn, SIGNAL(clicked()), this, SLOT(showOptions()));

        m_actions = new SettingConfigureNetworkListActions(this);
        m_actions->hide();

        connect(m_actions, SIGNAL(setDHCP()), this, SLOT(setDHCP()));
        connect(m_actions, SIGNAL(setManual()), this, SLOT(setManual()));

        connect(this, SIGNAL(hideMe()), this, SLOT(closeDialog()));

        QFile fileSpecific(":/res/settings_styles.qss");
        QFile fileCommons(":/res/settings_styles_generic.qss");
        fileSpecific.open(QFile::ReadOnly);
        fileCommons.open(QFile::ReadOnly);

        QString styles = QLatin1String(fileSpecific.readAll() + fileCommons.readAll());
        setStyleSheet(styles);
        keyboard = NULL;
        currentEdit = NULL;
}

SettingsConfigureNetwork::~SettingsConfigureNetwork()
{
        qDebug() << Q_FUNC_INFO;
}


void SettingsConfigureNetwork::showOptions()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();

    // Show Filter Layer as a Popup
    if(!m_actions->isVisible())
    {
        QPoint pos(confTypeBtn->mapToGlobal(QPoint(0,0)));
        pos.setY(pos.y() + confTypeBtn->height());
        m_actions->move(mapFromGlobal(pos));
        m_actions->resize(confTypeBtn->width(), m_actions->height());

        m_actions->show();
    }
    else
        m_actions->hide();

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

bool SettingsConfigureNetwork::isValidField(MyQLineEdit *e)
{
        qDebug() << Q_FUNC_INFO;
	if (e->text().isEmpty())
		return false;

	qDebug() << Q_FUNC_INFO << e << e->text();
	QHostAddress address(e->text());
	if (address.isNull())
		return false;

	qDebug() << address.protocol();
	if (QAbstractSocket::IPv4Protocol != address.protocol())
		return false;

	return true;
}

void SettingsConfigureNetwork::setDHCP()
{
    confTypeBtn->setText("DHCP");
    m_actions->hide();
    updateFields();
}

void SettingsConfigureNetwork::setManual()
{
    confTypeBtn->setText("Manual");
    m_actions->hide();
    updateFields();
    handleSaveBtnStyle();
}

void SettingsConfigureNetwork::onEditClicked(MyQLineEdit *fakeSender) {
        qDebug() << Q_FUNC_INFO;

        if (confTypeBtn->text() == "DHCP")
            return;

	MyQLineEdit *edit;
	if (fakeSender != NULL)
		edit = fakeSender;
	else
		edit = (MyQLineEdit*)sender();
	currentEdit = edit;
	foreach(MyQLineEdit *e, editWidgets) {
		disconnect(e, SIGNAL(textChanged(const QString&)), 0, 0);
	}

	keyboard = QBookApp::instance()->showKeyboard(tr("Hide"), false, Keyboard::NORMAL);
	disconnect(keyboard, SIGNAL(actionRequested()), 0, 0);
	connect(keyboard, SIGNAL(actionRequested()), this, SLOT(onHideKeyboard()));
    connect(keyboard, SIGNAL(newLinePressed()), this, SLOT(onHideKeyboard()));
	keyboard->handleMyQLineEdit(edit);
	connect(edit, SIGNAL(textChanged(const QString&)), this, SLOT(onEditChanged(const QString&)));
}

void SettingsConfigureNetwork::onEditChanged(const QString& text) {
        qDebug() << Q_FUNC_INFO;
	updateFields();
}

void SettingsConfigureNetwork::handleSaveBtnStyle(){
    //Apply style to save button only when hide the keyboard.
    if(fields_ok){
        saveBtn->setEnabled(true);
        saveBtn->setStyleSheet("color:#0F0F0F;");
    }
    else{
        saveBtn->setEnabled(false);
        saveBtn->setStyleSheet("color:#666;");
    }
}

void SettingsConfigureNetwork::onHideKeyboard() {
	qDebug() << Q_FUNC_INFO;
	keyboard->hide();
    handleSaveBtnStyle();
    Screen::getInstance()->refreshScreen();
}

void SettingsConfigureNetwork::updateFields()
{
        qDebug() << Q_FUNC_INFO;

	int fieldsOk = 0;

    if (confTypeBtn->text() == "DHCP") {
		foreach(MyQLineEdit *e, editWidgets) {
			e->setStyleSheet("background-color:#EDEDED;");
			e->setEnabled(false);
		}
	} else {
		foreach(MyQLineEdit *e, editWidgets) {
			e->setEnabled(true);
			if (isValidField(e)) {
				e->setStyleSheet("background-image: url(\":/res/check_ico.png\");");
				fieldsOk++;
			} else {
				e->setStyleSheet("background-image: none;");
			}
		}
	}
    if (confTypeBtn->text() == "DHCP" || fieldsOk == editWidgets.size())
        fields_ok = true;
    else
        fields_ok = false;
	return;
}

void SettingsConfigureNetwork::closeDialog()
{
        qDebug() << Q_FUNC_INFO;
        Screen::getInstance()->queueUpdates();
        if(keyboard && keyboard->isVisible())
            keyboard->hide();
        QBookApp::instance()->popForm(this);
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
        Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE,false);
        Screen::getInstance()->flushUpdates();
}

void SettingsConfigureNetwork::activateForm()
{
        qDebug() << Q_FUNC_INFO;

        fillConfig();
}

void SettingsConfigureNetwork::deactivateForm ()
{
        qDebug() << Q_FUNC_INFO;

}

void SettingsConfigureNetwork::paintEvent (QPaintEvent *)
{
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void SettingsConfigureNetwork::fillConfig () {
	Network network = ConnectionManager::getInstance()->getNetwork(m_path);
	qDebug() << Q_FUNC_INFO << network.getIpv4ConfMethod();

	networkNameLbl->setText(network.getName());

	if (!network.getFavorite() || network.getIpv4ConfMethod() == "dhcp") {
        	confTypeBtn->setText("DHCP");
		saveBtn->setEnabled(true);
		saveBtn->setStyleSheet("color:#0F0F0F;");
	} else {
        	confTypeBtn->setText("Manual");
		ipAddressLineEdit->setText(network.getIpv4ConfAddress());
		subnettingLineEdit->setText(network.getIpv4ConfNetmask());
		gatewayLineEdit->setText(network.getIpv4ConfRouter());
		dnsListLineEdit->setText(network.getIpv4ConfDns());
	}
	updateFields();
}
		
void SettingsConfigureNetwork::saveConfig () {
    qDebug() << Q_FUNC_INFO << confTypeBtn->text();

    if (confTypeBtn->text() == "DHCP") {
		ConnectionManager::getInstance()->setNetworkDHCPConfig(m_path);
	} else {
		QString ip = ipAddressLineEdit->text();
		QString mask = subnettingLineEdit->text();
		QString gw = gatewayLineEdit->text();
		QStringList dns;
		dns << dnsListLineEdit->text();
		ConnectionManager::getInstance()->setNetworkManualConfig(m_path, ip, mask, gw, dns);
	}
	closeDialog();
}

