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

#ifndef QBOOKWEBFAVO_H
#define QBOOKWEBFAVO_H

#include "FullScreenWidget.h"
#include "ui_BrowserFavo.h"
#include <QDate>

// Predeclarations
class BrowserFavoItem;
class BrowserFavoModel;
class QKeyEvent;

class BrowserFavo : public FullScreenWidget, protected Ui::BrowserFavo
{
    Q_OBJECT

public:
    BrowserFavo(QWidget* parent = 0);
    virtual ~BrowserFavo();

    int countSelect() const;
    void setup();
    void addFavo(QString& url, QString& s);
    bool isAdded   (QString& url);


    static bool isSelectAll;

public slots:

    void deleteBtnClicked();
    void selectBtnClicked();
    void handlemyfavo(QString);
    void handlselectbtn();
    void nextPage();
    void previousPage();

signals:
    void myfavoclick(QString Urlvalue);

protected:
    void                                    setFavoItems                        ( );
    void                                    getModel                            ( );

private:

	int eventListType;
	bool addToEmptyDate;
	QDate oneDay;
    QList<BrowserFavoItem*>                      items;
    BrowserFavoModel *m_model;
    int m_page;
    int m_totalPages;



protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    void                paintEvent                      (QPaintEvent *);
};

#endif // QBOOKWEBFAVO_H
