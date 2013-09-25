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

#ifndef QBOOKWEBFAVOMODEL_H
#define QBOOKWEBFAVOMODEL_H

/*-------------------------------------------------------------------------*/
#include <QtCore>
#include <QtGui>
#include <QtDebug>
#include <QDate>
#include <QTime>
#include "QBook.h"
/*-------------------------------------------------------------------------*/

class QBookWebFavoInfo
{
public:  
	QString eventLabel;
    QString eventUrl;
	bool check;
	int id;
    
    void fillData(int idNum, QString label, QString url) {

		eventLabel = label;
        eventUrl = url;
		check = false;
		id = idNum;
    }

    QString getEventLabel(){ return eventLabel;}
    QString getEventurl(){ return eventUrl;}
    bool    getCheck(){ return check;}
    int     getId(){ return id;}
};

Q_DECLARE_METATYPE(QBookWebFavoInfo);

/*-------------------------------------------------------------------------*/

class BrowserFavoModel : public QStringListModel
{
    Q_OBJECT

public:
    BrowserFavoModel(QObject *parent = 0);
    ~BrowserFavoModel();
    
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void clean();
    void addEventData(const QBookWebFavoInfo &);
	void replace(int, QBookWebFavoInfo&);
	void deleteSelect();
	static BrowserFavoModel *getModel(); 
	static int fileCount;
	int listCount();
    void addFavo(QString& url, QString& s);

	QBookWebFavoInfo getInfoData(int);
    
signals:


private:
    QList<QBookWebFavoInfo> *m_info;
    void modelClear();
	static QString path;

};

/*-------------------------------------------------------------------------*/

#endif // QBOOKWEBFAVOMODEL_H
