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

#include "MyAgent.h"

#define NUM_RETRIES 3

MyAgent::MyAgent(QObject *parent) {
	passwordRequester = NULL;
    b_wrongPassSignalSent = false;
	tries = 0;
}

void MyAgent::Cancel() {
	qDebug() << "MyAgent::Cancel";
}

void MyAgent::Release() {
	qDebug() << "MyAgent::Release";
	emit released();
}

void MyAgent::ReportError(const QDBusObjectPath &path, const QString &error, const QDBusMessage &message) {
	qDebug() << "Error for object: " << path.path();
	qDebug() << "error message: " << error;
	if (error == "invalid-key") {
		qDebug() << "Wrong password";
        	if(!b_wrongPassSignalSent) {
            		b_wrongPassSignalSent = true;
            		emit wrongPassword(path.path());
        	} else {
            		qDebug() << "--->" << Q_FUNC_INFO << "ERROR: invalid-key received twice. This shouldn't happen.";
        	}
	} else {
        /*qDebug() << "Connection failed: " << error;
		if (tries < NUM_RETRIES) {
			qDebug() << "sending connman a retry reply";
			QString replyMsg;
			replyMsg.append("Retry ");
			replyMsg.append(tries);
			replyMsg.append(" of ");
			replyMsg.append(NUM_RETRIES);
               		QDBusMessage error = message.createErrorReply("net.connman.Agent.Error.Retry", replyMsg);
			QDBusConnection::systemBus().send(error);
			tries++;
        } else {*/
			qDebug() << "Agent got a connect-fail error";
			emit connectionFailed(path.path(), error);
			tries = 0;
    }
}


void MyAgent::RequestBrowser(const QDBusObjectPath &path, const QString &url) {
}

void MyAgent::setPasswordRequester(PasswordRequester *requester) {
	if (passwordRequester)
		delete passwordRequester;
	passwordRequester = requester;
}


struct RequestData
 {
     QString request;
     QString processedData;
     QDBusMessage reply;
 };


QVariantMap MyAgent::RequestInput(const QDBusObjectPath &path, const QVariantMap &fields, const QDBusMessage &message) {
	tries = 0;
    b_wrongPassSignalSent = false;
	qDebug() << "Requesting input for path:" << path.path();
	QVariantMap v(fields);
	foreach (QString k, v.keys()) {
		qDebug() << Q_FUNC_INFO << "Key: " << k;
		QDBusArgument a = v[k].value<QDBusArgument>();
		QVariantMap args;
		a >> args;
		foreach(QString arg, args.keys()) {
			qDebug() << Q_FUNC_INFO << " arg: " << arg << "value: " << args[arg].toString();
		}
	}

	qDebug() << "Requested fields:" << fields;
	QVariantMap result;
	if (fields.contains("Passphrase")) {
		qDebug() << "REQUEST FOR PASSPHRASE";
		if (passwordRequester) {
			qDebug() << "calling passwordRequester askPassword";
			QString *password = passwordRequester->askPassword(path.path());
			if (password == NULL) {
				result["Error"] = "User cancel password input";
                		QDBusMessage error = message.createErrorReply("net.connman.Error.Canceled", "User cancel");
				QDBusConnection::systemBus().send(error);
			} else { 
				qDebug() << "MyAgent::RequestInput got password, setting to the reply: " << *password;
				result["Passphrase"] = QVariant(QString(*password));
				delete password;
			}
		} else {
			qDebug() << "No passwordRequester available";
		}
	}
	qDebug() << "MyAgent::RequestInput: returning map to connman: " << result;
	return result;
}

