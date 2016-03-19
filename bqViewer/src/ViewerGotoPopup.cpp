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

#include "ViewerGotoPopup.h"

#include "Viewer.h"
#include "QDocView.h"
#include "PowerManager.h"
#include "QBookApp.h"
#include "Screen.h"
#include "FastQPushButton.h"

#include <QDebug>
#include <QPainter>

ViewerGotoPopup::ViewerGotoPopup(Viewer* viewer) :
    ViewerMenuPopUp(viewer)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);

    // Signals
    connect(number0Btn, SIGNAL(clicked()), this, SLOT(handleNumber()), Qt::UniqueConnection);
    connect(number1Btn, SIGNAL(clicked()), this, SLOT(handleNumber()), Qt::UniqueConnection);
    connect(number2Btn, SIGNAL(clicked()), this, SLOT(handleNumber()), Qt::UniqueConnection);
    connect(number3Btn, SIGNAL(clicked()), this, SLOT(handleNumber()), Qt::UniqueConnection);
    connect(number4Btn, SIGNAL(clicked()), this, SLOT(handleNumber()), Qt::UniqueConnection);
    connect(number5Btn, SIGNAL(clicked()), this, SLOT(handleNumber()), Qt::UniqueConnection);
    connect(number6Btn, SIGNAL(clicked()), this, SLOT(handleNumber()), Qt::UniqueConnection);
    connect(number7Btn, SIGNAL(clicked()), this, SLOT(handleNumber()), Qt::UniqueConnection);
    connect(number8Btn, SIGNAL(clicked()), this, SLOT(handleNumber()), Qt::UniqueConnection);
    connect(number9Btn, SIGNAL(clicked()), this, SLOT(handleNumber()), Qt::UniqueConnection);
    connect(beginningBtn, SIGNAL(clicked()), this, SLOT(goToBeggining()), Qt::UniqueConnection);
    connect(endBtn,     SIGNAL(clicked()), this, SLOT(goToEnd()), Qt::UniqueConnection);

    connect(deleteCharBtn,  SIGNAL(released()), this, SLOT(handleDeleteOne()),    Qt::UniqueConnection);
    connect(closeBtn, SIGNAL(clicked()), this, SIGNAL(hideMe()));
    connect(goToBtn, SIGNAL(clicked()), this, SLOT(handlePage()));
    hide();
}

ViewerGotoPopup::~ViewerGotoPopup()
{
    qDebug() << Q_FUNC_INFO;
}

void ViewerGotoPopup::setup()
{}

void ViewerGotoPopup::start()
{}

void ViewerGotoPopup::stop()
{
    clearLineEdit();
}

void ViewerGotoPopup::goToPage(int pageNumber)
{
    qDebug() << Q_FUNC_INFO << "pageNumber" << pageNumber;    

    int totalPages = m_parentViewer->docView()->pageCount();

    if(pageNumber < 0 || (totalPages > 0 && pageNumber > totalPages))
        return;

    emit goPage(pageNumber);
}

//void ViewerGotoPopup::showEvent(QShowEvent *)
//{
//    // NOTE: Ugly hack piece of shit. We can't find a way to force the QLineEdit to grab the focus (setFocus does not work). :)
//    QApplication::postEvent(numberPageEdit, new QMouseEvent( QEvent::MouseButtonPress, numberPageEdit->pos(), Qt::LeftButton, Qt::NoButton, Qt::NoModifier));
//    QApplication::postEvent(numberPageEdit, new QMouseEvent( QEvent::MouseButtonRelease, numberPageEdit->pos(), Qt::LeftButton, Qt::NoButton, Qt::NoModifier));
//    // End NOTE
//}

void ViewerGotoPopup::handleNumber()
{
    qDebug() << Q_FUNC_INFO;
    FastQPushButton* button = (FastQPushButton*)sender();
    numberPageEdit->insert(button->text());
}

void ViewerGotoPopup::clearLineEdit()
{
    numberPageEdit->clear();
}

void ViewerGotoPopup::handlePage()
{
    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(true);
    Screen::getInstance()->flushUpdates();
    int pageNumber = numberPageEdit->text().toInt();

    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(false);
    emit hideMe();
    goToPage(pageNumber);
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_WAITFORCOMPLETION,Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void ViewerGotoPopup::handleDeleteOne()
{
    qDebug() << Q_FUNC_INFO;
    numberPageEdit->backspace();
}

void ViewerGotoPopup::goToBeggining()
{
    qDebug() << Q_FUNC_INFO;
    clearLineEdit();
    handlePage();
}

void ViewerGotoPopup::goToEnd()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(true);
    Screen::getInstance()->flushUpdates();
    int pageCount = m_parentViewer->docView()->pageCount() - 1;
    Screen::getInstance()->lockScreen();
    QCoreApplication::processEvents();
    Screen::getInstance()->releaseScreen();
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
    goToPage(pageCount);
    QBookApp::instance()->getStatusBar()->setSpinner(false);
    emit hideMe();
}

//void ViewerGotoPopup::paintEvent(QPaintEvent* )
//{
//    QStyleOption opt;
//    opt.init(this);
//    QPainter p(this);
//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
//}
