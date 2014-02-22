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

#include "bqPublicServices.h"
#include "bqPublicServicesClient.h"

#include <QDebug>
#include <QDir>


bqPublicServices::bqPublicServices()
{
    qDebug() << "--->" << Q_FUNC_INFO;
    connect(&m_client, SIGNAL(downloadProgress(int)),   this, SIGNAL(downloadProgress(int)));
    connect(&m_client, SIGNAL(hideProgressBar()),       this, SIGNAL(hideProgressBar()));
    connect(&m_client, SIGNAL(notEnoughtSpace()),       this, SIGNAL(notEnoughtSpace()));
}

bqPublicServices::~bqPublicServices()
{
    qDebug() << "--->" << Q_FUNC_INFO;
}

void bqPublicServices::cancelDownload()
{
    m_client.setDownloadCanceled(true);
}

void bqPublicServices::processSyncCanceled()
{
    m_client.processSyncCanceled();
}

QString bqPublicServices::getWikipediaWord(QString lang, QString text)
{
    qDebug() << Q_FUNC_INFO;
    int sections = 0;

    QString textToSearch = text.replace(" ","_");

    QString wikipediaSearch = m_client.getWikipediaHeaders(lang, textToSearch, sections) + "\n\n";
    int progressValue = 10;
    int progressCalculated = 0;
    emit downloadProgress(progressValue);

    for (int i = 1; i <= sections; i++)
    {
        wikipediaSearch+= m_client.getWikipediaInfo(lang, textToSearch, i);
        wikipediaSearch+= "\n";
        progressCalculated = int((float(i) / float(sections)) * 0.9 * 100);
        emit downloadProgress(progressValue + progressCalculated);
    }

    processSyncCanceled();

    if(wikipediaSearch.trimmed().isEmpty())
        wikipediaSearch = m_client.getWikipediaSearch(lang,textToSearch);

//    qDebug() << Q_FUNC_INFO << wikipediaSearch;

    return wikipediaSearch;
}

