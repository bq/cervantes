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

#include "QSettingLinkPc.h"
#include "Screen.h"
#include <QDebug>

#include "InfoDialog.h"
#include "Storage.h"
#include "QBookApp.h"


QSettingLinkPc::QSettingLinkPc(QWidget * parent)
    : m_dialog(NULL)
{
    qDebug() << Q_FUNC_INFO << parent;
}

QSettingLinkPc::~QSettingLinkPc (){
    qDebug() << "--->" << Q_FUNC_INFO;

    if (m_dialog)
    {
        Screen::getInstance()->queueUpdates();
        m_dialog->reject();
        delete m_dialog;
        m_dialog = NULL;
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();

        Storage::getInstance()->stopSharingOverUSB();
    }
    emit connectedToPc(false);
}

void QSettingLinkPc::linkPc () {

        qDebug() << Q_FUNC_INFO ;

        QBookApp::instance()->hideKeyboard();
        m_dialog = new InfoDialog(NULL, tr("Mass storage enabled.\nPlease don't extract or insert SD cards."));
        Screen::getInstance()->resetQueue();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        m_dialog->show();
        qDebug() << Q_FUNC_INFO << ": Dialog shown!!!!!!";
        QBookApp::instance()->syncModel();

        if (Storage::getInstance()->startSharingOverUSB()) {
            qDebug() << Q_FUNC_INFO << ": sharing over USB";
            emit connectedToPc(true);
        }else{
            Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
            if (m_dialog)
            {
                Storage::getInstance()->stopSharingOverUSB();

                m_dialog->reject();
                delete m_dialog;
                m_dialog = NULL;
            }
            emit connectedToPc(false);
        }
}

