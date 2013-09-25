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

#include "InfoDialog.h"
#include <QPainter>
#include "PowerManager.h"
#include "PowerManagerLock.h"
#include "Screen.h"
#include "QBookApp.h"

#include <QFile>
#include <QDialog>
#include <QTimerEvent>
#include <QBasicTimer>
#include <QDebug>
#include <QMouseEvent>
#include <QKeyEvent>

InfoDialog::InfoDialog(QWidget * parent, QString textStr, long lastMilis) :
    QDialog(parent,Qt::Popup | Qt:: Dialog)
  , m_horizontal(false)
{
    setupUi(this);
    text->setText(textStr);
    l_timeMilis = lastMilis;

    m_powerLock = PowerManager::getNewLock(this);

    QFile fileSpecific(":/res/ui_styles.qss");
    QFile fileCommons(":/res/ui_styles_generic.qss");
    fileSpecific.open(QFile::ReadOnly);
    fileCommons.open(QFile::ReadOnly);

    QString styles = QLatin1String(fileSpecific.readAll() + fileCommons.readAll());
    setStyleSheet(styles);

    //We need this only for the mass storage enabled dialog.
    connect(QBookApp::instance(), SIGNAL(powerKeyLongPressed()), this, SLOT(hideDialog()));
    QBookApp::instance()->enablePowerKeyWatcher(true);
}

InfoDialog::~InfoDialog()
{
    if(m_powerLock)
        delete m_powerLock;
}

void InfoDialog::setText( const QString& textStr)
{
    text->setText(textStr);
}

void InfoDialog::hideSpinner() {
    spinerLbl->hide();
}

void InfoDialog::setTimeLasts(long milis)
{
    l_timeMilis = milis;

    if(m_timer.isActive())
        showForSpecifiedTime();
}

/** To show it permanently until accept() is called, use exec() instead */
void InfoDialog::showForSpecifiedTime()
{

    m_powerLock->activate();
    if(m_timer.isActive())
        m_timer.stop();

    m_timer.start(l_timeMilis,this);
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_PARTIALSCREEN_UPDATE,Q_FUNC_INFO);
    exec();    
    m_powerLock->release();
}

void InfoDialog::timerEvent(QTimerEvent *event)
{
    if ( event->timerId() == m_timer.timerId() ) {
        m_timer.stop();
        event->accept();
        if(isVisible()){
            QBookApp::instance()->enablePowerKeyWatcher(false);
            Screen::getInstance()->queueUpdates();
            accept();
             Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
             Screen::getInstance()->flushUpdates();
        }
	qDebug() << "**** CLOSING DIALOG ***";
    } else {
        QObject::timerEvent(event);
    }
}

void InfoDialog::paintEvent (QPaintEvent *)
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void InfoDialog::mousePressEvent(QMouseEvent* event)
{
        qDebug() << Q_FUNC_INFO;
        event->accept();
}

void InfoDialog::mouseReleaseEvent(QMouseEvent* event)
{
        qDebug() << Q_FUNC_INFO;
        event->accept();
}

void InfoDialog::keyPressEvent(QKeyEvent* event)
{
        qDebug() << Q_FUNC_INFO;
        event->accept();
}

void InfoDialog::keyReleaseEvent(QKeyEvent* event)
{
        qDebug() << Q_FUNC_INFO;
        event->accept();
}

void InfoDialog::hideDialog()
{
        qDebug() << Q_FUNC_INFO;
        QBookApp::instance()->enablePowerKeyWatcher(false);
        hide();
        QBookApp::instance()->shuttingDown();
}
