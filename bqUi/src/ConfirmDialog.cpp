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

#include <QDebug>
#include <QPainter>

#include "Screen.h"
#include "QBookApp.h"
#include "ConfirmDialog.h"
#include "PowerManagerDefs.h"
#include "ui_ConfirmDialog.h"

ConfirmDialog::ConfirmDialog(QWidget * parent, QString textStr, QString btnStr, Screen::ScreenMode mode):
    QDialog(parent,Qt::Popup | Qt:: Dialog)
{
    m_refreshMode = mode;
    setupUi(this);
    setText(textStr);
    setButtonText(btnStr);
    l_timeMilis = 0;

    m_powerLock = PowerManager::getNewLock(this);

    if (!QBookApp::instance()->userEventsBlocked())
        connect(ok,SIGNAL(clicked()),this,SLOT(doAccept()));
    else
        QTimer::singleShot(2000, this, SLOT(doAccept()));

    //Show dialog the time to going to sleep if the user dont accept.
    int timeToSleep = QBook::settings().value("setting/sleepTimeInSecs", POWERMANAGER_TIME_AUTOSLEEPSECS).toInt() * 1000;
    QTimer::singleShot(timeToSleep, this, SLOT(doAccept()));

    connect(QBookApp::instance(), SIGNAL(startSleep()), this, SLOT(reject()));
    connect(QBookApp::instance(), SIGNAL(powerKeyLongPressed()), this, SLOT(hideAndShutDown()));
    QBookApp::instance()->enablePowerKeyWatcher(true);
    QFile fileSpecific(":/res/ui_styles.qss");
    QFile fileCommons(":/res/ui_styles_generic.qss");
    fileSpecific.open(QFile::ReadOnly);
    fileCommons.open(QFile::ReadOnly);

    QString styles = QLatin1String(fileSpecific.readAll() + fileCommons.readAll());
    setStyleSheet(styles);
}

ConfirmDialog::~ConfirmDialog()
{
    if(m_powerLock)
    {
        delete m_powerLock;
        m_powerLock = NULL;
    }
}

void ConfirmDialog::setText(QString textStr)
{
    text->setText(textStr);
}

void ConfirmDialog::setButtonText(QString btnStr)
{
    ok->setText(btnStr);
}

void ConfirmDialog::doAccept(){
    QBookApp::instance()->enablePowerKeyWatcher(false);
    Screen::getInstance()->queueUpdates();
    accept();
    Screen::getInstance()->setMode(m_refreshMode,true, FLAG_PARTIALSCREEN_UPDATE|FLAG_WAITFORCOMPLETION,Q_FUNC_INFO);
    /* this processEvents is absolutely necessary to prevent glitches */
    QApplication::processEvents();
    Screen::getInstance()->flushUpdates();
}

void ConfirmDialog::setTimeLasts(long milis)
{
    l_timeMilis = milis;

    if(m_timer.isActive())
        showForSpecifiedTime(l_timeMilis);
}

/** To show it permanently until accept() is called, use exec() instead */
void ConfirmDialog::showForSpecifiedTime(long milis)
{
    if(milis <= 0) return;

    l_timeMilis = milis;
    m_powerLock->activate();
    if(m_timer.isActive())
        m_timer.stop();

    m_timer.start(l_timeMilis,this);
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_PARTIALSCREEN_UPDATE,Q_FUNC_INFO);
    exec();
    m_powerLock->release();
}

void ConfirmDialog::timerEvent(QTimerEvent *event)
{
    if ( event->timerId() == m_timer.timerId() )
    {
        m_timer.stop();
        event->accept();
        if(isVisible())
        {
            Screen::getInstance()->queueUpdates();
            accept();
            Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
            Screen::getInstance()->flushUpdates();
        }
    }
    else
    {
        QObject::timerEvent(event);
    }
}

void ConfirmDialog::paintEvent (QPaintEvent *)
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }

void ConfirmDialog::hideAndShutDown()
{
    qDebug() << Q_FUNC_INFO;
    QBookApp::instance()->enablePowerKeyWatcher(false);
    doAccept();
    QBookApp::instance()->shuttingDown();
}
