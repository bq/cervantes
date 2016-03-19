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

#ifndef QBOOKAPPSERVICE_H
#define QBOOKAPPSERVICE_H

#include <QDBusAbstractAdaptor>
#include "QBook.h"
#include <stdio.h>

class QBookAppService : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.bqreaders.QBookAppService")

    public:
        QBookAppService(QObject *parent) : QDBusAbstractAdaptor(parent) { }
        virtual ~QBookAppService() {}
    public slots:
        Q_NOREPLY void setDebug(QString debug) { printf("LLAMADA A setDebug"); QBook::setDebug(debug); }
};

#endif // QBOOKAPPSERVICE_H
