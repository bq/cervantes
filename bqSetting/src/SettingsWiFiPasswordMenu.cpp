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

#include "SettingsWiFiPasswordMenu.h"
#include "QBookApp.h"
#include "Keyboard.h"
#include <QWidget>
#include <QPainter>
#include <QKeyEvent>

SettingsWiFiPasswordMenu::SettingsWiFiPasswordMenu (QWidget *parent, QString name, QString security)
    : QWidget(parent)
{
        qDebug() << Q_FUNC_INFO;

        setupUi(this);

        wifiNameLbl->setText(name);

        connect(cancelBtn, SIGNAL(clicked()), this, SLOT(cancelClicked()));
        connect(acceptBtn, SIGNAL(clicked()), this, SLOT(okClicked()));
        connect(passwordLineEdit, SIGNAL(clicked()), this, SLOT(setKeyboard()));
        connect(hiddenPassword, SIGNAL(clicked(bool)), this, SLOT(hiddenPasswordHandler(bool)));
        setKeyboard();

        passwordLineEdit->setFocus();

}

SettingsWiFiPasswordMenu::~SettingsWiFiPasswordMenu ()
{
        qDebug() << Q_FUNC_INFO;
}

void SettingsWiFiPasswordMenu::cancelClicked ()
{
        qDebug() << Q_FUNC_INFO;

        hideMe();
        emit canceled();
}

void SettingsWiFiPasswordMenu::okClicked ()
{
        qDebug() << Q_FUNC_INFO;

        hideMe();
        password = passwordLineEdit->text();
        emit ok(password);
}

void SettingsWiFiPasswordMenu::hideMe()
{
        qDebug() << Q_FUNC_INFO;

        Screen::getInstance()->queueUpdates();
        QBookApp::instance()->hideKeyboard();
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
        hide();
        Screen::getInstance()->flushUpdates();
}

void SettingsWiFiPasswordMenu::setKeyboard ()
{
        qDebug() << Q_FUNC_INFO;

        Keyboard* keyboard = QBookApp::instance()->showKeyboard(tr("Connect"));
        keyboard->handleMyQLineEdit(passwordLineEdit);
        connect(keyboard, SIGNAL(actionRequested()), this, SLOT(okClicked()));
        connect(keyboard, SIGNAL(newLinePressed()), this, SLOT(okClicked()));
}

/*virtual*/ void SettingsWiFiPasswordMenu::keyPressEvent(QKeyEvent *event)
{
        qDebug() << Q_FUNC_INFO;

        event->accept();
}

/*virtual*/ void SettingsWiFiPasswordMenu::keyReleaseEvent(QKeyEvent *event)
{
        qDebug() << Q_FUNC_INFO;

        if(event->key() == QBook::QKEY_BACK)
            hideMe();
        else
            event->accept();
}

void SettingsWiFiPasswordMenu::hiddenPasswordHandler(bool clicked)
{
        qDebug() << Q_FUNC_INFO;
        if(clicked)
            passwordLineEdit->setEchoMode(QLineEdit::Password);
        else
            passwordLineEdit->setEchoMode(QLineEdit::Normal);

}

void SettingsWiFiPasswordMenu::paintEvent (QPaintEvent *)
{
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }
