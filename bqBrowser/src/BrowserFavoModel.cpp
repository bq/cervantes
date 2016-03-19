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

#include <QDir>
#include <QDebug>
#include <QFileInfoList>
#include <QXmlStreamReader>
#include <QStringRef>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "Storage.h"

#include "BrowserFavoModel.h"
#include "BrowserFavo.h"

int BrowserFavoModel::fileCount;
QString BrowserFavoModel::path;
/*-------------------------------------------------------------------------*/

BrowserFavoModel::BrowserFavoModel(QObject *parent)
    : QStringListModel(parent)
{
    fileCount = 0;
    m_info = new QList<QBookWebFavoInfo>;

#ifdef Q_WS_QWS
    path = Storage::getInstance()->getDataPartition()->getMountPoint() + "/webfavo.sqlite";
#endif

}

BrowserFavoModel::~BrowserFavoModel()
{
    delete m_info;
}

Q_GLOBAL_STATIC(BrowserFavoModel, WebFavoListModel);

bool BrowserFavoModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    qDebug() << Q_FUNC_INFO;
    if (index.isValid() && role == Qt::EditRole) {
        m_info->insert(index.row(), value.value<QBookWebFavoInfo>());

        emit dataChanged(index, index);
        return true;
    }

    return false;
}

QVariant BrowserFavoModel::data(const QModelIndex &index, int role) const
{
    qDebug() << Q_FUNC_INFO;
    if (!index.isValid())
        return QVariant();

    if (role == Qt::TextAlignmentRole) {
        return int(Qt::AlignRight | Qt::AlignVCenter);
    } else if (role == Qt::DisplayRole) {
        QVariant v;
        QBookWebFavoInfo eventInfo = m_info->at(index.row());

        v.setValue(eventInfo);
        return v;
    }

    return QVariant();
}

void BrowserFavoModel::modelClear()
{
	m_info->clear();
}

void BrowserFavoModel::clean()
{
    fileCount = 0;
    m_info->clear();
}

BrowserFavoModel *BrowserFavoModel::getModel()
{
    qDebug() << Q_FUNC_INFO;
    WebFavoListModel()->modelClear();
    fileCount = 0;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);

    if ( db.open() ) {
        QSqlQuery query;
        query.exec("create table webfavo(id INTEGER PRIMARY KEY, label varchar, url varchar)");
        query.exec("select * from webfavo order by id");

        while(query.next())
        {
            int id = query.value(0).toInt();
            QString label = query.value(1).toString();
            QString note = query.value(2).toString();

            qDebug() << Q_FUNC_INFO << "id = " << query.at() << ", label = " << label << ", note " << note ;

            QBookWebFavoInfo eventInfo;
            eventInfo.fillData(id, label, note);
            fileCount++;
            WebFavoListModel()->addEventData(eventInfo);
        }
    }
    db.close();
    return WebFavoListModel();
}

void BrowserFavoModel::addEventData(const QBookWebFavoInfo &eventInfo)
{
	m_info->insert(m_info->count(), eventInfo);
}

void BrowserFavoModel::replace(int index, QBookWebFavoInfo &replaceInfo)
{
    m_info->replace(index, replaceInfo);
}

void BrowserFavoModel::addFavo(QString& url, QString& s)
{
    qDebug() << Q_FUNC_INFO;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    db.open();

    QSqlQuery query;
    query.exec("create table webfavo(id INTEGER PRIMARY KEY, label varchar, url varchar)");

    if (s.length()==0)
    {
        query.prepare("insert into webfavo (label, url) "
                "values (?, ?)");
        query.addBindValue("Favorite");
        query.addBindValue(url);
        query.exec();
    }else{
        query.prepare("insert into webfavo (label, url) "
                "values (?, ?)");
        query.addBindValue(s);
        query.addBindValue(url);
        query.exec();
    }
}

void BrowserFavoModel::deleteSelect()
{
    qDebug() << Q_FUNC_INFO;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    if ( db.open() ) {
        int number = listCount();
        for ( int i = 0 ; i < number ; i++ ) {
            if ( this->m_info->at(i).check == true ) {
                int id = this->m_info->at(i).id;

                QSqlQuery query;
                query.prepare("delete from webfavo where id = (?)");
                query.addBindValue(id);
                query.exec();
                //query.exec("delete from calendar where id = ");
            }
        }
    }
    db.close();
}

QBookWebFavoInfo BrowserFavoModel::getInfoData(int i){

	QBookWebFavoInfo favoinfo = this->m_info->at(i);
	return favoinfo;
}

int BrowserFavoModel::listCount()
{
	return m_info->count();
}
