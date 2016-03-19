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


#include "BrowserPageSlider.h"
#include "ui_BrowserPageSlider.h"
#include "QBookApp.h"
#include "MouseFilter.h"

/*-------------------------------------------------------------------------*/

BrowserPageSlider::BrowserPageSlider(QWidget *parent)
    : QFrame(parent)

{
    setupUi(this);

    connect(label, SIGNAL(clicked()), this, SIGNAL(buttonClicked()));
    connect(QBookApp::instance(),SIGNAL(swipe(int)),this,SLOT(handleSwipe(int)));
    connect(downPageBtn, SIGNAL(clicked()), this, SLOT(nextPage()));
    connect(upPageBtn,   SIGNAL(clicked()), this, SLOT(previousPage()));
    noteMode->setStyleSheet("background:none;");

    m_isShowPageOnly = false;
}

BrowserPageSlider::~BrowserPageSlider()
{
}

void BrowserPageSlider::setPage(int pos, int max)
{
    setPage(pos, pos, max);
}

void BrowserPageSlider::setPage(int start, int end, int max)
{
	m_start = qMax(0, start);
	m_end = qMax(0, end);
	m_count = qMax(1, max);

	updateInfo();

}

void BrowserPageSlider::nextPage()
{
    if (m_start > m_count - 1)
        return;

	emit nextPagePressed();
}

void BrowserPageSlider::previousPage()
{
    if (m_start < 0)
        return;
	emit previousPagePressed();
}

void BrowserPageSlider::handleSwipe(int direction)
{
    qDebug()<< Q_FUNC_INFO << "Direction" << direction;

    switch(direction)
    {
    case MouseFilter::SWIPE_R2L:
    case MouseFilter::SWIPE_D2U:
        nextPage();
        break;

    case MouseFilter::SWIPE_L2R:
    case MouseFilter::SWIPE_U2D:
        previousPage();
        break;

    default:
        //ignore
        break;
    }
}

void BrowserPageSlider::setShowPageOnly(bool on)
{
    m_isShowPageOnly = on;
}

void BrowserPageSlider::updateInfo()
{
	QString str;

    if (m_isShowPageOnly)
        str = QString("%1").arg(m_start + 1);
    else if (m_start == m_end)
        str = tr("%1 / %2").arg(m_start + 1).arg(m_count);
    else
        str = tr("%1-%2 / %3").arg(m_start + 1).arg(m_end + 1).arg(m_count);

    label->setText(str);
}

void BrowserPageSlider::setNoteMode(int mode)
{
	switch(mode)
	{
		case HANDWRITE:
            noteMode->setStyleSheet("background-image:url(:/allmark/allmark-hwrnote);");
			break;
		case HIGHLIGHT:
            noteMode->setStyleSheet("background-image:url(:/allmark/allmark-highlight);");
			break;
		case ORIGINAL:
            noteMode->setStyleSheet("background:none;");
			break;
	}
}
