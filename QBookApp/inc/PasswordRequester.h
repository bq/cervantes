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

#ifndef __PASSWORDREQUESTER__
#define __PASSWORDREQUESTER__

//#include "QBookApp.h"
#include <QObject>
#include "ConnectionManager.h"

class PasswordRequester : public QObject {

Q_OBJECT

public:
	PasswordRequester(ConnectionManager *m_cm, void *parent);
	~PasswordRequester();
	QString *askPassword(QString);


private:
	ConnectionManager *m_cm;
	QString *text;
	bool gotPassword;
	void *m_parent;
	QString m_path;

private slots:
	void passwordDialogOk(QString);
	void agentReleased();
        void registerAgent();

public slots:

    void passwordDialogCancel();

signals:
	void passwordRequested();
	void passwordSent();
	void wrongPassword(QString);
	void cancelPassword();
	void connectionFailed(QString,QString);
};



#endif
