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

#include "PdfMetaDataExtractor.h"

#include <QString>
#include <QDebug>
#include <QProcess>

void copyStringWithoutControlChars( QString& ori, QString& dst )
{
    ori.remove(QChar::fromAscii(0x00));
    ori.remove(QChar::fromAscii(0x01));
    ori.remove(QChar::fromAscii(0x02));
    ori.remove(QChar::fromAscii(0x03));
    ori.remove(QChar::fromAscii(0x04));
    ori.remove(QChar::fromAscii(0x05));
    ori.remove(QChar::fromAscii(0x06));
    ori.remove(QChar::fromAscii(0x07));
    ori.remove(QChar::fromAscii(0x08));
    ori.remove(QChar::fromAscii(0x09));
    // Do not remove 0x0A LINE FEED
    ori.remove(QChar::fromAscii(0x0B));
    ori.remove(QChar::fromAscii(0x0C));
    // Do not remove 0x0D CARRIAGE RETURN
    ori.remove(QChar::fromAscii(0x0E));
    ori.remove(QChar::fromAscii(0x0F));
    ori.remove(QChar::fromAscii(0x1A));
    ori.remove(QChar::fromAscii(0x1B));
    ori.remove(QChar::fromAscii(0x1C));
    ori.remove(QChar::fromAscii(0x1D));
    ori.remove(QChar::fromAscii(0x1E));
    ori.remove(QChar::fromAscii(0x1F));
    dst = ori;
}

/*static*/ bool PdfMetaDataExtractor::getMetaData( const QString& filename, QString& title, QString& author, QString& publisher, QDateTime& date, QString&, QString& language )
{
    QString* _title = NULL;
    QString* _author = NULL;
    QString* _publisher = NULL;
    QString* _date = NULL;

    QProcess *proc = new QProcess();
	QStringList arguments;
	arguments << "-meta" << filename;
	proc->start("pdfinfo", arguments);

    if (!proc->waitForFinished(10000)) {
		delete proc;
		return false;
	}

	QByteArray output = proc->readAllStandardOutput();
	QTextStream in(output);
	in.setAutoDetectUnicode(true);
	while (!in.atEnd()) {
		QString line = in.readLine();
        if (line.startsWith("Title: "))
            _title = new QString(line.mid(strlen("Title: ")).trimmed());
        else if (line.startsWith("Author: "))
            _author = new QString(line.mid(strlen("Author: ")).trimmed());
        else if (line.startsWith("Creator: "))
            _publisher = new QString(line.mid(strlen("Creator: ")).trimmed());
        else if (line.startsWith("CreationDate: "))
            // Fixme ensure we return a ISO8601 string
            _date = new QString(line.mid(strlen("CreationDate: ")).trimmed());
        if (_title && _author && _publisher && _date)
			break;
	}

    if(_title)
        copyStringWithoutControlChars(*_title, title);
    if(_author)
        copyStringWithoutControlChars(*_author, author);
    if(_publisher)
        copyStringWithoutControlChars(*_publisher, publisher);
    if(_date)
        date = QDateTime::fromString(*_date, Qt::ISODate);

    delete _title;
    delete _author;
    delete _publisher;
    delete _date;

	delete proc;
	return true;
}
