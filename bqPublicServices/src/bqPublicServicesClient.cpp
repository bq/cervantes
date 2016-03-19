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

#include "bqPublicServicesClient.h"

#include <QApplication>
#include <QDebug>
#include <QNetworkReply>
#include <QElapsedTimer>
#include <QStringList>
#include <QString>
#include <QByteArray>
#include <QVariant>
#include <QDir>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QXmlStreamReader>

bqPublicServicesClient::bqPublicServicesClient():
    downloadCanceled(false)
{
    qDebug() << "--->" << Q_FUNC_INFO;
}

bqPublicServicesClient::~bqPublicServicesClient()
{
    qDebug() << "--->" << Q_FUNC_INFO;
}

void bqPublicServicesClient::setDownloadCanceled(bool _downloadCanceled) {
    downloadCanceled = _downloadCanceled;
    m_requester.b_isDownloadCanceled = _downloadCanceled;
}

void bqPublicServicesClient::processSyncCanceled()
{
    downloadCanceled = false;
    m_requester.b_isDownloadCanceled = false;
}

QString bqPublicServicesClient::getWikipediaInfo(QString lang, QString wordToSearch, int section)
{
    qDebug() << Q_FUNC_INFO;

    if(downloadCanceled)
        return "";

    QString content;
    QNetworkReply* reply = m_requester.getWikipediaInfo(lang, wordToSearch, section);
    content = QString::fromUtf8 (reply->readAll());
    delete reply;

    QString info;
    QXmlStreamReader xml(content);
    while( !xml.atEnd() && !xml.hasError())
    {
        if(xml.name() == "section" && xml.attributes().value("id").toString().toUInt() == section)
        {
            info = xml.readElementText();
            break;
        }else
            xml.readNext();
    }
    return info;
}

QString bqPublicServicesClient::getWikipediaHeaders(QString lang, QString wordToSearch, int &sections)
{
    qDebug() << Q_FUNC_INFO << "wordToSearch: " << wordToSearch << " sections: " << sections;

    QString headers;
    QNetworkReply* reply = m_requester.getWikipediaHeaders(lang, wordToSearch);
    headers = QString::fromUtf8 (reply->readAll());
    delete reply;

    QString content;
    int currentNode = 0;
    int currentSubNode = 0;
    QXmlStreamReader xml(headers);
    while( !xml.atEnd() && !xml.hasError())
    {
        if(xml.name() == "section")
        {
            if(xml.attributes().value("id").toString().toUInt() == 0)
                content += xml.readElementText();
            else if(xml.attributes().value("toclevel").toString().toUInt() == 1)
            {
                currentNode++;
                currentSubNode = 0;
                sections++;
                content += "<li><p>&nbsp;&nbsp;" + QString(QString::number(currentNode) + ". ") + xml.attributes().value("line").toString() + "</p></li>";
            }
            else if(xml.attributes().value("toclevel").toString().toUInt() == 2)
            {
                sections++;
                currentSubNode++;
                content += "<li type=circle><pre><p>&nbsp;&nbsp;&nbsp;&nbsp;" + QString(QString::number(currentNode) + "." + QString::number(currentSubNode) + " ") + xml.attributes().value("line").toString() + "</p></pre></li>";
            }
            xml.readNext();
        }
        else
            xml.readNext();
    }

    return content;
}

QString bqPublicServicesClient::getWikipediaSearch(QString lang, QString wordToSearch)
{
    qDebug() << Q_FUNC_INFO << "wordToSearch: " << wordToSearch;

    QString searchResults;
    QString htmlText;
    QString title;
    QString desc;
    QString link;
    bool resultsFound = false;

    QNetworkReply* reply = m_requester.getWikipediaSearch(lang, wordToSearch);

    if(reply->error() == QNetworkReply::NoError)
    {
        searchResults = QString::fromUtf8 (reply->readAll());
        //    qDebug() << Q_FUNC_INFO << searchResults;
        delete reply;


        QXmlStreamReader xml(searchResults);
        while( !xml.atEnd() && !xml.hasError())
        {
            if(xml.name() == "Item")
            {
                resultsFound = true;
                while(xml.readNextStartElement()){

                    if(xml.name() == "Text")
                        title = xml.readElementText();
                    else if(xml.name() == "Description")
                        desc = xml.readElementText();
                    else if(xml.name() == "Url")
                        link = xml.readElementText().remove("http://" + lang + ".wikipedia.org");
                    else
                        xml.readElementText();
                }

                htmlText += "<li><a href=\"" + link + "\" title=\"" + title + "\">" + title + "</a>, " + desc + "</li>";
                xml.readNext();
            }
            else
            {
                xml.readNext();
            }
        }
        if(resultsFound)
            htmlText.prepend("<html><body><p>" + tr("Se han encontrado las siguientes paginas relacionadas con tu busqueda:") + "</p><ul>");
        else
            htmlText = "<html><body><p>" + tr("No se han encontrado paginas relacionadas con tu busqueda.") + "</p><ul>";
    }
    else // Network error
    {
        qDebug() << Q_FUNC_INFO << "Network error";
        htmlText = "<html><body><p>" + tr("Se ha producido un error de conexion y no se ha podido obtener información de wikipedia.") + "</p><ul>";
    }

    htmlText.append("</ul><div style=\"clear:both;\"></div><p><br></p><table style=\"\" class=\"noprint plainlinks ambox ambox-notice\"><tr><td class=\"ambox-image\"><div style=\"width:50px;\"><span class=\"mw-mf-image-replacement\"></span></td></tr></table></body></html>");
    return htmlText;
}
