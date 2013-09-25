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

#include "QDocView.h"

#ifndef DISABLE_ADOBE_SDK
#include "QAdobeDocView.h"
#endif
#include "BookInfo.h"
#include "Screen.h"

#include <QDebug>
#include <QStringList>
#include <QProcess>
#include <QPainter>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QDir>
#include <cmath>
#include <QTextCodec>

#include "QCREngineDocView.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

/*-------------------------------------------------------------------------*/

#define VIEWER_ADOBE     1
#define VIEWER_WEBKIT    2
#define VIEWER_TEXT      3
#define VIEWER_CRENGINE  5

QString QDocView::s_path = "";

struct QMimeTypeMapping
{
    int             viewer;
    const char*     suffix;
    const char*     type;
};

static const QMimeTypeMapping MIME_MAP[] = {
#ifndef DISABLE_ADOBE_SDK
    { VIEWER_ADOBE,     "|pdf|",            "application/pdf" },
    { VIEWER_ADOBE,     "|psf|",            "application/psf" },
    { VIEWER_ADOBE,     "|epub|",           "application/epub+zip" },
#else
    { VIEWER_CRENGINE,  "|epub|",           "application/epub+zip" },
#endif
    { VIEWER_CRENGINE,      "|doc|",            "text/doc" },
    { VIEWER_CRENGINE,    "|html|htm|",         "text/html" },
    { VIEWER_CRENGINE,      "|txt|",            "text/plain" },
    { VIEWER_CRENGINE,      "|fb2|",            "application/fb2+zip" },
    { VIEWER_CRENGINE,      "|chm|",            "application/chm" },
    { VIEWER_CRENGINE,      "|mobi|",           "application/x-mobipocket-ebook" },
    { VIEWER_CRENGINE,      "|rtf|",            "application/rtf" },

#ifdef _DJVULIBRE_
   { VIEWER_DJVULIBRE,      "|djvu|",            "application/djvu" },
#endif

#ifdef _CHM_
   { VIEWER_CHM, 	 "|chm|",               "application/chm" },
#endif

    { 0 , "", ""}
};

static const QMimeTypeMapping* getMimeTypeMapping(const QString& mimeType)
{
    const QMimeTypeMapping* map = MIME_MAP;
    while (map->viewer) {
        if (mimeType == QLatin1String(map->type)) return map;
        map++;
    }
    return 0;
}

int QDocView::getMimeTypeViewer(const QString& mimeType)
{
    const QMimeTypeMapping* map = MIME_MAP;
    while (map->viewer) {
        if (mimeType == QLatin1String(map->type)) return map->viewer;
        map++;
    }
    return -1;
}

QStringList QDocView::supportedFileTypes()
{
    QStringList names;
    const QMimeTypeMapping* map = MIME_MAP;
    while (map->viewer) {
        QStringList suffix = QString(map->suffix).split(QChar('|'), QString::SkipEmptyParts);
        for (int i = 0; i < suffix.size(); i++) {
            names.append("*." + suffix.at(i));
        }
        map++;
    }
    return names;
}

bool QDocView::isMimeTypeSupported(const QString& mimeType)
{
    return getMimeTypeMapping(mimeType) != 0;
}

QString QDocView::guessMimeType(const QUrl& url)
{
    QString key = url.path().simplified().toLower();
    int idx = key.lastIndexOf('.');
    if (idx >= 0) {
        key = QChar('|') + key.right(key.length() - idx - 1) + QChar('|');
        const QMimeTypeMapping* map = MIME_MAP;
        while (map->viewer) {
            if (QString(map->suffix).indexOf(key) >= 0) {
                return QString(map->type);
            }
            map++;
        }
    }
    return url.scheme();
}

/*-------------------------------------------------------------------------*/

QDocView* QDocView::create(const QString& mime, const QString& path, QWidget* parent)
{
    QDocView* doc = 0;
    s_path = path;
    qDebug() << Q_FUNC_INFO << "m_path :: == " << s_path << "parent" << parent;

    const QMimeTypeMapping* map = getMimeTypeMapping(mime);
    if (map) {
        switch (map->viewer) {
#ifndef DISABLE_ADOBE_SDK
        case VIEWER_ADOBE:
            doc = QAdobeDocView::create(mime, parent);
            break;
#endif
        case VIEWER_CRENGINE:
            doc = new QCREngineDocView(parent);
            break;

        default:
            qDebug() << Q_FUNC_INFO << "NOT EXPECTED VIEWER";
            break;
        }
    }

    return doc;
}


QString QDocView::coverPage(const QString & path, const QString & destination)
{
    const QMimeTypeMapping* map = getMimeTypeMapping(guessMimeType(QUrl::fromLocalFile(path)));
    if (map) {
        switch (map->viewer) {
#ifndef DISABLE_ADOBE_SDK
        case VIEWER_ADOBE:
            return QAdobeDocView::coverPage(path,destination);
#endif
        case VIEWER_CRENGINE:
            return QCREngineDocView::coverPage(path,destination);
	}
        return NULL;
    }
}

double QDocView::getPosFromBookmark(const QString& ref){
    qDebug() << Q_FUNC_INFO << ref;
    return 0;
}

double QDocView::getPosFromHighlight(const QString& ref){
    qDebug() << Q_FUNC_INFO << ref;
    return 0;
}

int QDocView::getPageFromMark(const QString& ref){
    qDebug() << Q_FUNC_INFO << ref;
    return 0;
}

QPixmap *QDocView::coverPage(const QString & path, QSize& size)
{
    const QMimeTypeMapping* map = getMimeTypeMapping(guessMimeType(QUrl::fromLocalFile(path)));
    if (map) {
        switch (map->viewer) {
#ifndef DISABLE_ADOBE_SDK
        case VIEWER_ADOBE:
            return QAdobeDocView::coverPage(path,size);
#endif
        case VIEWER_CRENGINE:
            //FIXME: implement this on crengine
            //return QCREngineDocView::coverPage(path,size);
            return NULL;
        }
        return NULL;
    }
}


/*-------------------------------------------------------------------------*/

QDocView::QDocView(QWidget* parent)
    : QWidget(parent)
    , m_topMargin(0.0)
    , m_bottomMargin(0.0)
    , m_leftMargin(0.0)
    , m_rightMargin(0.0)
    , m_noteFont("DejaVu Sans")
    , m_dictionaryMode(false)
    , m_highlightMode(false)
    , m_blockPaintEvents(false)
    , m_bookInfo(NULL)
{
    m_noteIcon = new QPixmap(":/reader/ind-note");
    m_noteGap = m_noteIcon->height() / 3;
    m_noteFont.setPixelSize(m_noteIcon->height() / 3);
}

QDocView::~QDocView()
{
    delete m_noteIcon;
    m_noteIcon = NULL;
    clearNoteList();
}

/*-------------------------------------------------------------------------*/

QSize QDocView::sizeHint() const
{
    return QSize(parentWidget ()->width(), parentWidget ()->height());
}

bool QDocView::goBack()
{
    return false;
}

bool QDocView::goForward()
{
    return false;
}

QString QDocView::wordAt(int, int)
{
    return QString();
}

QStringList QDocView::wordAt(int,int, int)
{
    return QStringList();
}


QDocView::TableOfContent* QDocView::tableOfContent()
{
    return 0;
}

bool QDocView::isHorizontal() const
{
    return false;
}

bool QDocView::setHorizontal(bool)
{
    return false;
}

bool QDocView::isSearchSupported() const
{
        return true;
}
/*-------------------------------------------------------------------------*/

QList<int> QDocView::noteList() const
{
    return m_notes;
}

void QDocView::setNoteList(const QList<int>& notes)
{
    m_notes = notes;
    update();
}

void QDocView::clearNoteList()
{
    m_notes.clear();
    update();
}

int QDocView::noteAt(int x, int y) const
{
    QRect r(m_noteGap, m_noteGap, m_noteIcon->width(), m_noteIcon->height());
    int d = m_noteGap / 2;

    int count = m_notes.count();
    for (int i = 0; i < count; ++i)
    {
        QRect t = r;
        t.adjust(-d, -d, d, d);
        if (t.contains(x, y)) return m_notes.at(i);
        r.translate(0, r.height() + m_noteGap);
        if (!rect().contains(r)) r.moveTo(r.right() + m_noteGap, m_noteGap);
    }

    return -1;
}

void QDocView::drawNotes(QPainter& painter, const QRect& wr, const QRect& clip)
{
    if (!m_notes.isEmpty()) {
        QRect r(m_noteGap, m_noteGap, m_noteIcon->width(), m_noteIcon->height());
        painter.setFont(m_noteFont);

        int count = m_notes.count();
        for (int i = 0; i < count; ++i)
        {
            if (clip.intersects(r)) {
                painter.drawPixmap(r, *m_noteIcon);
                QRect t = r;
                t.setTop(t.top() + t.height() / 2);
                t.setHeight(t.height() / 2);
                painter.drawText(t, Qt::AlignCenter, QString::number(m_notes.at(i)));
            }
            r.translate(0, r.height() + m_noteGap);
            if (!wr.contains(r)) {
                r.moveTo(r.right() + m_noteGap, m_noteGap);
                if (!wr.contains(r)) break;
            }
        }
    }
}

void QDocView::setFileExist(bool)
{
    // do nothing
}
bool QDocView::isOpenPWDialog(void)
{
        return false;
}

bool QDocView::isHighlightSupported() const
{
    return false;
}

int QDocView::trackHighlight(HighlightMode, int, int)
{
    return -1;
}

QStringList QDocView::highlightList() const
{
    return QStringList();
}

void QDocView::setHighlightList(const QStringList&)
{
    // do nothing
}

void QDocView::setHighlightList(const QStringList&, const QStringList&)
{
    // do nothing
}

int QDocView::highlightCount() const
{
    return 0;
}

int QDocView::highlightAt(int, int) const
{
    return -1;
}

void QDocView::removeHighlight(int)
{
    // do nothing
}

bool QDocView::eraseHighlight(int, int)
{
        return false;
}

bool QDocView::isHiliEdited()
{
        return false;
}

void QDocView::setEdited(bool)
{
        // do nothing
}


void QDocView::clearHighlightList()
{
    // do nothing
}

QDocView::Location* QDocView::highlightLocation(int) const
{
    return 0;
}

QRect QDocView::getHighlightBBox(int) const
{
    return QRect();
}

/*-------------------------------------------------------------------------*/

QString QDocView::extractWord(const QString& text, int pos, bool isHiliMode)
{
    qDebug() << Q_FUNC_INFO << "Extracting word with text" << text << "pos" << pos << "and hili" << isHiliMode;
    QString word;

    if(isHiliMode) {

        QChar ch = text.at(pos);
        if(ch.isPunct() && ch != '-')
            return word.append(ch);

        for (int i = pos; i >= 0; --i) {
            ch = text.at(i);
            if (ch.isSpace() || (ch.isPunct() && ch != '-') || (i < pos && isHanzi(ch))) break;
            word.prepend(ch);
        }

        int length = text.length();
        for (int i = pos + 1; i < length; ++i) {
            ch = text.at(i);
            if (ch.isSpace() || (ch.isPunct() && ch != '-') || isHanzi(ch)) break;
            word.append(ch);
        }
    } else {

        for (int i = pos - 1; i >= 0; --i) {
            QChar ch = text.at(i);
            if (ch.isSpace() || (ch.isPunct() && ch != '-') || isHanzi(ch)) break;
            word.prepend(ch);
        }

        int length = text.length();
        for (int i = pos; i < length; ++i) {
            QChar ch = text.at(i);
            if (ch.isSpace() || (ch.isPunct() && ch != '-') || (i > pos && isHanzi(ch))) break;
            word.append(ch);
        }
    }

    return word;
}

QDateTime QDocView::fromIsoDate(const QString& text)
{
    int i = 0;
    int n = text.length();
    bool ok = false;
    if (n < 7) return QDateTime();

    // YYYY-MM-DD or YYYYMMDD or YYYY-MM
    int y = text.mid(0, 4).toInt(&ok);
    if (!ok) return QDateTime();
    i += 4;

    int ch = text.at(i).unicode();
    if (ch == '-') i++;

    if (i + 2 > n) return QDateTime();
    int m = text.mid(i, 2).toInt(&ok);
    if (!ok) return QDateTime();
    i += 2;
    if (i == n) return QDateTime(QDate(y, m, 1));

    ch = text.at(i).unicode();
    if (ch == '-') i++;

    if (i + 2 > n) return QDateTime();
    int d = text.mid(i, 2).toInt(&ok);
    if (!ok) return QDateTime();
    i += 2;

    if (i == n) return QDateTime(QDate(y, m, d));
    ch = text.at(i).unicode();
    if (ch == 'T' || ch == ' ') i++;

    // hh:mm:ss	or hhmmss or hh:mm or hhmm or hh
    if (i + 2 > n) return QDateTime();
    int hh = text.mid(i, 2).toInt(&ok);
    if (!ok) return QDateTime();
    i += 2;
    if (i == n) return QDateTime(QDate(y, m, d), QTime(hh, 0, 0));

    ch = text.at(i).unicode();
    if (ch == ':') i++;

    if (i + 2 > n) return QDateTime();
    int mm = text.mid(i, 2).toInt(&ok);
    if (!ok) return QDateTime();
    i += 2;
    if (i == n) return QDateTime(QDate(y, m, d), QTime(hh, mm, 0));

    ch = text.at(i).unicode();
    if (ch == ':') i++;

    if (i + 2 > n) return QDateTime();
    int ss = text.mid(i, 2).toInt(&ok);
    if (!ok) return QDateTime();
    i += 2;
    if (i == n) return QDateTime(QDate(y, m, d), QTime(hh, mm, ss));

    // <time>Z or <time>hh:mm or <time>hhmm or <time>hh
    QDateTime date(QDate(y, m, d), QTime(hh, mm, ss), Qt::UTC);
    ch = text.at(i).unicode();
    if (ch == 'Z') return date;

    int delta = 0;
    if (ch == '+') {
        delta = -1;
        i++;
    } else {
        delta = 1;
        i++;
    }
    if (i == n) return QDateTime();

    if (i + 2 > n) return QDateTime();
    hh = text.mid(i, 2).toInt(&ok);
    if (!ok) return QDateTime();
    i += 2;
    if (i == n) return date.addSecs(delta * hh * 60 * 60);

    ch = text.at(i).unicode();
    if (ch == ':') i++;

    if (i + 2 > n) return QDateTime();
    mm = text.mid(i, 2).toInt(&ok);
    if (!ok) return QDateTime();

    return date.addSecs(delta * (hh * 60 + mm) * 60);
}

QString QDocView::toIsoDate(const QDateTime& date)
{
    return date.toUTC().toString("yyyy-MM-dd'T'hh:mm:ss'Z'");
}

/*-------------------------------------------------------------------------*/

void QDocView::staticInit()
{
#ifndef DISABLE_ADOBE_SDK
    QAdobeDocView::staticInit();
#endif
}

void QDocView::staticDone()
{
    qDebug() << Q_FUNC_INFO;
#ifndef DISABLE_ADOBE_SDK
    QAdobeDocView::staticDone();
#endif
}

void QDocView::setDictionaryMode(bool state)
{
    m_dictionaryMode = state;
}

void QDocView::setHighlightMode(bool state)
{
    m_highlightMode = state;
}

void QDocView::setBookInfo(const BookInfo &bookInfo)
{
    m_bookInfo = &bookInfo;
}

QString QDocView::qlabelEntityEncode(const QString & text)
{
    QString encodeText;
    int size = text.size();
    for(int i = 0; i < size; ++i)
    {
       int code = (int)text.at(i).unicode();

        if(60 == code)
            encodeText.append("&lt;");// now only handle "<" character entity for QLabel
        else
            encodeText.append(QChar(code));
    }

    return encodeText;
}

void QDocView::setFormActivated(bool)
{
   return;
}
