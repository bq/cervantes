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

#include <QDebug>
#include <QObject>
#include <QApplication>
#include <QWSServer>

#include <Connect.h>
#include <ConnectionManager.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef Q_WS_QWS
    QWSServer::setCursorVisible(false);
    a.setOverrideCursor(QCursor(Qt::BlankCursor));
#endif

	ConnectionManager *cm = ConnectionManager::getInstance();
	if (!cm) {
		qDebug() << "Cannot initialize ConnectionManager. Aborting";
		QApplication::quit();
		return -1;
	}

	Connect *connect = new Connect(&a, cm);
	QTimer::singleShot(50, connect, SLOT(doWork()));

    return a.exec();
}




