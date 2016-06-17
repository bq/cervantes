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

#include "QBookEmu.h"
#include "ui_QBookEmu.h"

#include <QVBoxLayout>
#include <QActionGroup>
#include <QKeyEvent>

#include "QBookApp.h"
#include "QBook.h"
#include "ADConverter.h"
#include "Battery.h"
#include "BatteryEmu.h"
#include "Storage.h"


QBookEmu::QBookEmu(QWidget* parent)
    : QMainWindow(0, Qt::Window
        | Qt::MSWindowsFixedSizeDialogHint
        | Qt::WindowTitleHint
        | Qt::WindowMinimizeButtonHint
        | Qt::WindowCloseButtonHint)
    , m_ui(new Ui::QBookEmu())
{
    m_ui->setupUi(this);
    setWindowIconText("QBookApp");


    QVBoxLayout* layout = new QVBoxLayout(m_ui->centralwidget);
    layout->setMargin(0);
    layout->setSpacing(0);
    m_ui->centralwidget->setLayout(layout);

    QBookApp::staticInit();
    m_app = QBookApp::instance();

#ifndef Q_WS_QWS
    switch(QBook::getInstance()->getResolution()){
    case QBook::RES600x800:
        qDebug() << "********** Setting screen size to 825 (800 + menubar)";
        setFixedSize(600,825);
        m_ui->centralwidget->setFixedSize(600,800);
        break;
    case QBook::RES758x1024:
        qDebug() << "********* Setting screen size to 1049 (1024 + menubar)";
        setFixedSize(758,1049);
        m_ui->centralwidget->setFixedSize(758,1024);
        break;
    case QBook::RES1072x1448:
        qDebug() << "********* Setting screen size to 1473 (1448 + menubar)";
        setFixedSize(1072,1473);
        m_ui->centralwidget->setFixedSize(1072,1448);
        break;
    default:
        qWarning() << Q_FUNC_INFO << "UNKNOWN SCREEN SIZE";
    }
#endif

    layout->addWidget(m_app);

    connect(m_ui->menubar, SIGNAL(triggered(QAction*)), this, SLOT(handleAction(QAction*)));

    QActionGroup* battery = new QActionGroup(this);
    battery->addAction(m_ui->actionBattery0);
    battery->addAction(m_ui->actionBattery1);
    battery->addAction(m_ui->actionBattery2);
    battery->addAction(m_ui->actionBattery3);
    battery->addAction(m_ui->actionBatteryWarning);
    battery->addAction(m_ui->actionBatteryFull);

    QActionGroup* antenna = new QActionGroup(this);
    antenna->addAction(m_ui->actionAntenna0);
    antenna->addAction(m_ui->actionAntenna1);
    antenna->addAction(m_ui->actionAntenna2);
    antenna->addAction(m_ui->actionAntenna3);
    antenna->addAction(m_ui->actionAntenna4);
    antenna->addAction(m_ui->actionAntennaDisable);

    m_ui->actionBattery3->trigger();

    m_ui->actionMmc->setChecked(true);
    m_ui->actionCharger->setChecked(ADConverter::getInstance()->getStatus() != ADConverter::ADC_STATUS_NO_WIRE);
}

QBookEmu::~QBookEmu()
{
    QBookApp::staticDone();
    delete m_ui;
}

QBookApp* QBookEmu::appWidget()
{
    return m_app;
}

/*-------------------------------------------------------------------------*/

void QBookEmu::sendPressKey(int key)
{
    QWidget* focus = QApplication::focusWidget();
    if (!focus) focus = m_app;
    QKeyEvent* event = new QKeyEvent(QEvent::KeyPress, key, Qt::NoModifier);
    QApplication::postEvent(focus, event);
}

void QBookEmu::sendKey(int key)
{
    QWidget* focus = QApplication::focusWidget();
    if (!focus) focus = m_app;
    QKeyEvent* event = new QKeyEvent(QEvent::KeyRelease, key, Qt::NoModifier);
    QApplication::postEvent(focus, event);
}

void QBookEmu::handleAction(QAction* action)
{
    if (action == m_ui->actionQuit) {
	qDebug() << "Quit command not supported";
    } else if (action == m_ui->actionHome) {
        sendPressKey(QBook::QKEY_HOME);
        sendKey(QBook::QKEY_HOME);
    } else if (action == m_ui->actionMenu) {
        sendKey(QBook::QKEY_MENU);
    } else if (action == m_ui->actionPower) {
        sendKey(QBook::QKEY_POWER);
    } else if (action == m_ui->actionBattery0) {
	((BatteryEmu*)Battery::getInstance())->setBatteryLevel(10);
    } else if (action == m_ui->actionBattery1) {
	((BatteryEmu*)Battery::getInstance())->setBatteryLevel(20);
    } else if (action == m_ui->actionBattery2) {
	((BatteryEmu*)Battery::getInstance())->setBatteryLevel(50);
    } else if (action == m_ui->actionBattery3) {
	((BatteryEmu*)Battery::getInstance())->setBatteryLevel(100);
    } else if (action == m_ui->actionBatteryWarning) {
        ((BatteryEmu*)Battery::getInstance())->setBatteryLevel(9);
    } else if (action == m_ui->actionBatteryFull) {
        ((BatteryEmu*)Battery::getInstance())->setBatteryLevel(100);
    } else if (action == m_ui->actionAntennaDisable) {
	qDebug() << "Lost connection emulation not supported";
    } else if (action == m_ui->actionCharger) {
        if(ADConverter::getInstance()->getStatus() != ADConverter::ADC_STATUS_NO_WIRE){
            //TODO
        } else {
            //TODO
        }
        action->setChecked(ADConverter::getInstance()->getStatus() != ADConverter::ADC_STATUS_NO_WIRE);
        return;
    } else if (action == m_ui->actionMmc) {
	if (Storage::getInstance()->getRemovablePartition()->isMounted()) {
		Storage::getInstance()->umountStoragePartition(Storage::getInstance()->getRemovablePartition());
		action->setChecked(false);
	} else {
		Storage::getInstance()->mountStoragePartition(Storage::getInstance()->getRemovablePartition());
		action->setChecked(true);
	}
        return;
    } else if (action == m_ui->actionUsb) {
        //TODO
        return;
    } else if (action == m_ui->actionWakeup) {
        //Do Nothing
    } else if (action == m_ui->actionSleep) {
	qDebug() << "Sleep command not supported";
    } else if (action == m_ui->actionShutdown) {
        QApplication::quit();
    } else {
        return;
    }

    if (action->isCheckable()) action->setChecked(true);
}
