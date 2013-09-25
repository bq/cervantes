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

#ifndef QBOOKPAGESLIDER_H
#define QBOOKPAGESLIDER_H

#include "ui_BrowserPageSlider.h"

#include <QFrame>
#include "QBook.h"


/*-------------------------------------------------------------------------*/

class BrowserPageSlider : public QFrame, protected Ui::BrowserPageSlider
{
	Q_OBJECT

public:
    BrowserPageSlider(QWidget *parent = 0);
    ~BrowserPageSlider();

    void setShowPageOnly(bool on = false);
	void setNoteMode(int mode);

	enum {ORIGINAL, HANDWRITE, HIGHLIGHT};

public slots:
	void setPage(int pos, int max);
	void setPage(int start, int end, int max);
    void nextPage();
    void previousPage();
    void handleSwipe(int direction);

signals:
	void nextPagePressed();
	void previousPagePressed();
    void buttonClicked();

protected:
	void updateInfo();

private:

    bool m_isShowPageOnly;
	int m_count;
	int m_start;
    int m_end;
};

#endif
