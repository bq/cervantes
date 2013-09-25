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

#include "QBookApp.h"
#include "Power.h"
#include "QBook.h"
#include "QDocView.h"
#include "Screen.h"
#include "Storage.h"
#include "Keyboard.h"
#include "Wifi.h"
#include "Battery.h"
#include "ADConverter.h"
#include "DeviceInfo.h"
#include "FrontLight.h"
#include "hgversion.h"
#include "AfterUpdateWorker.h"
#include "SleeperThread.h"

#include <QApplication>
#include <QWSServer>
#include <signal.h>

#ifndef Q_WS_QWS
    #include "QBookEmu.h"
    #define Status int
    extern "C" Status XInitThreads();
#endif

#include "SettingsDateTimeMenu.h"

void loadRccs()
{
    QString resourcePath;
#ifndef Q_WS_QWS
    resourcePath = "x86-bin/langs/";
#else
    resourcePath = "/app/langs/";
#endif

    QStringList rccs;
    switch(QBook::getResolution()){
    case QBook::RES600x800:
        qDebug() << Q_FUNC_INFO << "********** Setting screen size to 600x800";
        rccs << "QBookApp800.rcc" <<"Browser800.rcc" << "common800.rcc" << "Viewer800.rcc" << "Ui800.rcc" << "Settings800.rcc" << "Library800.rcc" << "Search800.rcc" << "Home800.rcc" << "Wizard800.rcc" << "WebStore800.rcc";
        break;
    case QBook::RES758x1024:
        qDebug() << Q_FUNC_INFO << "********* Setting screen size to 1024";
        rccs << "QBookApp1024.rcc" << "Browser1024.rcc" << "common1024.rcc" << "Viewer1024.rcc" << "Ui1024.rcc" << "Settings1024.rcc" << "Library1024.rcc" << "Search1024.rcc" << "Home1024.rcc" << "Wizard1024.rcc" << "WebStore1024.rcc";
        break;
    default:
        qWarning() << Q_FUNC_INFO << "UNKNOWN SCREEN SIZE";

    }

    foreach (QString rcc, rccs) {
        QString file = resourcePath + rcc;
        qDebug() << "Registering rcc: " << file;
        if (!QResource::registerResource(file)) {
            qDebug() << "Error registering " << file;
        }
    }
}

void bugbuddy_segv_handle(int signum)
{
    if(signum == 7)
        exit(7);
}

/*-------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
    static struct sigaction *setptr;
    static struct sigaction old_action;
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    setptr = &sa;
    sa.sa_handler = bugbuddy_segv_handle;
    sigaction(SIGBUS, NULL, &old_action);
    if (old_action.sa_handler == SIG_DFL)
        sigaction(SIGBUS, setptr, NULL);


#ifndef Q_WS_QWS
    XInitThreads();
#endif
    QApplication* app = new QApplication(argc, argv);

#ifdef Q_WS_QWS
    QWSServer::setCursorVisible(false);
    app->setOverrideCursor(QCursor(Qt::BlankCursor));
#endif

    qDebug() << "Starting QBookApp version:" << HG_VERSION;

    // Singletons
    loadRccs();


    QFile fileCommon(":/res/common_styles_generic.qss");
    QFile fileSpecific(":/res/common_styles.qss");
    fileCommon.open(QFile::ReadOnly);
    fileSpecific.open(QFile::ReadOnly);
    QString StyleSheet = QLatin1String(fileCommon.readAll() + fileSpecific.readAll());
    app->setStyleSheet(StyleSheet);

    Screen::staticInit();

    int errorCode = QString(argv[1]).toInt();
    qDebug() << "Start code:" << errorCode;

    Storage::staticInit();
    Storage::getInstance()->initAppData();
    QBook::init(app);

    if ((errorCode - 128) == SIGSEGV || (errorCode - 128) == SIGBUS || (errorCode - 128) == SIGILL || errorCode == SIGSEGV || errorCode == SIGBUS || errorCode == SIGILL)
    {
        QString lang = QBook::settings().value("setting/language", QVariant("es")).toString();
        QString restoreImage = QString("recoverError_%1.png").arg(lang);
        QImage restoringImage(":/" + restoreImage);
        Screen::getInstance()->flushImage(restoringImage, QPoint(0,0));
    }else{
        QImage splashImage(":/starting.png");
        Screen::getInstance()->flushImage(splashImage, QPoint(0,0));
    }

    Power::staticInit();
    Keyboard::staticInit();
    Wifi::staticInit();
    Battery::staticInit();
    DeviceInfo::staticInit();
    ADConverter::staticInit();
    FrontLight::staticInit();

    QString splashImageResource(":/starting.png");
#ifdef HACKERS_EDITION
    // Modify starting image if running on external SD
    QProcess checkRootPartition;
    checkRootPartition.start("ls  -la /dev/root");
    if(!checkRootPartition.waitForFinished())
    {
        qDebug () << Q_FUNC_INFO << "Error reading running parition";
    }
    else
    {
        QString result(checkRootPartition.readAll());
        qDebug() << Q_FUNC_INFO << result;
        result.chop(2); // To remove px part of the string
        if(result.trimmed().endsWith("mmcblk1"))
            splashImageResource = ":/startingSD.png";
    }
#endif

    QImage splashImage(splashImageResource);
    Screen::getInstance()->flushImage(splashImage, QPoint(0,0));

    AfterUpdateWorker::getInstance()->work();

#ifndef HACKERS_EDITION // Not to show shop image if hackes edition
    QString imagePath = Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + QString("startDeviceImage.png");
    QFile imageFile(imagePath);
    if(imageFile.exists()){ //To show a customized image from the current store.
        QImage splashImageCustomized(imagePath);
        if(splashImageCustomized.width() <= Screen::getInstance()->screenWidth() &&
                splashImageCustomized.height() <= Screen::getInstance()->screenHeight())
            Screen::getInstance()->flushImage(splashImageCustomized, QPoint(0,0));
    }
#endif

    SettingsDateTimeMenu::setTimeZoneEnvironment();
    Screen::getInstance()->lockScreen();

    QBookApp::staticInit();
    QDocView::staticInit();

#ifdef Q_WS_QWS
    QBookApp* top = QBookApp::instance();
    top->showFullScreen();
    Screen::getInstance()->refreshScreen(Screen::MODE_SAFE, true);
#else
    QBookEmu* top = new QBookEmu();
    top->show();
    QBook::setDebug("*");
#endif

    int err = app->exec();

    qDebug("QApplication::exit = %d", err);

    QBookApp::staticDone();
    QDocView::staticDone();
#ifndef Q_WS_QWS
    if (top)
	delete top;
#endif
	

    if (err == 199) { 
        Power::getInstance()->powerOff();
    } else {
        delete app;
    }

    return err;
}
