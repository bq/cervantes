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

#include "LibraryBooksSubFilterLayer.h"

#include <QButtonGroup>
#include <QFile>
#include <QPainter>
#include <QDebug>
#include "Screen.h"

#define CHECKED "background-image: url(':/res/check.png')"
#define UNCHECKED "background-image:none;"


LibraryBooksSubFilterLayer::LibraryBooksSubFilterLayer(QWidget* parent) :
        QWidget(parent)
{
    setupUi(this);

    connect(allBooksBtn,            SIGNAL(clicked()), this, SLOT(allBooksClicked()));
    connect(newBooksBtn,            SIGNAL(clicked()), this, SLOT(newBooksClicked()));
    connect(activeBooksBtn,         SIGNAL(clicked()), this, SLOT(activeBooksClicked()));
    connect(readBooksBtn,           SIGNAL(clicked()), this, SLOT(readBooksClicked()));
    connect(sampleBooksBtn,         SIGNAL(clicked()), this, SLOT(sampleBooksClicked()));
    connect(purchasedBooksBtn,      SIGNAL(clicked()), this, SLOT(purchasedBooksClicked()));
    connect(subscriptionBooksBtn,   SIGNAL(clicked()), this, SLOT(subscriptionBooksClicked()));
    connect(archivedBtn,            SIGNAL(clicked()), this, SLOT(archivedStoreBooksClicked()));
    connect(archivedPremiumBtn,     SIGNAL(clicked()), this, SLOT(archivedPremiumBooksClicked()));

    m_allBooksName = allBooksBtn->text();
    m_newBooksName = newBooksBtn->text();
    m_activeBooksName = activeBooksBtn->text();
    m_readBooksName = readBooksBtn->text();
    m_sampleBooksName = sampleBooksBtn->text();
    m_purchasedBooksName = purchasedBooksBtn->text();
    m_subscriptionBooksName = subscriptionBooksBtn->text();
    m_archivedStoreBooksName = archivedBtn->text();
    m_archivedPremiumBooksName = archivedPremiumBtn->text();

    allBooksBtn->setStyleSheet(CHECKED);
}

LibraryBooksSubFilterLayer::~LibraryBooksSubFilterLayer()
{}

void LibraryBooksSubFilterLayer::paintEvent( QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void LibraryBooksSubFilterLayer::allBooksClicked ()
{
    Screen::getInstance()->queueUpdates();
    allBooksBtn->setStyleSheet(CHECKED);
    newBooksBtn->setStyleSheet(UNCHECKED);
    activeBooksBtn->setStyleSheet(UNCHECKED);
    readBooksBtn->setStyleSheet(UNCHECKED);
    sampleBooksBtn->setStyleSheet(UNCHECKED);
    purchasedBooksBtn->setStyleSheet(UNCHECKED);
    subscriptionBooksBtn->setStyleSheet(UNCHECKED);
    archivedBtn->setStyleSheet(UNCHECKED);
    archivedPremiumBtn->setStyleSheet(UNCHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit allBooksSelected();
}

void LibraryBooksSubFilterLayer::newBooksClicked ()
{
    Screen::getInstance()->queueUpdates();
    allBooksBtn->setStyleSheet(UNCHECKED);
    newBooksBtn->setStyleSheet(CHECKED);
    activeBooksBtn->setStyleSheet(UNCHECKED);
    readBooksBtn->setStyleSheet(UNCHECKED);
    sampleBooksBtn->setStyleSheet(UNCHECKED);
    purchasedBooksBtn->setStyleSheet(UNCHECKED);
    subscriptionBooksBtn->setStyleSheet(UNCHECKED);
    archivedBtn->setStyleSheet(UNCHECKED);
    archivedPremiumBtn->setStyleSheet(UNCHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit newBooksSelected();
}

void LibraryBooksSubFilterLayer::activeBooksClicked ()
{
    Screen::getInstance()->queueUpdates();
    allBooksBtn->setStyleSheet(UNCHECKED);
    newBooksBtn->setStyleSheet(UNCHECKED);
    activeBooksBtn->setStyleSheet(CHECKED);
    readBooksBtn->setStyleSheet(UNCHECKED);
    sampleBooksBtn->setStyleSheet(UNCHECKED);
    purchasedBooksBtn->setStyleSheet(UNCHECKED);
    subscriptionBooksBtn->setStyleSheet(UNCHECKED);
    archivedBtn->setStyleSheet(UNCHECKED);
    archivedPremiumBtn->setStyleSheet(UNCHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit activeBooksSelected();
}

void LibraryBooksSubFilterLayer::readBooksClicked()
{
    Screen::getInstance()->queueUpdates();
    allBooksBtn->setStyleSheet(UNCHECKED);
    newBooksBtn->setStyleSheet(UNCHECKED);
    activeBooksBtn->setStyleSheet(UNCHECKED);
    readBooksBtn->setStyleSheet(CHECKED);
    sampleBooksBtn->setStyleSheet(UNCHECKED);
    purchasedBooksBtn->setStyleSheet(UNCHECKED);
    subscriptionBooksBtn->setStyleSheet(UNCHECKED);
    archivedBtn->setStyleSheet(UNCHECKED);
    archivedPremiumBtn->setStyleSheet(UNCHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit readBooksSelected();
}


void LibraryBooksSubFilterLayer::sampleBooksClicked ()
{
    Screen::getInstance()->queueUpdates();
    allBooksBtn->setStyleSheet(UNCHECKED);
    newBooksBtn->setStyleSheet(UNCHECKED);
    activeBooksBtn->setStyleSheet(UNCHECKED);
    readBooksBtn->setStyleSheet(UNCHECKED);
    sampleBooksBtn->setStyleSheet(CHECKED);
    purchasedBooksBtn->setStyleSheet(UNCHECKED);
    subscriptionBooksBtn->setStyleSheet(UNCHECKED);
    archivedBtn->setStyleSheet(UNCHECKED);
    archivedPremiumBtn->setStyleSheet(UNCHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit sampleBooksSelected();
}

void LibraryBooksSubFilterLayer::purchasedBooksClicked ()
{
    Screen::getInstance()->queueUpdates();
    allBooksBtn->setStyleSheet(UNCHECKED);
    newBooksBtn->setStyleSheet(UNCHECKED);
    activeBooksBtn->setStyleSheet(UNCHECKED);
    readBooksBtn->setStyleSheet(UNCHECKED);
    sampleBooksBtn->setStyleSheet(UNCHECKED);
    purchasedBooksBtn->setStyleSheet(CHECKED);
    subscriptionBooksBtn->setStyleSheet(UNCHECKED);
    archivedBtn->setStyleSheet(UNCHECKED);
    archivedPremiumBtn->setStyleSheet(UNCHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit purchasedBooksSelected();
}

void LibraryBooksSubFilterLayer::subscriptionBooksClicked ()
{
    Screen::getInstance()->queueUpdates();
    allBooksBtn->setStyleSheet(UNCHECKED);
    newBooksBtn->setStyleSheet(UNCHECKED);
    activeBooksBtn->setStyleSheet(UNCHECKED);
    readBooksBtn->setStyleSheet(UNCHECKED);
    sampleBooksBtn->setStyleSheet(UNCHECKED);
    purchasedBooksBtn->setStyleSheet(UNCHECKED);
    subscriptionBooksBtn->setStyleSheet(CHECKED);
    archivedBtn->setStyleSheet(UNCHECKED);
    archivedPremiumBtn->setStyleSheet(UNCHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit subscriptionBooksSelected();
}

void LibraryBooksSubFilterLayer::archivedStoreBooksClicked ()
{
    Screen::getInstance()->queueUpdates();
    allBooksBtn->setStyleSheet(UNCHECKED);
    newBooksBtn->setStyleSheet(UNCHECKED);
    activeBooksBtn->setStyleSheet(UNCHECKED);
    readBooksBtn->setStyleSheet(UNCHECKED);
    sampleBooksBtn->setStyleSheet(UNCHECKED);
    purchasedBooksBtn->setStyleSheet(UNCHECKED);
    subscriptionBooksBtn->setStyleSheet(UNCHECKED);
    archivedPremiumBtn->setStyleSheet(UNCHECKED);
    archivedBtn->setStyleSheet(CHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit archivedStoreBooksSelected();
}

void LibraryBooksSubFilterLayer::archivedPremiumBooksClicked ()
{
    Screen::getInstance()->queueUpdates();
    allBooksBtn->setStyleSheet(UNCHECKED);
    newBooksBtn->setStyleSheet(UNCHECKED);
    activeBooksBtn->setStyleSheet(UNCHECKED);
    readBooksBtn->setStyleSheet(UNCHECKED);
    sampleBooksBtn->setStyleSheet(UNCHECKED);
    purchasedBooksBtn->setStyleSheet(UNCHECKED);
    subscriptionBooksBtn->setStyleSheet(UNCHECKED);
    archivedBtn->setStyleSheet(UNCHECKED);
    archivedPremiumBtn->setStyleSheet(CHECKED);
    Screen::getInstance()->setMode(Screen::MODE_FASTEST, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();

    emit archivedPremiumBooksSelected();
}

void LibraryBooksSubFilterLayer::setAllBooksChecked()
{
    allBooksBtn->setStyleSheet(CHECKED);
    newBooksBtn->setStyleSheet(UNCHECKED);
    activeBooksBtn->setStyleSheet(UNCHECKED);
    readBooksBtn->setStyleSheet(UNCHECKED);
    sampleBooksBtn->setStyleSheet(UNCHECKED);
    purchasedBooksBtn->setStyleSheet(UNCHECKED);
    subscriptionBooksBtn->setStyleSheet(UNCHECKED);
    archivedBtn->setStyleSheet(UNCHECKED);
    archivedPremiumBtn->setStyleSheet(UNCHECKED);
}

void LibraryBooksSubFilterLayer::setReadBooksEnabled( bool enabled )
{
    readBooksBtn->setEnabled(enabled);
}

void LibraryBooksSubFilterLayer::setNewBooksEnabled( bool enabled )
{
    newBooksBtn->setEnabled(enabled);
}

void LibraryBooksSubFilterLayer::setActiveBooksEnabled( bool enabled )
{
    activeBooksBtn->setEnabled(enabled);
}

void LibraryBooksSubFilterLayer::setSampleBooksEnabled( bool enabled )
{
    sampleBooksBtn->setEnabled(enabled);
}

void LibraryBooksSubFilterLayer::setSubscriptionBooksShown( bool shown )
{
    if(shown)
    {
        archivedPremiumBtn->show();
        subscriptionBooksBtn->show();
    }
    else
    {
        archivedPremiumBtn->hide();
        subscriptionBooksBtn->hide();
    }
}

void LibraryBooksSubFilterLayer::setPurchasedBooksEnabled( bool enabled )
{
    purchasedBooksBtn->setEnabled(enabled);
}

void LibraryBooksSubFilterLayer::setSubscriptionBooksEnabled( bool enabled )
{
    subscriptionBooksBtn->setEnabled(enabled);
}

void LibraryBooksSubFilterLayer::setStoreArchivedBooksEnabled( bool enabled )
{
    archivedBtn->setEnabled(enabled);
}

void LibraryBooksSubFilterLayer::setPremiumArchivedBooksEnabled( bool enabled )
{
    archivedPremiumBtn->setEnabled(enabled);
}

void LibraryBooksSubFilterLayer::setAllBooksNumber( int size )
{
    allBooksBtn->setText(m_allBooksName + " ("+ QString::number(size) + ")");
}

void LibraryBooksSubFilterLayer::setNewBooksNumber( int size )
{
    newBooksBtn->setText(m_newBooksName + " ("+ QString::number(size) + ")");
}

void LibraryBooksSubFilterLayer::setActiveBooksNumber( int size )
{
    activeBooksBtn->setText(m_activeBooksName + " ("+ QString::number(size) + ")");
}

void LibraryBooksSubFilterLayer::setReadBooksNumber( int size )
{
    readBooksBtn->setText(m_readBooksName + " ("+ QString::number(size) + ")");
}

void LibraryBooksSubFilterLayer::setSampleBooksNumber( int size )
{
    sampleBooksBtn->setText(m_sampleBooksName + " ("+ QString::number(size) + ")");
}

void LibraryBooksSubFilterLayer::setPurchasedBooksNumber( int size )
{
    purchasedBooksBtn->setText(m_purchasedBooksName + " ("+ QString::number(size) + ")");
}

void LibraryBooksSubFilterLayer::setSubscriptionBooksNumber( int size )
{
    subscriptionBooksBtn->setText(m_subscriptionBooksName + " ("+ QString::number(size) + ")");
}

void LibraryBooksSubFilterLayer::setStoreArchivedBooksNumber( int size )
{
    archivedBtn->setText(m_archivedStoreBooksName + " ("+ QString::number(size) + ")");
}

void LibraryBooksSubFilterLayer::setPremiumArchivedBooksNumber( int size )
{
    archivedPremiumBtn->setText(m_archivedPremiumBooksName + " ("+ QString::number(size) + ")");
}

QString LibraryBooksSubFilterLayer::getAllBooksName() const
{
    return allBooksBtn->text();
}

QString LibraryBooksSubFilterLayer::getNewBooksName() const
{
    return newBooksBtn->text();
}

QString LibraryBooksSubFilterLayer::getActiveBooksName() const
{
    return activeBooksBtn->text();
}

QString LibraryBooksSubFilterLayer::getReadBooksName() const
{
    return readBooksBtn->text();
}

QString LibraryBooksSubFilterLayer::getSampleBooksName() const
{
    return sampleBooksBtn->text();
}

QString LibraryBooksSubFilterLayer::getSubscriptionBooksName() const
{
    return subscriptionBooksBtn->text();
}

QString LibraryBooksSubFilterLayer::getPurchasedBooksName() const
{
    return purchasedBooksBtn->text();
}

QString LibraryBooksSubFilterLayer::getStoreArchivedBooksName() const
{
    return archivedBtn->text();
}

QString LibraryBooksSubFilterLayer::getPremiumArchivedBooksName() const
{
    return archivedPremiumBtn->text();
}

