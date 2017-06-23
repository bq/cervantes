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

#include "AfterUpdateWorker.h"
#include "QBook.h"
#include "Storage.h"
#include "bqUtils.h"
#include <QFile>
#include <QDir>
#include <QDebug>

/** INITIALIZATION **/
AfterUpdateWorker* AfterUpdateWorker::m_instance = NULL;


AfterUpdateWorker::AfterUpdateWorker()
{
}

AfterUpdateWorker::~AfterUpdateWorker()
{
    // Do nothing
}

// Double checked locking + Singleton pattern.
AfterUpdateWorker* AfterUpdateWorker::getInstance()
{
    if(!AfterUpdateWorker::m_instance)
        m_instance = new AfterUpdateWorker();

    return m_instance;
}


void AfterUpdateWorker::work()
{
    QFile isAfterUpdate("./afterUpdate");
    if(!isAfterUpdate.exists())
        return;

    /**
    * Call here you would like to do after OTA
    */

    copyNewImages();

    checkShopMigration();

    setActivated();

    removeExpiredDictionaries();

    system("./rmdeprecatedfiles.sh");

    isAfterUpdate.remove();

}

void AfterUpdateWorker::checkShopMigration()
{
    qDebug() << Q_FUNC_INFO;

    const QString shopName = QBook::settings().value("shopName", "").toString();

    if(shopName.size() && (shopName == "bq" || shopName == "liberdrac" || shopName == "fnac"))
    {

        qDebug() << Q_FUNC_INFO << "unlinking device";

        // Clean Settings
        QBook::settings().setValue("setting/linked",false);
        QBook::settings().setValue("setting/activated",false);
        QBook::settings().setValue("setting/initial",true);
        QBook::settings().setValue("eMail", "");
        QBook::settings().setValue("shopName", "Tienda");
        QBook::settings().setValue("readerPartitionName", "reader");
        QBook::settings().setValue("deviceModelName", "reader");

        // Remove customized images
        QString customizedImagesPath = Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator();

        if(QFile::exists(customizedImagesPath + QString("shop_main_menu.png")))
            QFile::remove(customizedImagesPath + QString("shop_main_menu.png"));

        if(QFile::exists(customizedImagesPath + QString("powerOffDeviceImage.png")))
            QFile::remove(customizedImagesPath + QString("powerOffDeviceImage.png"));

        if(QFile::exists(customizedImagesPath + QString("restDeviceImage.png")))
            QFile::remove(customizedImagesPath + QString("restDeviceImage.png"));

        if(QFile::exists(customizedImagesPath + QString("startDeviceImage.png")))
            QFile::remove(customizedImagesPath + QString("startDeviceImage.png"));

        if(QFile::exists(customizedImagesPath + QString("updateDeviceImage.png")))
            QFile::remove(customizedImagesPath + QString("updateDeviceImage.png"));

        if(QFile::exists(customizedImagesPath + QString("lowBatteryImage.png")))
            QFile::remove(customizedImagesPath + QString("lowBatteryImage.png"));
    }
}

void AfterUpdateWorker::copyNewImages()
{
    QString imagePath = "../res/newImages";
    QString shopName = QBook::settings().value("shopName", "").toString();

    QString resolution;
    QString scriptName;

    switch (QBook::getInstance()->getResolution())
    {
        case QBook::RES1072x1448:
            resolution = "1448";
            scriptName = "./changeStartingImage1448.sh";
            break;
        case QBook::RES758x1024:
            resolution = "1024";
            scriptName = "./changeStartingImage1024.sh";
            break;
        case QBook::RES600x800: default:
            resolution = "800";
            scriptName = "./changeStartingImage800.sh";
            break;
    }

    /** In order to replace customization images in initial versions (before 4.1.1)
        it is necessary to ensure update of image for the shops below
    */
    if(shopName == "bq" || shopName == "elkar" || shopName == "fnac" || shopName == "liberdrac" || shopName == "nubico"){


        QFile powerOffDeviceImage(imagePath + "/powerOffDeviceImage" + "_" +shopName + "_" + resolution + ".png");
        if(powerOffDeviceImage.exists()){
            QFile::remove(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("powerOffDeviceImage.png"));
            qDebug() << "Replacing powerOffDeviceImage.png" << powerOffDeviceImage.copy(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("powerOffDeviceImage.png"));
        }

        QFile restDeviceImage(imagePath +"/restDeviceImage" + "_" +shopName + "_" + resolution + ".png");
        if(restDeviceImage.exists()){
            QFile::remove(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("restDeviceImage.png"));
            qDebug() << "Replacing restDeviceImage.png" << restDeviceImage.copy(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("restDeviceImage.png"));
        }

        QFile updateDeviceImage(imagePath +"/updateDeviceImage" + "_" +shopName + "_" + resolution + ".png");
        if(updateDeviceImage.exists()){
            QFile::remove(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("updateDeviceImage.png"));
            qDebug() << "Replacing updateDeviceImage.png" << updateDeviceImage.copy(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("updateDeviceImage.png"));
        }

        QFile lowBatteryImage(imagePath +"/lowBatteryImage" + "_" +shopName + "_" + resolution + ".png");
        if(lowBatteryImage.exists()){
            QFile::remove(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("lowBatteryImage.png"));
            qDebug() << "Replacing lowBatteryImage.png" << lowBatteryImage.copy(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("lowBatteryImage.png"));
        }

        QFile startDeviceImage(imagePath +"/startDeviceImage" + "_" +shopName + "_" + resolution + ".png");
        if(startDeviceImage.exists()){
            QFile::remove(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("startDeviceImage.png"));
            qDebug() << "Replacing startDeviceImage.png" << startDeviceImage.copy(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("startDeviceImage.png"));
        }
    }

    ////// WARNING This Script execute dd ///////////////

    system(scriptName.toUtf8().constData());

    /////////////////////////////////////////////////////

    qDebug() << "Removed new images:" << bqUtils::removeDir(imagePath);
}

void AfterUpdateWorker::setActivated()
{
    if(QBook::settings().value("setting/linked", false).toBool())
        QBook::settings().setValue("setting/activated", true);
}

void AfterUpdateWorker::removeExpiredDictionaries()
{
    qDebug() << Q_FUNC_INFO;

    if(QFile::exists(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("dictionaries/2c8683d073c8fdc019a24ffbaf51a2da.dix")))
        qDebug() << Q_FUNC_INFO << "remove 1:" << QFile::remove(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("dictionaries/2c8683d073c8fdc019a24ffbaf51a2da.dix"));

    if(QFile::exists(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("dictionaries/380f1042f0376649c59f83fd4b9fb0e0.dix")))
        qDebug() << Q_FUNC_INFO << "remove 2:" <<  QFile::remove(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("dictionaries/380f1042f0376649c59f83fd4b9fb0e0.dix"));

    if(QFile::exists(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("dictionaries/f1b2938873d665eecee63e39a68862a5.dix")))
        qDebug() << Q_FUNC_INFO << "remove 3:" << QFile::remove(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("dictionaries/f1b2938873d665eecee63e39a68862a5.dix"));

    if(QFile::exists(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("dictionaries/aa4eceb10187d79aebd416b7d75d2659.dix")))
        qDebug() << Q_FUNC_INFO << "remove 4:" << QFile::remove(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("dictionaries/aa4eceb10187d79aebd416b7d75d2659.dix"));

    if(QFile::exists(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("dictionaries/cd9a346bd6528176ced3ddbe105ec4d6.dix")))
        qDebug() << Q_FUNC_INFO << "remove 5:" << QFile::remove(Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("dictionaries/cd9a346bd6528176ced3ddbe105ec4d6.dix"));
}
