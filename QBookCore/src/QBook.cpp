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

#include "QBook.h"
#include "Screen.h"

#ifdef Q_WS_QWS
#include <ntx.h>
#include <QImage>
#include <QPoint>
#endif //#ifdef Q_WS_QWS
#include "QDocView.h"
#include "Power.h"
#include "RTCManager.h"
#include "Storage.h"
#include "Battery.h"
#include "QBookApp.h"
#include "QBookAppService.h"

#include <syslog.h>

#ifdef Q_WS_QWS
QMap<int, QString> bufferImageMap;
#endif //#ifdef Q_WS_QWS
/*-------------------------------------------------------------------------*/

#ifdef MX508
#define APPLICATION_PATH "/app/"
#endif
#ifndef Q_WS_QWS
#define APPLICATION_PATH "./"
#endif

#define POWERKEY_LONG_PRESS     2000

#define TEMP_PATH               "/tmp"

/*-------------------------------------------------------------------------*/

QBook* QBook::s_instance = NULL;

static QString _debug;
static bool debugInit = false;
static bool debugNone = false;
static bool debugAll = false;
static char **debugPrefixes = NULL;

static void buildDebugPrefixes(void)
{
    debugAll = false;
    debugNone = false;

    if (_debug.isEmpty() || _debug == "off")
    {
        if (debugPrefixes)
        {
            char** p = debugPrefixes;
            while (*p != NULL)
            {
                free(*p);
                ++p;
            }
            free(debugPrefixes);
            debugPrefixes = NULL;
        }
        debugNone = true;
        closelog();
        return;
    }

    openlog(NULL, LOG_PERROR, LOG_SYSLOG);
    if (_debug == "*")
        debugAll = true;
    else
    {
        QStringList qprefixes = _debug.split(":");
        int size = qprefixes.size();
        debugPrefixes = (char**)malloc(sizeof(char*)*(size + 1));// NOTE: Last one will be NULL.

        for (int i=0; i < size; i++)
        {
            QByteArray qba = qprefixes.at(i).toAscii();
            debugPrefixes[i] = strdup(qba.constData());
        }
        debugPrefixes[size] = NULL;
    }
}


static void QBook_MessageOutput(QtMsgType type, const char *msg)
{
    if (!debugInit)
    {
        buildDebugPrefixes();
        debugInit = true;
    }

    if (debugNone)
        return;

    if (!debugAll)
    {
        bool should_print = false;
        const char *colon = strstr(msg, "::");
        if (colon)
        {
            char **prefix = debugPrefixes;
            while (*prefix != NULL)
            {
                char *class_start = (char*)colon;
                while (*(class_start-1) != ' ' && (class_start-1) != msg) class_start--;
                if ((class_start-1) != msg && strncmp(*prefix, class_start, colon-class_start) == 0)
                {
                    should_print = true;
                    break;
                }
                prefix++;
            }
            if (!should_print)
                return;
        }
    }

    QString dt = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QByteArray qba = dt.toAscii();

    const char* dtchar = qba.constData();

    #ifdef Q_WS_QWS
        syslog(LOG_SYSLOG|LOG_DEBUG, "%s Debug: %s\n", dtchar,msg);
    #else
        fprintf(stderr, "%s: %s\n", dtchar,msg);
    #endif
}

QString QBook::getThumbnailPath(const QString & bookpath)
{
        QString output = QString(bookpath + Q_BOOK_THUMBNAIL_EXTENSION);
        return output;
}

static QString searchPath(const QString &path_list, const QString &name, const QString &def_path)
{
    QDir dir = QApplication::applicationDirPath();
    QStringList list = path_list.split(':');

    for (int i = 0; i < list.size(); i++) {
        QString path = list.at(i) + name;
        if (dir.exists(path)) return QDir::cleanPath(dir.absoluteFilePath(path));
    }

    return QDir::cleanPath(dir.absoluteFilePath(def_path + name));
}

static inline QString& initPath(QString& target, const QString &path_list, const QString &name, const QString &def_path)
{
    if (target.isEmpty()) target = searchPath(path_list, name, def_path);
    return target;
}

QDateTime QBook::rtcDateTime()
{
    return RTCManager::rtcDateTime();
}

void QBook::setRtcDateTime(const QDateTime& dateTime)
{
    RTCManager::setRtcDateTime(dateTime);
}

QString QBook::etcDirPath()
{
#ifdef Q_WS_QWS
    return APPLICATION_PATH "etc";
#else
    return APPLICATION_PATH "QHome/etc";
#endif
}

QString QBook::resDirPath()
{
#ifdef Q_WS_QWS
    return APPLICATION_PATH "res";
#else
    return APPLICATION_PATH "QHome/res";
#endif
}

QString QBook::fontsDirPath()
{
#ifdef Q_WS_QWS
    return "/usr/lib/fonts";
#else
    return ROOTFS "/usr/lib/fonts";
#endif
}

QString QBook::settingsDirPath()
{
    return Storage::getInstance()->getDataPartition()->getMountPoint() + "/settings/";
}

QString QBook::langsDirPath()
{
#ifdef Q_WS_QWS
    return APPLICATION_PATH "langs";
#else
    return "x86-bin/langs";
#endif
}

QString QBook::tmpDirPath()
{
    return "/tmp";
}

/*-------------------------------------------------------------------------*/

QSettings& QBook::settings()
{
    QSettings *settings = QBook::getInstance()->m_settings;
    if (!settings)
    {
        QFileInfo info = QFileInfo(QApplication::applicationFilePath());
        QString name = info.baseName() + ".ini";
        QString path = QBook::settingsDirPath() + name;

        if (!QFile::exists(path)) {
            qDebug() << "QBookApp.ini not found";

            QBook::getInstance()->resetSettings();
        }

        settings = new QSettings(path, QSettings::IniFormat);
        settings->setIniCodec("UTF-8");
    }
    return *settings;
}

bool QBook::resetSettings()
{
    qDebug() << "--->" << Q_FUNC_INFO;

    QFileInfo info = QFileInfo(QApplication::applicationFilePath());
    QString name = info.baseName() + ".default.ini";

    QString factory_path = QBook::etcDirPath() + "/" + name;

    if (!QFile(factory_path).exists()) return false;

    QString path;

    QSettings *settings = QBook::getInstance()->m_settings;
    if (settings) {
        path = settings->fileName();
        delete settings;
        settings = NULL;
    }
    else {
        name = info.baseName() + ".ini";
        path = QBook::settingsDirPath() + name;
    }

    // Rename last
    QFile::remove(path + ".bak");
    QFile::rename(path, path + ".bak");

    //Prepare folder
    QDir dir(QBook::settingsDirPath());
    if(!dir.exists()) {
	QString dirname = dir.dirName();  
	dir.cdUp();
        dir.mkdir(dirname);
    }

    // Overwrite QBookApp.ini
    if (!QFile::copy(factory_path, path)) {
        QFile::rename(path + ".bak", path);
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

extern "C" void QBookScreen_SetMode(int, int);
extern "C" void QBookScreen_Refresh(int,QString);
extern "C" void QBookScreen_Flush(void);
extern "C" void QBookScreen_ForceRefresh(int);

int QBook::screenDpi()
{
    int dpi = QBookApp::instance()->logicalDpiY();
    qDebug() << Q_FUNC_INFO << dpi;
    return dpi;
}

/*-------------------------------------------------------------------------*/

QBook::QBook()
    : m_settings(NULL)
{
    qDebug() << Q_FUNC_INFO;
    m_qbook_event_type = QEvent::registerEventType();
}

QBook::~QBook()
{
    qDebug() << Q_FUNC_INFO;
    clearTranslations();

    for (int i = 0; i < m_resources.count(); ++i)
    {
        QResource::unregisterResource(m_resources.at(i));
    }
    m_resources.clear();

    delete m_settings;
    m_settings = NULL;

    // Free debug prefixes
    _debug = "";
    buildDebugPrefixes();
}

/*static*/ QBook* QBook::getInstance()
{
    if (!s_instance)
        s_instance = new QBook();
    return s_instance;
}

/*-------------------------------------------------------------------------*/

void QBook::init(QApplication* app)
{
    qDebug() << Q_FUNC_INFO;
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    _debug = getDebugFilter();

    qInstallMsgHandler(QBook_MessageOutput);
    qDebug() << "Initializing QBook, filter for logs = " << _debug;

#ifdef QT_KEYPAD_NAVIGATION
    QApplication::setNavigationMode(Qt::NavigationModeNone);
#endif

    QString lang = settings().value("setting/language", QVariant("es")).toString();
    QBook::getInstance()->setupFonts();
    QBook::getInstance()->setupLanguage(lang);
    QLocale::setDefault(QLocale(lang));

#ifdef Q_WS_QWS
    QApplication::setCursorFlashTime(0);
    QApplication::setDoubleClickInterval(800);
#endif

    new QBookAppService(app);

    // connect to D-Bus and register as an object:
    bool res = QDBusConnection::sessionBus().registerService("com.bqreaders.QBookAppService");
    if (!res)
    {
        qDebug() << Q_FUNC_INFO << "Cannot register the com.bqreaders.QBookAppService";
        qDebug() << Q_FUNC_INFO << QDBusConnection::sessionBus().lastError().message();
        return;
    }

    res = QDBusConnection::sessionBus().registerObject("/QBookAppService", app);
    if (!res)
    {
        qDebug() << Q_FUNC_INFO << "Cannot register the /QBookAppService object";
        qDebug() << Q_FUNC_INFO << QDBusConnection::sessionBus().lastError().message();
        return;
    }
}

QString QBook::getDebugFilter()
{
    qDebug() << Q_FUNC_INFO;

    QFile debugCfgFile("/mnt/sd/debug.cfg"); //Storage class may have not been initialized
    if(debugCfgFile.exists()
       && debugCfgFile.open(QIODevice::ReadOnly))
    {
        QString qbookDebugValue = debugCfgFile.readLine().trimmed();
        setenv("QBOOK_DEBUG",qbookDebugValue.toStdString().c_str(),1); // Overwrite variable
        debugCfgFile.close();
    }

    QString filter = getenv("QBOOK_DEBUG");

    return filter;
}

void QBook::setDebug(const QString& debug)
{
    qDebug() << Q_FUNC_INFO << debug;
    _debug = debug;
    buildDebugPrefixes();
}

void QBook::setProgressBarMaxValue(int max)
{
        emit QBook::getInstance()->progressBarMaxValueChanged(max);
}

void QBook::setProgressBarValue(int value)
{
        emit QBook::getInstance()->progressBarValueChanged(value);
}

void QBook::setProgressBarVisible(bool isVisible)
{
        emit QBook::getInstance()->progressBarVisibleChanged(isVisible);
}

void QBook::setProgressBarText(QString str)
{
        emit QBook::getInstance()->progressBarTextChanged(str);
}

void QBook::setProgressBarCanceled()
{
        emit QBook::getInstance()->progressBarCanceled();
}

void QBook::setProgressBarButtonVisible(bool isVisible)
{
        emit QBook::getInstance()->progressBarButtonVisibleChanged(isVisible);
}

void QBook::setupFonts()
{
#ifdef Q_WS_QWS
    QString font_def = settings().value("font/default", "none").toString();
#else
    QString font_def = "Movistar Text";
#endif
    QApplication::setFont(QFont(font_def));

    settings().beginGroup("font.substitutions");
    QStringList font_subs = settings().childKeys();
    for (int i = 0; i < font_subs.size(); i++) {
        QString fam = font_subs.at(i);
        QStringList subs = settings().value(fam).toStringList();
        QFont::insertSubstitutions(fam, subs);
    }
    settings().endGroup();

    setupFallbackFonts();
}

void QBook::setupFallbackFonts()
{
    qDebug() << "--->" << Q_FUNC_INFO;
    settings().beginGroup("font.fallbacks");

    // TODO freescale
//#ifdef Q_WS_QWS
//    if (!settings.childKeys().isEmpty()) {
//        extern void qt_applyFontDatabaseSettings(const QSettings &);
//        qt_applyFontDatabaseSettings(settings);
//    }
//#endif
    settings().endGroup();

}

void QBook::setupLanguage(const QString& locale)
{
    QString dir_path = QBook::langsDirPath();
    QDir dir = dir_path;
    QString lookup = "_" + locale;

    // load translations
    QStringList files = dir.entryList(QStringList("*.qm"), QDir::Files|QDir::Readable);
    QStringList trans_names;

    // remove old translations
    clearTranslations();

    // find base name of translations
    for (int i = 0; i < files.count(); i++) {
        QString name = files.at(i);
        int pos = name.indexOf('_');
        if (pos < 0) pos = name.length() - 3;
        name.resize(pos);
        if (!trans_names.contains(name)) trans_names.append(name);
    }

    // install locale specific translations
    for (int i = 0; i < trans_names.count(); i++) {
        QString name = trans_names.at(i);
        QTranslator* tr = new QTranslator();
        if (tr->load(name + lookup, dir_path)) {
            m_translations.append(tr);
            QApplication::installTranslator(tr);
        } else {
            delete tr;
        }
    }

    // load dynamic resources
    files = dir.entryList(QStringList("*.rcc"), QDir::Files|QDir::Readable);

    // remove old resources
    for (int i = 0; i < m_resources.count(); i++) {
        QResource::unregisterResource(m_resources.at(i));
    }
    m_resources.clear();

    // classify resources
    QStringList base_resources;
    QStringList fallback_resources;

    for (int i = 0; i < files.count(); i++) {
        QString name = files.at(i);
        int pos = name.indexOf('_');
        if (pos < 0) {
            base_resources += dir.absoluteFilePath(name);
        } else {
            QString suffix = name.mid(pos, name.length() - pos - 4);
            if (lookup == suffix) m_resources += dir.absoluteFilePath(name);
            else if (lookup.startsWith(suffix)) fallback_resources += dir.absoluteFilePath(name);
        }
    }
    // append base and fall back resources after language resource
    m_resources += fallback_resources;
    m_resources += base_resources;
    // register new resources
    for (int i = 0; i < m_resources.count(); i++) {
        QString path = m_resources.at(i);
        QResource::registerResource(path);
    }
}

void QBook::clearTranslations()
{
    qDeleteAll(m_translations);
    m_translations.clear();
}

int QBook::getResolution()
{
    static int res = 0;
    if (res == 0) {
#ifndef Q_WS_QWS
        foreach (QString arg, QCoreApplication::arguments()) {
            switch(arg)
            {
                case "-1448":
                    res = RES1072x1448;
                    break;
                case "-1024":
                    res = RES758x1024;
                    break;
                case "-800": default:
                    res = RES600x800;
                    break;
            }
        }
        return res;
#else
        struct ntxhwconfig hwconfig;
        if (ntx_read_hwconfig("/dev/mmcblk0", &hwconfig) == 0) {
            switch (hwconfig.values.display_resolution) {
                case 1:
                    res = RES758x1024;
                    break;
                case 5:
                    res = RES1072x1448;
                    break;
                default:
                    res = RES600x800;
                    break;
            }
        } else {
            qWarning() << "Cannot get resolution from hwconfig, setting to 800";
            res = RES600x800;
        }
#endif
    }
    return res;
}
