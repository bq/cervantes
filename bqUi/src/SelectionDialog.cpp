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

#include "SelectionDialog.h"
#include <QPainter>
#include "QBookApp.h"
#include "PowerManager.h"

#include "Screen.h"

SelectionDialog::SelectionDialog(QWidget * parent, QString textStr, QString okBtnTxt, QString cancelBtnText):
    QDialog(parent,Qt::Popup | Qt:: Dialog)
{
    setupUi(this);
    widget = NULL;
    setText(textStr);
    setOkBtnText(okBtnTxt);
    setCancelBtnText(cancelBtnText);
    connect(okBtn,SIGNAL(clicked()),this,SLOT(doAccept()));
    connect(cancelBtn,SIGNAL(clicked()),this,SLOT(doReject()));
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

void SelectionDialog::setText(QString textStr)
{
    textLbl->setText(textStr);
}

void SelectionDialog::setOkBtnText(QString okBtnTxt)
{
    okBtn->setText(okBtnTxt);
}

void SelectionDialog::setCancelBtnText(QString cancelBtnText)
{
    cancelBtn->setText(cancelBtnText);
}

void SelectionDialog::addWidget(QWidget *widget)
{
    this->widget = widget;
    horizontalLayout_2->addWidget(widget);
}

QWidget* SelectionDialog::getWidget()
{
    return widget;
}

void SelectionDialog::doReject(){
    QBookApp::instance()->enablePowerKeyWatcher(false);
    Screen::getInstance()->queueUpdates();
    reject();
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE || FLAG_WAITFORCOMPLETION,Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
    emit rejected();
}

void SelectionDialog::doAccept(){
    QBookApp::instance()->enablePowerKeyWatcher(false);
    Screen::getInstance()->queueUpdates();
    accept();
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true, FLAG_FULLSCREEN_UPDATE || FLAG_WAITFORCOMPLETION,Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
    emit accepted();
}


void SelectionDialog::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }

void SelectionDialog::hideAndShutDown()
{
    qDebug() << Q_FUNC_INFO;
    QBookApp::instance()->enablePowerKeyWatcher(false);
    doReject();
    QBookApp::instance()->shuttingDown();
}
