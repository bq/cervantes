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

#ifndef QBOOK_H
#define QBOOK_H

/*-------------------------------------------------------------------------*/

#include <QObject>
#include <QLocale>
#include <QDateTime>
#include <QMap>
#include <QRect>
#include <QEvent>
#include <QSettings>
#include <QApplication>
#include <QStringList>

class QImage;
class QPoint;
class QBookApp;
class QTranslator;

#define Q_BOOK_THUMBNAIL_DIR     ".thumbnail"
#define Q_BOOK_THUMBNAIL_EXTENSION	".jpg"

#ifdef Q_WS_QWS
#define Q_BOOK_PDF_DIR "/tmp/.doc/"
#else
#define Q_BOOK_PDF_DIR ".doc/"
#endif

#define Q_BOOK_MMCPDF_DIR ".mmc"

/*-------------------------------------------------------------------------*/

class QBook : public QObject
{
    Q_OBJECT

public:

    enum Key
    {
        QKEY_POWER          = Qt::Key_F20,
        QKEY_HOME           = Qt::Key_F23,
        QKEY_BACK           = Qt::Key_F24,
        QKEY_MENU           = Qt::Key_F25
    };

    enum ScreenResolution {
        RES600x800,
        RES758x1024,
        RES1072x1448
    };

    static void init(QApplication* app);

    static QString getDebugFilter();
    static void setDebug( const QString& );

    static QString getThumbnailPath(const QString & );

    static QDateTime dateTime() { return QDateTime::currentDateTime(); }
    static QDateTime rtcDateTime();

    static void setRtcDateTime(const QDateTime& time);

    static QString getInfoPath(const QString &bookpath);
    static int getResolution();


    static QString etcDirPath();
    static QString resDirPath();
    static QString fontsDirPath();
    static QString settingsDirPath();
    static QString langsDirPath();
    static QString tmpDirPath();

    static QSettings& settings();
    static bool resetSettings();

    static int screenDpi();

	static void setProgressBarMaxValue(int max);
	static void setProgressBarValue(int value);
	static void setProgressBarText(QString str);
	static void setProgressBarVisible(bool isVisible);
	static void setProgressBarCanceled();
	static void setProgressBarButtonVisible(bool isVisible);

    static void lockScreen(bool onceOnly = false, QString from = "");

    static QBook *getInstance();

signals:
    void setKeyboardText(QString);
    void sleepStateChanged(bool on);
    void sleepRequested(bool deep);
    void timeChanged();
    void opmodeStateChanged(int on);

    void screenRotate(int rot);
    void progressBarMaxValueChanged(int value);
    void progressBarValueChanged(int value);
    void progressBarVisibleChanged(bool isVisible);
    void progressBarTextChanged(QString text);
    void progressBarCanceled();
    void progressBarButtonVisibleChanged(bool isVisible);


    //Push refresh
    void refreshPushed();

#ifdef Q_WS_QWS
	void requestToGrabScreen(const QRect &rect);
	void flushCache();
#endif //#ifdef Q_WS_QWS
	void requestPassword(QString, QString);

private:
    QBook();
    virtual ~QBook();

    static QBook *s_instance;

    friend class QBookEvent;

protected:
    void setupFonts();
    void setupFallbackFonts();
    void setupLanguage(const QString& locale);
    void clearTranslations();

    QList <QTranslator *> m_translations;
    QStringList m_resources;
    QSettings *m_settings;

    int                 m_qbook_event_type;

};

/*-------------------------------------------------------------------------*/

#endif
