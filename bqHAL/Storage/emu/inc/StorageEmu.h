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

#ifndef STORAGEEMU_H
#define STORAGEEMU_H

#include <QObject>
#include <Storage.h>
#include <StoragePartition.h>

class StorageEmu : public Storage
{

Q_OBJECT

public:

    StorageEmu();

    virtual unsigned long long getFreeInternalMemory();
    virtual unsigned long long getFreePrivateMemory();
    virtual unsigned long long getFreeExternalMemory();
    QString getMountsPath();
    StoragePartition* getPrivatePartition();
    StoragePartition* getPublicPartition();
    StoragePartition* getRemovablePartition();
    StoragePartition* getDataPartition();
    bool startSharingOverUSB();
    bool stopSharingOverUSB();
    virtual bool restartSharingOverUSB();
    virtual unsigned long long getTotalInternalMemory();
    virtual unsigned long long getTotalPrivateMemory();
    virtual unsigned long long getTotalExternalMemory();
    void init();
    void initAppData();
    void umountStoragePartition(StoragePartition *partition);
    void mountStoragePartition(StoragePartition *partition);
    void formatStoragePartition(StoragePartition*);




};

#endif // STORAGEEMU_H
