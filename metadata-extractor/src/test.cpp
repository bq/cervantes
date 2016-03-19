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

#include <QString>
#include <QDebug>
#include <MetaDataExtractor.h>
#include "EpubMetaDataExtractor.h"
#include <QStringList>


int main (int argc, char *argv[])
{
	int i;
	MetaDataExtractor extractor = MetaDataExtractor();
	for (i = 1; i < argc; i++) {
		QString filename = QString(argv[i]);
        qDebug() << Q_FUNC_INFO << "Extracting data for " << filename;
        QString title, author, publisher, description, format;
        QDateTime date;
        extractor.getMetaData(filename, title, author, publisher, date, description, format);
        QStringList css;
        if(format == "epub")
        {
            EpubMetaDataExtractor epubExtractor = EpubMetaDataExtractor();
            css = epubExtractor.extractCSS(filename);
        }
    }
}
