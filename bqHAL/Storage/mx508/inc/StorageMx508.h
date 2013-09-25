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

#ifndef STORAGEMX508_H
#define STORAGEMX508_H

#include "Storage.h"
class StoragePartition;

class StorageMx508 : public Storage
{

Q_OBJECT

public:

    StorageMx508();
    virtual ~StorageMx508();

    void init();
    void initAppData();

    virtual unsigned long long getFreeInternalMemory();
    virtual unsigned long long getFreePrivateMemory();
    virtual unsigned long long getFreeExternalMemory();
    virtual unsigned long long getTotalInternalMemory();
    virtual unsigned long long getTotalPrivateMemory();
    virtual unsigned long long getTotalExternalMemory();
    QString getMountsPath();
    StoragePartition* getPrivatePartition();
    StoragePartition* getPublicPartition();
    StoragePartition* getRemovablePartition();
    StoragePartition* getDataPartition();
        void formatStoragePartition(StoragePartition*);
    bool startSharingOverUSB();
    bool stopSharingOverUSB();
    bool restartSharingOverUSB();

private slots:
    void deviceUnshared(QString);

private:

    QStringList sharedDevices;
};

#endif // STORAGEMX508_H
