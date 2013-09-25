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

#include "LibraryPageSlider.h"
#include "Screen.h"

#include <QAbstractSlider>
#include <QMouseEvent>
#include <QDebug>
#include <QCursor>

#define PRESSED   "QSlider::handle:horizontal {background: none;}"
#define UNPRESSED "QSlider::handle:horizontal:hover {background: url(:/res/slider_indicator.png) no-repeat center;}"

LibraryPageSlider::LibraryPageSlider(QWidget* parent) : QSlider(parent),
    b_sliderPressed(false), b_sliderMoved(false), m_sliderPage(0), m_sliderValue(0), m_currentPage(0), m_totalPages(0), m_initialPage(0),
    originalStyle("")
{
    QSlider::setTickPosition(QSlider::TicksAbove);
    connect(this, SIGNAL(sliderPressed()),  this,  SLOT(sliderPressedSlot()));
    originalStyle = styleSheet();
}

LibraryPageSlider::~LibraryPageSlider()
{
}

void LibraryPageSlider::initSlider(int _totalPages)
{
    qDebug() << Q_FUNC_INFO;
    m_totalPages = _totalPages;
}

void LibraryPageSlider::mousePressEvent(QMouseEvent* event)
{
    qDebug() << Q_FUNC_INFO;
    viewSlideHandler(true);
    m_previousRefresh = Screen::getInstance()->getCurrentMode();
    m_initialPage = m_currentPage;
    emit sliderPressed();
    QSlider::mousePressEvent(event);
}

void LibraryPageSlider::sliderPressedSlot()
{
    qDebug() << Q_FUNC_INFO;
    QPoint localMousePos = mapFromGlobal(QCursor::pos());
    bool clickOnSlider = (localMousePos.x() >= 0 && localMousePos.y() >= 0 &&
                          localMousePos.x() < size().width() && localMousePos.y() < size().height());
    if (clickOnSlider)
    {
        float posRatio = localMousePos.x() / (float)size().width();
        int sliderRange = maximum() - minimum();
        int sliderPosUnderMouse = minimum() + sliderRange * posRatio;
        m_sliderPage = float((sliderPosUnderMouse * m_totalPages) / float(100.0));
        m_sliderValue = sliderPosUnderMouse;
        b_sliderPressed = true;
    }
}

void LibraryPageSlider::mouseMoveEvent(QMouseEvent* event)
{
    qDebug() << Q_FUNC_INFO;
    b_sliderMoved = true;

    int page;
    if(b_sliderPressed)
    {
        page = m_sliderPage;
        b_sliderPressed = false;
    }
    else
        page = float((value() * m_totalPages) / float(100.0));

    Screen::getInstance()->setMode(Screen::MODE_QUICK, true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
    m_currentPage = page;
    emit sliderMoved(page);
    setValue(m_sliderValue);
    QSlider::mouseMoveEvent(event);
}

void LibraryPageSlider::mouseReleaseEvent(QMouseEvent* event)
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    Screen::getInstance()->setMode(m_previousRefresh,false,Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE,false);

    if(!b_sliderMoved)
        m_currentPage = m_sliderPage;

    qDebug() << Q_FUNC_INFO << " m_currentPage: " << m_currentPage;
    b_sliderPressed = false;
    b_sliderMoved = false;

    qDebug() << Q_FUNC_INFO << "SliderPage: " << m_sliderPage << "m_currentPage: " << m_currentPage << " m_initialPage: " << m_initialPage;
    if(m_initialPage == m_currentPage && m_totalPages > 1)
        setValue((100 / (m_totalPages - 1)) * m_currentPage);
    else
        emit jumpSliderPage(m_currentPage);

    QSlider::mouseReleaseEvent(event);
    viewSlideHandler(false);
    Screen::getInstance()->flushUpdates();
}

void LibraryPageSlider::updateSliderPosition( int currentPage)
{
    qDebug() << Q_FUNC_INFO << " m_totalPages: " << m_totalPages << "; currentPage: " << currentPage;
    double interval = 100;
    if(m_totalPages > 1)
        interval = interval / (m_totalPages - 1);

    setValue(interval * currentPage);
}

void LibraryPageSlider::viewSlideHandler(bool pressed)
{
    qDebug() << Q_FUNC_INFO;
    if(pressed)
    {
        Screen::getInstance()->queueUpdates();
        setStyleSheet(PRESSED);
        Screen::getInstance()->setMode(Screen::MODE_FAST, true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
        Screen::getInstance()->flushUpdates();
    }
    else
        setStyleSheet(originalStyle);

}
