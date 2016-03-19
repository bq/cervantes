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

#include "KeyboardButton.h"
#include "ui_KeyboardLayer.h"
#include "Screen.h"
#include "Keyboard.h"
#include "QBookApp.h"

#include <QDebug>
#include <QMouseEvent>
#include <QScreen>

KeyboardButton::KeyboardButton(QWidget* parent)
    : QLabel(parent)
{
    init();
}

KeyboardButton::KeyboardButton(QWidget *parent, QString text)
    : QLabel(parent)
{
    init();
    setText(text);
}

void KeyboardButton::setText(const QString text)
{
    QLabel::setText(text);

    QPixmap pix = QPixmap::grabWidget(this);
    viewInactive = pix.toImage().convertToFormat(QImage::Format_ARGB32);
    viewActive = QImage(viewInactive);
    viewActive.invertPixels();
}

void KeyboardButton::init()
{
    m_kbd = NULL;
    m_kbdLayer = NULL;
    m_ui_kbdLayer = NULL;
    b_special = false;
    b_pressStyle = true;  
    b_pressed = false;
    b_disableDehighlight = false;
    connect (&m_timer_longpress, SIGNAL(timeout()), this, SLOT(showLayer()));
}

KeyboardButton::~KeyboardButton()
{
    delete m_kbdLayer;
    m_kbdLayer = NULL;
    delete m_ui_kbdLayer;
    m_ui_kbdLayer = NULL;
}

void KeyboardButton::pressBtn()
{
    qDebug() << "--->" << Q_FUNC_INFO << text();

    if(!hasPressStyle())
        return;

    /* do nothing for special keys */
    if(isDehighlightDisabled())
        return;

    b_pressed = true;

    QBookApp::instance()->getKeyboard()->queueRefresh(viewActive,mapToGlobal(QPoint(0,0)));

}

void KeyboardButton::releaseBtn()
{
    qDebug() << "--->" << Q_FUNC_INFO << text();

    /* dehighlight the button later, to let it complete its highlight first */
    QTimer::singleShot(300, this, SLOT(dehighlightButton()));
}

void KeyboardButton::dehighlightButton()
{
    qDebug() << "--->" << Q_FUNC_INFO << text();

    if (!isVisible())
        return;

    /* do nothing for special keys */
    if(isDehighlightDisabled())
        return;

    if(!b_pressed)
        return;

    QBookApp::instance()->getKeyboard()->queueRefresh(viewInactive,mapToGlobal(QPoint(0,0)));

    b_pressed = false;
}

void KeyboardButton::resetButton()
{
    qDebug() << "--->" << Q_FUNC_INFO;

    releaseBtn();
    if(m_kbdLayer && m_kbdLayer->isVisible())
        m_kbdLayer->hide();
}

void KeyboardButton::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "--->" << Q_FUNC_INFO << "Event:" << event->type();
    pressBtn();
    emit pressed();

    if(m_altKeys.size()>0) {
        qDebug() << Q_FUNC_INFO << "SIZE" << m_altKeys.size();
        m_timer_longpress.start(800);

    }

    event->accept();
}

void KeyboardButton::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_timer_longpress.isActive())
        m_timer_longpress.stop();

    if(!m_kbdLayer || !m_kbdLayer->isVisible()) {
        releaseBtn();
        emit released();
    }

    event->accept();
}

void KeyboardButton::showLayer()
{
    if(!m_kbdLayer)
        return;

    /* Flush pending qt-updates, they shouldn't interfere with our layer */
    QCoreApplication::processEvents();
    QCoreApplication::flush();

    Screen::getInstance()->queueUpdates();

    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_PARTIALSCREEN_UPDATE,Q_FUNC_INFO);

    m_kbdLayer->raise();
    m_kbdLayer->show();

    /* Make sure the layer gets rendered before we try to read its dimension.
     * Otherwise incorrect values might result.
     * This doesn't produce output in itself, as all screen-updates are queued.
     */
    QCoreApplication::processEvents();
    QCoreApplication::flush();

    m_kbdLayer->adjustSize();

    // Move layer to proper position
    int newX, newY;

    // X
    // Key is on the right part. Adjust from right
    if(this->x() > (parentWidget()->width() / 2)) {

        newX = parentWidget()->width() -  m_ui_kbdLayer->keyboardLayerCont->width();

        // Layer too short, out of key. Move the layer until its above the key
        if(newX > x()) {
            newX = x();
        }

    } else { // No need to adjust from left

        // Layer too short, out of key. Move the layer until its above the key
        if((m_ui_kbdLayer->keyboardLayerCont->width() + m_kbdLayer->x()) < (x() + width())) {
            newX = x() + width() - m_ui_kbdLayer->keyboardLayerCont->width();
        } else
            newX = m_kbdLayer->x();
    }

    // Y
    newY = this->y() - m_ui_kbdLayer->keyboardLayerCont->height() + 2; // 2 px to avoid line overlapping

    // Move layer
    m_kbdLayer->move(newX,newY);

    // Adjust arrow.
    int xPositionInParent = x() + width() / 2;
    QPoint posInLayer = m_kbdLayer->mapFrom(parentWidget(),QPoint(xPositionInParent,y()));
    m_ui_kbdLayer->arrow_bottom->move(posInLayer.x(),posInLayer.y());

    /* set the style for the button,as the layer overwrites our hardcoded blit */
    setStyleSheet("font-size:25px;color:#FFFFFF;background-color:#000000;border:2px solid #000000;border-radius: 5px;");

    /* Unlock the queue and flush the updates to the screen */
    Screen::getInstance()->flushUpdates();
}

// Sets alt keys inside a KeyboardLayer widget
//
void KeyboardButton::setAltKeys(QStringList listTop,
                                QStringList listBottom,
                                BaseKeyboard* keyboard)
{
    m_altKeys.clear();

    if(listTop.isEmpty() && listBottom.isEmpty())
        return;

    if(!m_kbdLayer) {
        m_kbdLayer = new QWidget(parentWidget());
        m_ui_kbdLayer = new Ui::KeyboardLayer();
        m_ui_kbdLayer->setupUi(m_kbdLayer);
        m_kbdLayer->hide();
        connect(m_ui_kbdLayer->LayerCloseBtn,SIGNAL(clicked()),this,SLOT(hideLayer()));        
    }

    // Connection made everytime because keyboard signals are disconnected everytime is hidden
    connect(keyboard,SIGNAL(hideLayer()),this,SLOT(hideLayer()),Qt::UniqueConnection);
    connect(this, SIGNAL(released()), keyboard, SIGNAL(hideLayer()));

    // Top line
    if(listTop.size() > 0)
    {
        QList<KeyboardButton*> upLine =
                m_ui_kbdLayer->upLine->findChildren<KeyboardButton*>();

        for(int i = 0; i < upLine.size(); ++i)
        {
            upLine.at(i)->setDehighlightDisabled(true);
            if(i < listTop.size()) {
                upLine.at(i)->setText(listTop.at(i));
                m_altKeys.append(upLine.at(i));
                connect(upLine.at(i),SIGNAL(released()),this,SLOT(hideLayer()),Qt::UniqueConnection);
                connect(upLine.at(i),SIGNAL(released()), keyboard, SLOT(handleKey()), Qt::UniqueConnection);
            } else
                upLine.at(i)->hide();
        }
    } else {
        m_ui_kbdLayer->upLine->hide();
    }

    // Bottom line
    if(listBottom.size() > 0)
    {
        QList<KeyboardButton*> downLine =
                m_ui_kbdLayer->downLine->findChildren<KeyboardButton*>();

        for(int i = 0; i < downLine.size(); ++i)
        {
            downLine.at(i)->setDehighlightDisabled(true);
            if(i < listBottom.size()) {
                downLine.at(i)->setText(listBottom.at(i));
                m_altKeys.append(downLine.at(i));
                connect(downLine.at(i),SIGNAL(released()),this,SLOT(hideLayer()));
                connect(downLine.at(i),SIGNAL(released()), keyboard, SLOT(handleKey()), Qt::UniqueConnection);
            } else
                downLine.at(i)->hide();
        }
    } else {
        m_ui_kbdLayer->downLine->hide();
    }

    m_kbdLayer->hide();
}

void KeyboardButton::hideLayer()
{
    if(m_kbdLayer && m_kbdLayer->isVisible()) {
        Screen::getInstance()->queueUpdates();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
        m_kbdLayer->hide();

        /* reset the button style */
        QString fontSize;
        if(QBook::getResolution() == QBook::RES758x1024)
            fontSize = "30px";
        else
            fontSize = "25px";

        setStyleSheet("font-size:" + fontSize + ";color:#000000;background-color:#FFFFFF;border:2px solid #000000;border-radius: 5px;");
        Screen::getInstance()->flushUpdates();
    }
}
