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

#include "QAdobeDocView.h"

#include "QAdobeClient.h"
#include "AdobeDRM.h"
#include "QBook.h"
#include "Screen.h"
#include "BookInfo.h"
#include "ViewerFloatingNote.h"
#include "QBookApp.h"
#include "Viewer.h"
#include "DeviceInfo.h"

#include <dp_fuse.h>
#include <dp_drm.h>
#include <dp_res.h>


#include <QSettings>
#include <QTime>
#include <QApplication>
#include <QTimerEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEngine>
#include <QPixmap>
#include <QFile>
#include <QDebug>

#include <cmath>

#include <zlib.h>
#include <epub.h>
#include <epub_shared.h>
#include <QBasicTimer>
#include <QDir>
#include <QFileInfo>
#include <QFontDatabase>
#include <QSettings>

#define Q_MODE_DEFAULT          MODE_REFLOW_PAGES

#define THUMBNAIL_WIDTH 768
#define THUMBNAIL_HEIGHT 1024

double Q_FONT_SIZE_BEST = 20.0;
double Q_FONT_SIZE_MIN = 16.0;
double Q_FONT_SIZE_MAX = 51.0;
double EPUB_INTERNAL_SIZE = 5.0;
double PDF_INTERNAL_SIZE = 0.5;

#define EPUB_FONT_SIZE_MAX_LEVEL 7
#define EPUB_FONT_SIZE_MIN_LEVEL 0
#define EPUB_FONT_SIZE_BEST_LEVEL 1
const double EPUB_FONT_SIZE_600[EPUB_FONT_SIZE_MAX_LEVEL+1] = {16.0, 20.0, 24.0, 27.0, 31.0, 36.0, 42.0, 51.0};
const double EPUB_FONT_SIZE_758[EPUB_FONT_SIZE_MAX_LEVEL+1] = {21.0, 27.0, 32.0, 37.0, 42.0, 50.0, 58.0, 66.5};
const double EPUB_FONT_SIZE_1072_QP2[EPUB_FONT_SIZE_MAX_LEVEL+1] = {30.0, 38.0, 45.0, 52.0, 59.0, 71.0, 82.0, 94.0};
const double EPUB_FONT_SIZE_1072[EPUB_FONT_SIZE_MAX_LEVEL+1] = {7.0, 9.0, 11.0, 13.0, 14.0, 17.0, 19.0, 23.0};
const double* EPUB_FONT_SIZE = 0;

#define Q_REFLOW_SCALE_BEST     2.0
#define Q_REFLOW_SCALE_MIN      2.0

#define Q_SCROLL_FUZZY_GAP      2
#define Q_SCROLL_INTERVAL       10

#define Q_HILI_ACTIVE           0x8f8f8f
#define Q_HILI_NORMAL           0xafafaf

#define Q_CLICK_FUZZY_LEN       15

#ifdef QADOBESURFACE_RGB32
    #define Q_SURFACE_FORMAT    Format_RGB32
    #define Q_SURFACE_SIGNATURE dpdoc::PL_BGRX
    #define Q_BYTES_PER_PIXEL   4
#else
    #define Q_SURFACE_FORMAT    Format_Indexed8
    #define Q_SURFACE_SIGNATURE dpdoc::PL_L
    #define Q_BYTES_PER_PIXEL   1
#endif

extern QString qAppFileName();

static void initializeResourceURL(dp::String & resFolderURL, dp::String & fontsFolderURL)
{
        QString adobe_res_path = QString("file://") + QBook::resDirPath() + "/";
        resFolderURL = dp::String(adobe_res_path.toUtf8().constData());

        QString adobe_fonts_path = QString("file://") + QBook::fontsDirPath() + "/";
        fontsFolderURL = dp::String(adobe_fonts_path.toUtf8().constData());
}


//////////
#include <QMutex>
#include <QMutexLocker>
QMutex s_adobeConcurrentMutex;
//////////

class AdobeDocResProvider : public dpres::ResourceProvider 
{
public:
        AdobeDocResProvider( dp::String resFolder, dp::String m_fontFolder, bool verbose );

	virtual ~AdobeDocResProvider();

	/**
	 *  Request a global resource download from a given url with a Stream with at least
	 *  given capabilities. Security considerations are responsibilities of the host.
	 *  If NULL is returned, request is considered to be failed.
	 */
	virtual dpio::Stream * getResourceStream( const dp::String& urlin, unsigned int capabilities );

private:
	dp::String m_resFolder;
        dp::String m_fontsFolder;
	bool m_verbose;
};


AdobeDocResProvider::AdobeDocResProvider( dp::String resFolder, dp::String fontsFolder, bool verbose )
        : m_resFolder(resFolder),
          m_fontsFolder(fontsFolder),
          m_verbose(verbose)
{
}

AdobeDocResProvider::~AdobeDocResProvider()
{
}

dpio::Stream * AdobeDocResProvider::getResourceStream( const dp::String& urlin, unsigned int capabilities )
{
        dp::String url = urlin;

        if( m_verbose )
                printf( "Loading %s\n", url.utf8() );
        if( ::strncmp( url.utf8(), "data:", 5 ) == 0 )
                return dpio::Stream::createDataURLStream( url, NULL, NULL );

        // resources: user stylesheet, fonts, hyphenation dictionaries and resources they references
        if( ::strncmp( url.utf8(), "res:///", 7 ) == 0 && url.length() < 1024)
        {
            char tmp[2048];
            ::strcpy( tmp, "file://");
            /* We store fonts in a separate dir than resources */
            if ( ::strncmp( url.utf8(), "res:///fonts", strlen("res:///fonts") ) == 0 ) {
                ::strcat( tmp, QBook::fontsDirPath().toUtf8());
                ::strcat( tmp, "/" );
                ::strcat( tmp, url.utf8() + strlen("res:///fonts"));
            } else {
                ::strcat( tmp, QBook::resDirPath().toUtf8());
                ::strcat( tmp, "/" );
                ::strcat( tmp, url.utf8() + strlen("res:///"));
            }
            url = dp::String( tmp );
        }
        dpio::Partition * partition = dpio::Partition::findPartitionForURL( url );
        if( partition != NULL ){
                return partition->readFile( url, NULL, capabilities );
        }
        return NULL;
}


class QAdobeSurface : public QImage, public dpdoc::Surface
{
public:

    QAdobeSurface(const QAdobeSurface& that);
    QAdobeSurface(int width, int height, const QColor& color);
    virtual ~QAdobeSurface() {}

    virtual int getSurfaceKind() { return dpdoc::SK_RASTER; }
    virtual int getPixelLayout() { return Q_SURFACE_SIGNATURE; }
    virtual unsigned char* getTransferMap(int) { return 0; }
    virtual unsigned char* getDitheringClipMap(int) { return 0; }
    virtual int getDitheringDepth(int) { return 0; }
    virtual unsigned char* checkOut(int xMin, int yMin, int xMax, int yMax, size_t* stride);
    virtual void checkIn(unsigned char*) {}
    void clear(const QRect& r);

private:
    QColor m_color;
};

QAdobeSurface::QAdobeSurface(const QAdobeSurface& that)
    : QImage(that)
    , dpdoc::Surface()
    , m_color(that.m_color)
{
    // do nothing
}

QAdobeSurface::QAdobeSurface(int width, int height, const QColor& color)
    : QImage(width, height, Q_SURFACE_FORMAT)
    , m_color(color)
{
#ifdef QADOBESURFACE_RGB32
    fill(m_color.rgb());
#else
    setNumColors(256);
    for (int i = 0; i < 256; i++) {
        setColor(i, qRgb(i, i, i));
    }
    fill(m_color.blue());
#endif
}

unsigned char* QAdobeSurface::checkOut(int xMin, int yMin, int, int, size_t* stride)
{
	*stride = bytesPerLine();
	return scanLine(yMin) + xMin * Q_BYTES_PER_PIXEL;
}

void QAdobeSurface::clear(const QRect& box)
{
    int x0 = qMax(0, box.left());
    int x1 = qMin(width(), box.right() + 1);
    int y0 = qMax(0, box.top());
    int y1 = qMin(height(), box.bottom() + 1);
    if (x0 >= x1 || y0 >= y1) return;

    int b = m_color.blue();

#ifdef QADOBESURFACE_RGB32
    int g = m_color.green();
    int r = m_color.red();
    int a = m_color.alpha();
#endif

    for (int y = y0; y < y1; y++) {
        uchar *p = scanLine(y) + x0 * Q_BYTES_PER_PIXEL;
        for (int x = x0; x < x1; x++) {
            *p++ = b;
#ifdef QADOBESURFACE_RGB32
            *p++ = g;
            *p++ = r;
            *p++ = a;
#endif
        }
    }
}

class QAdobeEvent : public dpdoc::Event
{
public:
    QAdobeEvent(int kind, int type) 
        : m_kind(kind)
        , m_type(type)
        , m_rejected(false) 
        {}

    virtual int getEventKind() { return m_kind; }
    virtual int getEventType() { return m_type; }
    virtual void reject() { m_rejected = true; }
    bool isRejected() const { return m_rejected; }

private:
    int m_kind;
    int m_type;
    bool m_rejected;
};

class QAdobeMouseEvent : public dpdoc::MouseEvent
{
public:
    QAdobeMouseEvent(int type, int x, int y) 
        : m_type(type)
        , m_x(x)
        , m_y(y)
        , m_rejected(false) 
        {}

    virtual int getEventKind() { return dpdoc::EK_MOUSE; }
    virtual int getEventType() { return m_type; }
    virtual void reject() { m_rejected = true; }
    bool isRejected() const { return m_rejected; }

    virtual int getButton() { return 0; }
    virtual unsigned int getModifiers() { return dpdoc::MF_LEFT; }
	virtual int getX() { return m_x; }
	virtual int getY() { return m_y; }

private:
    int m_type;
    int m_x;
    int m_y;
    bool m_rejected;
};

class QAdobeTimer : public QObject, public dptimer::Timer
{
public:
    virtual void release() { delete this; }
    virtual void setTimeout(int millis) { m_timer.start(millis, this); }
    virtual void cancel() { m_timer.stop(); }

protected:
    virtual void timerEvent(QTimerEvent* event);

private:
    QBasicTimer m_timer;
};

void QAdobeTimer::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == m_timer.timerId()) {
        event->accept();
        m_timer.stop();
        dp::timerGetMasterClient()->timerFired(this);
    } else {
        QObject::timerEvent(event);
    }
}

/*-------------------------------------------------------------------------*/

bool QAdobeDocView::staticInit()
{
    static QAdobeTimer master_timer;
    static bool inited = false;
    static bool failed = false;

    if (inited) return true;
    if (failed) return false;

    int err = dp::platformInit(dp::PI_GENERIC_DEVICES);
    if (err != dp::IS_OK) {
        failed = true;
        return false;
    }

    dp::setVersionInfo("product", "bq Reader");
	dp::setVersionInfo("jpeg", "62");

	dp::cryptRegisterOpenSSL();
	dp::deviceRegisterPrimary();
	if (!dpdev::isMobileOS()) dp::deviceRegisterExternal();
	dp::deviceMountRemovablePartitions();
	dp::documentRegisterEPUB();
	dp::documentRegisterPDF();

	dp::String resFolderURL;
    dp::String fontsFolderURL;
    initializeResourceURL(resFolderURL, fontsFolderURL);
    AdobeDocResProvider* resProvider = new AdobeDocResProvider( resFolderURL, fontsFolderURL, false ) ;
    dpres::ResourceProvider::setProvider(resProvider);

    dp::timerRegisterMasterTimer(&master_timer);

    // Init the DRM
    AdobeDRM::staticInit();

    inited = true;
    return true;
}

void QAdobeDocView::staticDone()
{
    qDebug() << Q_FUNC_INFO;
    AdobeDRM::staticDone();
}

QAdobeDocView* QAdobeDocView::create(const QString& mimeType, QWidget* parent)
{
    if (!staticInit()) return 0;
    QAdobeClient* client = new QAdobeClient();
    dpdoc::Document* doc = dpdoc::Document::createDocument(client, mimeType.toAscii().data());
    if (doc) return new QAdobeDocView(doc, client, parent);
    delete client;
    return 0;
}

QPixmap* QAdobeDocView::coverPage(const QString & path, QSize& size)
{
    qDebug() << Q_FUNC_INFO << "Locking mutex";
    QMutexLocker locker(&s_adobeConcurrentMutex);
    qDebug() << Q_FUNC_INFO << "After locking mutex";

    QUrl url = QUrl::fromLocalFile(path);
    QString mime = guessMimeType(url);
    //QFileInfo fi(path);
    //bool pix;
    double Cover_w, Cover_h;

    QAdobeClient* client = new QAdobeClient();
    dpdoc::Document* doc = dpdoc::Document::createDocument(client, mime.toAscii().data());
    if (!doc) return 0;

    //QAdobeHost* host = new QAdobeHost(doc);
    //doc->setDocumentHost(host);
    doc->setURL(url.toEncoded().data());
    //doc->process();

	dpdoc::Renderer* renderer;

	renderer = doc->createRenderer(client);
	if(!renderer) return 0;

	dpdoc::Rectangle rec;
	renderer->getNaturalSize(&rec);
	Cover_w = rec.xMax- rec.xMin;
	Cover_h = rec.yMax- rec.yMin;
	
	QPixmap* pixmap = new QPixmap(size);

	QPainter painter(pixmap);

	QAdobeSurface surface(Cover_w, Cover_h, Qt::white);

	QRect t = surface.rect();

	painter.fillRect(t, Qt::white);


    // cover_loc must be released before renderer is released.
    dp::ref<dpdoc::Location> cover_loc = doc->getLocationFromPagePosition(0);

	if(!cover_loc)return 0;

	renderer->navigateToLocation(cover_loc);
    qDebug() << Q_FUNC_INFO << "Before paint";
	renderer->paint(0, 0, Cover_w, Cover_h, &surface);
    qDebug() << Q_FUNC_INFO << "After paint";
	double scale = qMin((double)pixmap->width() / Cover_w, (double)pixmap->height() / Cover_h);
	t.setWidth(Cover_w * scale);
	t.setHeight(Cover_h * scale);

	painter.drawImage(t, surface);

    renderer->release();
    doc->release();

    delete client;
    	
	return pixmap;
}



QDateTime QAdobeDocView::fromPdfDate(const QString& text)
{
    // see also http://www.verypdf.com/pdfinfoeditor/pdf-date-format.htm
    // D:YYYYMMDDHHmmSSOHH'mm'

    int n = text.length();
    if (!text.startsWith("D:") || n < 16) return QDateTime();

    bool ok = false;
    int y = text.mid(2, 4).toInt(&ok);
    if (!ok) return QDateTime();

    int m = text.mid(6, 2).toInt(&ok);
    if (!ok) return QDateTime();

    int d = text.mid(8, 2).toInt(&ok);
    if (!ok) return QDateTime();

    int hh = text.mid(10, 2).toInt(&ok);
    if (!ok) return QDateTime();

    int mm = text.mid(12, 2).toInt(&ok);
    if (!ok) return QDateTime();

    int ss = text.mid(14, 2).toInt(&ok);
    if (!ok) return QDateTime();
    if (n == 16) return QDateTime(QDate(y, m, d), QTime(hh, mm, ss));

    QDateTime date(QDate(y, m, d), QTime(hh, mm, ss), Qt::UTC);
    int ch = text.at(16).unicode();
    if (ch == 'Z') return date;

    if (n < 23) return QDateTime();
    int delta = 1;
    if (ch == '+') delta = -1;
    else if (ch != '-') return QDateTime();
    
    hh = text.mid(17, 2).toInt(&ok);
    if (!ok) return QDateTime();

    mm = text.mid(20, 2).toInt(&ok);
    if (!ok) return QDateTime();

    return date.addSecs(delta * (hh * 60 + mm) * 60);
}

QString QAdobeDocView::toPdfDate(const QDateTime& date)
{
    return date.toUTC().toString("'D:'yyyyMMddhhmmss'Z'");    
}

QString QAdobeDocView::coverPage(const QString & path, const QString & destination)
{
    qDebug() << Q_FUNC_INFO << "Locking mutex";
    QMutexLocker locker(&s_adobeConcurrentMutex);
    qDebug() << Q_FUNC_INFO << "After locking mutex";

    QUrl url = QUrl::fromLocalFile(path);
    QString mime = guessMimeType(url);
    QString filename ;
    QFileInfo fi(path);
    double Cover_w, Cover_h;
    double imageWidth = THUMBNAIL_WIDTH;
    double imageHeight = THUMBNAIL_HEIGHT;


    QAdobeClient* client = new QAdobeClient();
    dpdoc::Document* doc = dpdoc::Document::createDocument(client, mime.toAscii().data());
    qDebug()<<"[coverPage]: init fiished";
    if (!doc)
    {
        qDebug()<<"[coverpage]: entering trap 1, cover fail, using default_pdf image";
        delete client;
        QString filename = QString::fromUtf8(":/BookList/default_pdf");
        return filename;
    }
    qDebug() << "[coverPage]: phase1 done";

    doc->setURL(url.toEncoded().data());
    dp::ref<dp::ErrorList> errList = doc->getErrorList();
    if(errList->hasFatalErrors())
    {
        qDebug("[coverPage]: entering trap 1, cover fail, using default_pdf image");
        doc->release();
        delete client;
        QString filename = QString::fromUtf8(":/BookList/default_pdf");
        return filename;
    }
    qDebug() << "[coverPage]: phase2 done";

    dpdoc::Renderer* renderer;
    renderer = doc->createRenderer(client);
    if(!renderer)
    {
        qDebug()<<"[coverpage]: entering trap 2, cover fail, using default_pdf image";
        //renderer->release();
        doc->release();
        delete client;
        QString filename = QString::fromUtf8(":/BookList/default_pdf");
        return filename;
    }

    dpdoc::Rectangle rec;
    renderer->getNaturalSize(&rec);
    Cover_w = rec.xMax- rec.xMin;
    Cover_h = rec.yMax- rec.yMin;

    if(Cover_w < imageWidth || Cover_h < imageHeight){ // Small cover
        imageWidth = Cover_w;
        imageHeight = Cover_h;
    }

    QImage image(QSize(imageWidth, imageHeight), QImage::Format_RGB32);
    QPainter painter(&image);
    QAdobeSurface surface(Cover_w, Cover_h, Qt::white);
    QRect t = surface.rect();
    QRect r = image.rect();
    painter.fillRect(t, Qt::white);
    qDebug() << "[coverPage]: phase3 done";

    qDebug() << Q_FUNC_INFO << "Before paint";
    renderer->paint(0, 0, Cover_w, Cover_h, &surface);
    qDebug() << Q_FUNC_INFO << "After paint";
    double scale = qMin((double)image.width() / Cover_w, (double)image.height() / Cover_h);
    t.setWidth(Cover_w * scale);
    t.setHeight(Cover_h * scale);
    t.moveCenter(r.center());
    painter.drawImage(t, surface);
    qDebug() << "[coverPage]: phase4 done";

    if(destination.isEmpty()){
        filename = QBook::getThumbnailPath(path);
        filename = fi.absolutePath() + "/" + ".thumbnail" + "/" + fi.fileName();
        filename = filename.replace("\'", ""); //Books with apostrophe in the title cant load the thumbnail already generated.
        filename.append(".jpg");

        QDir dir = fi.dir();
        if (!dir.exists(".thumbnail"))
            dir.mkdir(".thumbnail");
    }
    else
        filename = destination;
	
    qDebug() << "saving image: "<< filename;
    bool ret = image.convertToFormat(QImage::Format_RGB32,Qt::AutoColor).scaled(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT,Qt::KeepAspectRatio,Qt::SmoothTransformation).save(filename,"JPG",90);
    qDebug() << "[coverPage] Cover Page" << (ret ? "successfully!" : "wrong") << "(End)";

    renderer->release();
    doc->release();
    delete client;
    return filename;
}

bool QAdobeDocView::isDrmFile() const
{
    bool display_drmFile = false;
    bool excerpt_drmFile = false;
    bool print_drmFile = false;

    // TODO: Hacer más eficiente. Y no tres llamadas a lo mismo.
    QString drm_display = AdobeDRM::getInstance()->getPermissionString(m_doc, "display", &display_drmFile);
    QString drm_excerpt = AdobeDRM::getInstance()->getPermissionString(m_doc, "excerpt", &excerpt_drmFile);
    QString drm_print = AdobeDRM::getInstance()->getPermissionString(m_doc, "print", &print_drmFile);

    qDebug() << Q_FUNC_INFO << drm_display << drm_excerpt << drm_print;

    if( display_drmFile || excerpt_drmFile || print_drmFile )
        return true;
    else
        return false;
}

/*-------------------------------------------------------------------------*/
// NOTE: We are leaving this commented, in case we needed for the future, because this code is not really well documented (thanks Adobe)
//static QString getMetaString(dpdoc::Document* doc, const char* name)
//{
//	dp::String data;
//	dp::ref<dpdoc::MetadataItem> meta = doc->getMetadata(name, 0);
//	if(meta)
//		data = meta->getValue();

//    if (data.length() == 0) return QString();
//    return QString::fromUtf8(data.utf8());
//}

// NOTE: We are leaving this commented, in case we needed for the future, because this code is not really well documented (thanks Adobe)
//static QString getMetaDate(dpdoc::Document* doc, const char* name)
//{
//    QString text = getMetaString(doc, name);
//    QDateTime date = QAdobeDocView::fromPdfDate(text);
//    if (date.isNull()) {
//        date = QDocView::fromIsoDate(text);
//        if (date.isNull()) {
//            bool ok = false;
//            text.toInt(&ok);
//            return ok ? text : QString();
//        }
//    }
//    return QDocView::toIsoDate(date);
//}

// NOTE: We are leaving this commented, in case we needed for the future, because this code is not really well documented (thanks Adobe)
//void QAdobeDocView::fillMetadata()
//{
//    m_metadata.title = getMetaString(m_doc, "DC.title");
//    m_metadata.creator = getMetaString(m_doc, "DC.creator");
//    m_metadata.subject = getMetaString(m_doc, "DC.subject");
//    m_metadata.description = getMetaString(m_doc, "DC.description");
//    m_metadata.publisher = getMetaString(m_doc, "DC.publisher");
//    m_metadata.contributor = getMetaString(m_doc, "DC.contributor");
//    m_metadata.date = getMetaDate(m_doc, "DC.date");
//    m_metadata.type = getMetaString(m_doc, "DC.type");
//    m_metadata.format = getMetaString(m_doc, "DC.format");
//    m_metadata.identifier = getMetaString(m_doc, "DC.identifier");
//    m_metadata.source = getMetaString(m_doc, "DC.source");
//    m_metadata.language = getMetaString(m_doc, "DC.language");
//    m_metadata.relation = getMetaString(m_doc, "DC.relation");
//    m_metadata.coverage = getMetaString(m_doc, "DC.coverage");
//    m_metadata.rights = getMetaString(m_doc, "DC.rights");

//    bool display_drmFile = false;
//    bool excerpt_drmFile = false;
//    bool print_drmFile = false;

//    m_metadata.deadline = QDocView::toIsoDate(AdobeDRM::getInstance()->expiredTime(m_doc));
//    m_metadata.drm_display = AdobeDRM::getInstance()->getPermissionString(m_doc, "display", &display_drmFile);
//    m_metadata.drm_excerpt = AdobeDRM::getInstance()->getPermissionString(m_doc, "excerpt", &excerpt_drmFile);
//    m_metadata.drm_print = AdobeDRM::getInstance()->getPermissionString(m_doc, "print", &print_drmFile);

//    qDebug() << Q_FUNC_INFO << m_metadata.deadline << m_metadata.drm_display << m_metadata.drm_excerpt << m_metadata.drm_print;

//    if( display_drmFile || excerpt_drmFile || print_drmFile )
//        m_metadata.isDRMFile = true;
//    else
//        m_metadata.isDRMFile = false;
//    /*
//    dplib::Library* lib = dplib::Library::getPartitionLibrary(0);
//    dp::ref<dplib::ContentRecord> cr = lib->createContentRecord(doc->get);
//    meta->returnable = getMetaString(doc, "ADEPT.returnable");
//    meta->fulfillment = getMetaString(doc, "ADEPT.fulfillment");
//    */

//    if (!m_metadata.isDRMFile)
//        extractCSS();
//}

/*-------------------------------------------------------------------------*/

QAdobeDocView::QAdobeDocView(dpdoc::Document* doc, QAdobeClient* client, QWidget* parent)
    : QDocView(parent)
    , m_host(client)
    , m_surface(0)
    , m_doc(doc)
    , m_renderer(0)
    , m_isBusy(false)
    , m_isFontScalable(false)
    , m_isHorizontal(false)
    , m_isHili(false)
    , m_hiliEdited(false)
    , m_tryPassHash(false)
    , m_isFileExist(true)
    , m_isDoingSetURL(false)
    , m_isFormActivated(false)
    , m_isPdf(false)
    , m_scale(1.0)
    , m_pdfZoomLevel(0)
    , m_offsetX(0)
    , m_offsetY(0)
    , m_hiliBegin(0)
    , m_hiliEnd(0)
    , m_hiliWordStart(0)
    , m_hiliWordEnd(0)
    , m_hiliOri(0)
    , m_hiliMove(0)
    , m_hiliLeft(0)
    , m_hiliRight(0)
    , m_hiliId(-1)
    , m_backColor(Qt::white)
    , m_autoFit(AUTO_FIT_PAGE)
    , m_pageMode(MODE_HARD_PAGES)
    , m_upIcon(":/reader/scroll-up")
    , m_downIcon(":/reader/scroll-down")
    , m_leftIcon(":/reader/scroll-left")
    , m_rightIcon(":/reader/scroll-right")
    , m_locNext(0)
    , m_locPrev(0)
    , m_surfaceNext(0)
    , m_surfacePrev(0)
    , m_pushOpStateCount(0)
    , m_curPos(-1)
    , m_curScale(-1)
    , m_noteFontMetrics(m_noteFont)
    , m_leftHighlightHeight(0)
    , m_rightHighlightHeight(0)
{
    setAttribute(Qt::WA_OpaquePaintEvent);

    connect(m_host, SIGNAL(stateChanged(int)), this, SLOT(updateState(int)));
    connect(m_host, SIGNAL(sizeChanged()), this, SLOT(updateNavigation()));
    connect(m_host, SIGNAL(navigationChanged()), this, SLOT(updateNavigation()));
    connect(m_host, SIGNAL(errorOccurred(const QString&)), this, SLOT(updateError(const QString&)));
    connect(m_host, SIGNAL(repaintRequested(const QRect&)), this, SLOT(updateSurface(const QRect&)));
    connect(m_host, SIGNAL(licenseRequested(const QString&, const QString&, const uchar*, size_t)), this, SLOT(handleLicense(const QString&, const QString&, const uchar*, size_t)));
    connect(m_host, SIGNAL(passwordRequested()), this, SLOT(handlePassword()));

    m_noteFont.setPixelSize(15);

    switch(QBook::getInstance()->getResolution())
    {
        case QBook::RES1072x1448:
            if(DeviceInfo::getInstance()->getHwId() == DeviceInfo::E60QH2)
                EPUB_FONT_SIZE = EPUB_FONT_SIZE_1072;
            else //E60QP2
                EPUB_FONT_SIZE = EPUB_FONT_SIZE_1072_QP2;
            EPUB_INTERNAL_SIZE = 9.0;//5.0*1.33;//6.2;//5.0*1.297619048;//6.25;
            PDF_INTERNAL_SIZE = 0.5;
            break;
        case QBook::RES758x1024:
            EPUB_FONT_SIZE = EPUB_FONT_SIZE_758;
            EPUB_INTERNAL_SIZE = 6.5;//5.0*1.33;//6.2;//5.0*1.297619048;//6.25;
            PDF_INTERNAL_SIZE = 0.5;
            break;
        case QBook::RES600x800: default:
            EPUB_FONT_SIZE = EPUB_FONT_SIZE_600;
            EPUB_INTERNAL_SIZE = 5.0;
            PDF_INTERNAL_SIZE = 0.5;
            break;
    }

    Q_FONT_SIZE_BEST = EPUB_FONT_SIZE[EPUB_FONT_SIZE_BEST_LEVEL];
    Q_FONT_SIZE_MIN = EPUB_FONT_SIZE[EPUB_FONT_SIZE_MIN_LEVEL];
    Q_FONT_SIZE_MAX = EPUB_FONT_SIZE[EPUB_FONT_SIZE_MAX_LEVEL];

    for(int i = 0; i < 8; i++)
    {
        m_fontSizeList << EPUB_FONT_SIZE[i];
    }
}

/**
  * extract all css from epub and save it at /tmp
  */
QSet<QString> QAdobeDocView::extractCSSFromEpub()
{
    // Reset the variable
    m_canOverrideFonts = true;

    // add css file spacing
    QFile spacingSettings(QUrl::fromUserInput(QBook::settings().value("setting/reader/spacing/epub", "file:///app/res/noSpacing.css").toString()).path());
    if (!spacingSettings.open(QFile::ReadOnly))
    {
        qDebug() << Q_FUNC_INFO << "Error reading file" << spacingSettings.fileName();
        return QSet<QString>();
    }

    qDebug() << Q_FUNC_INFO << "Copying file" << spacingSettings.fileName();

    QString spacing = spacingSettings.readAll().data();
    spacingSettings.close();

    spacing.append("font-size:1.00em;");
    int justifyValue = QBook::settings().value("setting/reader/justify/epub", 2).toInt();
    if(justifyValue == 0)
        spacing.append("\ntext-align:justify; text-justify:auto;");
    else if(justifyValue == 1)
        spacing.append("\ntext-align:left;");

    QSet<QString> fontFamilySet;

    QFile dataFile ("/tmp/epub.css");
    if (!dataFile.open(QFile::WriteOnly | QIODevice::Append)) {
        qDebug() << Q_FUNC_INFO << "cannot open css file for writing: " << dataFile.fileName();
        return fontFamilySet;
    }

    struct epub *book = epub_open (s_path.toLocal8Bit().data(), 0);
    if (book == NULL) {
        qDebug() << Q_FUNC_INFO << "Cannot open: " << s_path;
        // Maybe the book is corrupted.
        // We need to write the p, span and body css arguments.
    }
    else
    {
        const QStringList& cssList = m_bookInfo->getCSSFileList();
        qDebug() << Q_FUNC_INFO << "CSS List" << cssList.join(";");
        QStringList::const_iterator itEnd = cssList.end();
        for (QStringList::const_iterator it = cssList.begin(); it != itEnd; ++it)
        {
            QString cssPath(*it);
            cssPath = cssPath.remove(QRegExp("^(\\w)*/"));
            qDebug() << Q_FUNC_INFO << "extracting " << (*it);
            char *data = NULL;
            int cssSize = epub_get_data(book, cssPath.toLocal8Bit().data(), &data);
            if (cssSize <= 0 || !data) {
                qDebug() << Q_FUNC_INFO << "CSS file not found in epub:" << cssPath.toLocal8Bit().data();
                continue;
            }

            qDebug() << Q_FUNC_INFO << "Writing CSS file: " << cssPath << ": " << cssSize << " bytes";
            QString contentFile(data);


            // Removing imports
            contentFile = contentFile.replace(QRegExp("@import[^;]*;"), "");
            contentFile = contentFile.replace(QRegExp("@[^;]*;"), "");

            //Check if CSS has incorrect fields.
            checkIncorrectFieldCSS(contentFile);

            QRegExp fontFamilyRX("font-family\\s*:[\\w'-,\\s\"]*;");


            int pos = 0;
            QStringList list;
            while ((pos = fontFamilyRX.indexIn(contentFile, pos)) != -1) {
                list << fontFamilyRX.cap(0);
                pos += fontFamilyRX.matchedLength();
            }

            qDebug() << Q_FUNC_INFO << "founded families at file" << cssPath << ": " << list.size() << list.join(" -- ");
            QStringList::iterator itEnd = list.end();
            for (QStringList::iterator it = list.begin(); it != itEnd; ++it)
            {
                QString familyName(*it);
                familyName = familyName.remove(QRegExp("(font-family\\s*:|;)"));

                QStringList familyList = familyName.split(',', QString::SkipEmptyParts);
                int familyListCount = familyList.count();
                for (int i = 0; i < familyListCount; ++i)
                    fontFamilySet.insert(familyList[i].trimmed());
            }

            qDebug() << Q_FUNC_INFO << "Clean all CSS tag's";

            // Finding *{} inside the css
            if( contentFile.contains(QRegExp("\\*\\s*\\{(\\s*([a-zA-Z-])*:[^;]*;\\s*)+\\}")) )
            {
                m_canOverrideFonts = false;
            }

            // Removing imports
            contentFile = contentFile.replace(QRegExp("\\{(\\s*([a-zA-Z-])*:[^\n]*\n\\s*)+\\}"), QString("{%1}").arg(spacing));

            justifyValue = QBook::settings().value("setting/reader/justify/epub", 2).toInt();
            if(justifyValue == 0)
                contentFile = contentFile.replace(QRegExp("(text-align\\s*:[\\w'-,\\s\"]*;)"), QString("text-align: justify;"));
            else if(justifyValue == 1)
                contentFile = contentFile.replace(QRegExp("(text-align\\s*:[\\w'-,\\s\"]*;)"), QString("text-align: left;"));

            int res = dataFile.write(contentFile.toLocal8Bit().data());
            qDebug() << Q_FUNC_INFO << "Wrote " << res << " bytes";
        }
    }

    // Add at the end the p, span and body attributes
    QString p = QString("p {%1}").arg(spacing);
    QString span = QString("span {%1}").arg(spacing);
    QString body = QString("body {%1}").arg(spacing);
    dataFile.write(p.toLocal8Bit().data());
    dataFile.write(span.toLocal8Bit().data());
    dataFile.write(body.toLocal8Bit().data());

    dataFile.close();

    epub_close (book);

    qDebug() << Q_FUNC_INFO << "fontFamilySet" << QStringList(fontFamilySet.toList()).join(" -- ");
    return fontFamilySet;
}

void QAdobeDocView::cleanExtractedCSS()
{
    QDir tmpDir("/tmp");
    QFileInfoList fileList = tmpDir.entryInfoList(QStringList() << "*.css");
    QFileInfoList::iterator it = fileList.begin();
    QFileInfoList::iterator itEnd = fileList.end();
    for (; it != itEnd; ++it)
    {
        QFile::remove((*it).absoluteFilePath ());
        qDebug() << Q_FUNC_INFO << "Removing file" << (*it).absoluteFilePath ();
    }
}

void QAdobeDocView::checkIncorrectFieldCSS(QString &content)
{
    qDebug() << Q_FUNC_INFO;

    if(content.contains(QRegExp("(font-size\\s*:[^;]*pt;)")))
    {
        qDebug() << "Detect incorrect CSS";
        m_host->setIncorrectCSS(true);
        content = content.remove(QRegExp("(font-size\\s*:[^;]*;)"));

    }
    if(content.contains(QRegExp("(line-height\\s*:[^;]*pt;)")))
    {
        qDebug() << "Detect incorrect CSS";
        m_host->setIncorrectCSS(true);
        content = content.remove(QRegExp("(line-height\\s*:[^;]*;)"));
    }
}

void QAdobeDocView::addExtraCSSFiles(QSet<QString>& originalFamilySet)
{
    qDebug() << Q_FUNC_INFO;

    bool editorFonts = QBook::settings().value("setting/reader/editorFonts", true).toBool();
    if(!m_canOverrideFonts || editorFonts)
    {
        qDebug() << Q_FUNC_INFO << "We cannot override fonts";
        return;
    }


    // write the necessary font settings
    QFile dataFile ("/tmp/epub.css");
    if (!dataFile.open(QFile::WriteOnly | QIODevice::Append)) {
        qDebug() << Q_FUNC_INFO << "cannot open css file for writing: " << dataFile.fileName();
        dataFile.close();
        return;
    }

    // add css file font
    QFile fontSettings(QUrl::fromUserInput (QBook::settings().value("setting/reader/font/epub", "file:///app/res/veraUserStyle.css").toString()).path());
    if (!fontSettings.open(QFile::ReadOnly))
    {
        qDebug() << Q_FUNC_INFO << "Error reading file" << fontSettings.fileName();
        dataFile.close();
        return;
    }

    qDebug() << Q_FUNC_INFO << "Copying file" << fontSettings.fileName();

    QString fontSettingsContent = fontSettings.readAll();

    qDebug() << Q_FUNC_INFO << "Getting all font-family";
    QRegExp fontFamilyRX("font-family\\s*:[\\w'-,\\s\\\"]*;");


    int pos = 0;
    QStringList list;
    while ((pos = fontFamilyRX.indexIn(fontSettingsContent, pos)) != -1) {
        list << fontFamilyRX.cap(0);
        pos += fontFamilyRX.matchedLength();
    }

    // clean repeated and unnecessary strings
    QStringList::iterator itEnd = list.end();
    for (QStringList::iterator it = list.begin(); it != itEnd; ++it)
    {
        QString familyName(*it);
        familyName = familyName.remove(QRegExp("(font-family\\s*:|;)"));
        QStringList fList = familyName.split(',', QString::SkipEmptyParts);
        QStringList::iterator fItEnd = fList.end();
        for (QStringList::iterator fIt = fList.begin(); fIt != fItEnd; ++fIt)
        {
            originalFamilySet.insert((*fIt).trimmed());
        }
    }
    qDebug() << Q_FUNC_INFO << QStringList(originalFamilySet.toList()).join(" -- ");

    fontSettingsContent = fontSettingsContent.replace(fontFamilyRX, QString("font-family: %1;").arg(QStringList(originalFamilySet.toList()).join(", ")));
    int lastcomma = fontSettingsContent.lastIndexOf(',');
    fontSettingsContent.replace(lastcomma, 1, ";");

    int bytesWritten = dataFile.write(fontSettingsContent.toLocal8Bit().data());
    qDebug() << Q_FUNC_INFO << "write " << bytesWritten << "bytes";
    fontSettings.close();
    dataFile.close();
}

QAdobeDocView::~QAdobeDocView()
{
    releaseHighlightTracking();

    if (m_renderer) m_renderer->release();
    m_doc->release();

    delete m_host;
    delete m_surface;

    QList<ViewerFloatingNote*>::iterator it = m_floatingNotes.begin();
    QList<ViewerFloatingNote*>::iterator itEnd = m_floatingNotes.end();
    for(; it != itEnd; ++it)
    {
        delete *it;
    }
    m_floatingNotes.clear();
    m_notesLocation.clear();
}

/*-------------------------------------------------------------------------*/

QUrl QAdobeDocView::url() const
{
    return m_url;
}

void QAdobeDocView::setUrl(const QUrl& url)
{   
    releaseHighlightTracking();

    m_docSize = size();

    bool bSameUrl = m_url == url;
    QString mime = QDocView::guessMimeType(url);
	if(mime == QLatin1String("application/pdf"))
		m_isPdf = true;
	else
		m_isPdf = false;
    if(!bSameUrl && !m_isPdf)
        extractCSS();

    m_isDoingSetURL = true;
    m_doc->setURL(url.toEncoded().data());
    m_isDoingSetURL = false;
    m_url = url;
    updateTitle();

    emit urlChanged(m_url);
    emit aspectRatio(documentRect());
}

QString QAdobeDocView::backend() const
{
    return "adobe";
}

QString QAdobeDocView::title() const
{
    return m_title;
}

void QAdobeDocView::extractCSS()
{
    // clean all previous extracted css
    cleanExtractedCSS();
    QSet<QString> originalFamilySet(extractCSSFromEpub());
    addExtraCSSFiles(originalFamilySet);
}

QDocView::PageMode QAdobeDocView::pageMode() const
{
    return m_pageMode;
}

bool QAdobeDocView::isPageModeSupported(PageMode mode)
{
    return !m_isFontScalable || mode == MODE_HARD_PAGES;
}

bool QAdobeDocView::isHardPageMode() const
{
    return m_pageMode == MODE_HARD_PAGES || m_pageMode == MODE_HARD_PAGES_2UP;
}

bool QAdobeDocView::isHardModePDF() const
{
     return m_isPdf && isHardPageMode();
}

bool QAdobeDocView::setPageMode(PageMode mode)
{
    if (!m_renderer) return false;

    PageMode previousPageMode = m_pageMode;

    int hint = dpdoc::PM_HARD_PAGES;
    switch (mode) {
    case MODE_HARD_PAGES_2UP:
        hint = dpdoc::PM_HARD_PAGES_2UP;
        qDebug() << Q_FUNC_INFO << "Setting MODE_HARD_PAGES_2UP";
        break;
    case MODE_SCROLL_PAGES:
        hint = dpdoc::PM_SCROLL_PAGES;
        qDebug() << Q_FUNC_INFO << "Setting MODE_SCROLL_PAGES";
        break;
    case MODE_SCROLL:
        hint = dpdoc::PM_SCROLL;
        qDebug() << Q_FUNC_INFO << "Setting MODE_SCROLL";
        break;
    case MODE_REFLOW_PAGES:
        hint = dpdoc::PM_FLOW_PAGES;
        qDebug() << Q_FUNC_INFO << "Setting MODE_REFLOW_PAGES";
        break;
    default:
        hint = dpdoc::PM_HARD_PAGES;
        qDebug() << Q_FUNC_INFO << "Setting MODE_HARD_PAGES";
        break;
    }
    
    dp::ref<dpdoc::Location> loc = 0;
    if(!m_isDoingSetURL)
      loc = m_renderer->getScreenBeginning();
    
    m_renderer->setPagingMode(hint);
    int result = m_renderer->getPagingMode();

    switch (result)
    {
    case dpdoc::PM_HARD_PAGES_2UP:  m_pageMode = MODE_HARD_PAGES_2UP; break;
    case dpdoc::PM_SCROLL_PAGES:    m_pageMode = MODE_SCROLL_PAGES; break;
    case dpdoc::PM_SCROLL:          m_pageMode = MODE_SCROLL; break;
    case dpdoc::PM_FLOW_PAGES:      m_pageMode = MODE_REFLOW_PAGES; break;
    default:                        m_pageMode = MODE_HARD_PAGES; break;
    }

    if (m_pageMode == MODE_SCROLL_PAGES)
    {
        dpdoc::PageDecoration deco;
        deco.pageGap = 1;
        deco.borderWidth = 1;
        deco.shadowWidth = 0;
        deco.borderColorBR = 0;
        deco.borderColorTL = 0x808080;
        m_renderer->setPageDecoration(deco);
    }

    if (m_pageMode == MODE_REFLOW_PAGES) emit pageModeReflow();

    updateOffsetXY();
    setAutoFitMode(AUTO_FIT_BEST);
    if(loc)
    {
    #ifdef Q_WS_QWS
    m_renderer->navigateToLocation(loc);
    #endif
    }

    if (m_isPdf && previousPageMode == MODE_REFLOW_PAGES) takeMiniatureScreenshot();

    return true;

}

/*-------------------------------------------------------------------------*/

void QAdobeDocView::displayFit(AutoFitMode mode)
{
    setScaleFactor(autoFitFactor(mode));
}

double QAdobeDocView::autoFitFactor(AutoFitMode mode) const
{
    qDebug() << Q_FUNC_INFO << mode;
    if (!m_renderer || !m_surface || mode == AUTO_FIT_NONE) return m_scale;

    if (m_isFontScalable) {
        return Q_FONT_SIZE_MIN;
    }

    if (m_pageMode == MODE_REFLOW_PAGES) {
        return (mode == AUTO_FIT_PAGE) ? Q_REFLOW_SCALE_MIN : Q_REFLOW_SCALE_BEST;
    }

    if (mode == AUTO_FIT_BEST) {
        mode = (Screen::getInstance()->screenOrentation() == Screen::ROTATE_0) ? AUTO_FIT_PAGE : AUTO_FIT_WIDTH;
    }

    switch (mode) {
    case AUTO_FIT_PAGE:
        if (isHardPageMode())
            return qMin(m_surface->width() / m_docSize.width(), m_surface->height() / m_docSize.height());

    case AUTO_FIT_WIDTH:
        return m_surface->width() / m_docSize.width();
    case AUTO_FIT_HEIGHT:
        return m_surface->height() / m_docSize.height();
    default:
        return 1.0;
    }
}

QDocView::AutoFitMode QAdobeDocView::autoFitMode() const
{
    return m_autoFit;
}

void QAdobeDocView::setAutoFitMode(AutoFitMode mode)
{
    qDebug() << Q_FUNC_INFO << "autofit mode: " << mode;
    m_autoFit = mode;
    updateViewport();
}

double QAdobeDocView::pdfScaleStep() const
{
    return (MAX_PDF_ZOOM - 1)*minScaleFactor()/(MAX_PDF_ZOOM_LEVEL - 1);
}

double QAdobeDocView::epubScaleStep() const
{
    return EPUB_INTERNAL_SIZE;
}

double QAdobeDocView::scaleStep() const
{
    return m_isFontScalable ? epubScaleStep() : pdfScaleStep();
}

double QAdobeDocView::maxScaleFactor() const
{
    return m_isFontScalable ? Q_FONT_SIZE_MAX : pdfScaleStep()*(MAX_PDF_ZOOM_LEVEL - 1) + minScaleFactor();
}

double QAdobeDocView::minScaleFactor() const 
{
    if (m_isPdf){
        if (m_pageMode == MODE_REFLOW_PAGES)
            return Q_REFLOW_SCALE_MIN;
        else
            return autoFitFactor(AUTO_FIT_PAGE);
    }
    else // EPUB
        return Q_FONT_SIZE_MIN;
}

double QAdobeDocView::scaleFactor() const
{
    return m_scale;
}

bool QAdobeDocView::setScaleFactorOnNextZoomedPage(double factor, double delta_x, double delta_y)
{
    qDebug() << Q_FUNC_INFO << "factor: " << factor;
    QRectF r = documentRect();

    if (m_surface)
    {
        m_offsetX = 0.0;
        m_offsetY = 0.0;
    }
    else
    {
        m_offsetX = 0;
        m_offsetY = 0;
    }

    return updateScaleAndView(factor);
}

bool QAdobeDocView::setScaleFactorOnPreviousZoomedPage(double factor, double delta_x, double delta_y)
{
    qDebug() << Q_FUNC_INFO << "factor: " << factor << "delta_x: " << delta_x << "delta_y: " << delta_y;
    QRectF r = documentRect();

    if (m_surface)
    {
        m_offsetX = 0.0;
        m_offsetY = 1.0;
    }
    else
    {
        m_offsetX = 0;
        m_offsetY = 0;
    }

    return updateScaleAndView(factor);
}

 bool QAdobeDocView::setScaleFactorGoToZoomedPage(double factor)
 {
     qDebug() << Q_FUNC_INFO << "factor";

     m_offsetX = 0;
     m_offsetY = 0;

     return updateScaleAndView(factor);
 }

bool QAdobeDocView::setScaleFactor(double factor, double delta_x, double delta_y)
{
    qDebug() << Q_FUNC_INFO << "factor: " << factor << "delta_x: " << delta_x << "delta_y: " << delta_y;
    QRectF r = documentRect();

    if (m_surface)
    {
        m_offsetX = (-r.x() + delta_x * m_surface->width()) / r.width();
        m_offsetY = (-r.y() + delta_y * m_surface->height()) / r.height();
    }
    else
    {
        m_offsetX = 0;
        m_offsetY = 0;
    }

    return updateScaleAndView(factor);
}

bool QAdobeDocView::updateScaleAndView(double factor)
{
    m_curFitFactor= autoFitFactor(AUTO_FIT_PAGE);
    if ((factor <= m_curFitFactor) && m_isPdf )
    {
        updateScale(m_curFitFactor);
        m_autoFit = AUTO_FIT_PAGE;
    }
    else
    {
        updateScale(factor);
        m_autoFit = AUTO_FIT_NONE;
    }

    if (m_isPdf && !m_isFontScalable) emit pdfZoomLevelChange(sizeLevel());

    emit zoomChange(m_scale);

    bool result = updateViewport();

    if (m_isPdf && !m_isFontScalable) positioningMiniature();

    return result;
}

void QAdobeDocView::updateScale(const double factor)
{
    m_scale = factor;

    // Update m_pdfZoomLevel
    m_pdfZoomLevel = qRound((m_scale - autoFitFactor(AUTO_FIT_PAGE)) / pdfScaleStep());
    if(m_pdfZoomLevel < 0 || m_pdfZoomLevel > (MAX_PDF_ZOOM_LEVEL - 1)) m_pdfZoomLevel = 0;
}

/*-------------------------------------------------------------------------*/

QString QAdobeDocView::highlightToBookmark(dp::ref<dpdoc::Location> start,  dp::ref<dpdoc::Location> end) const
{
    qDebug() << Q_FUNC_INFO;
    return QString("#HL|%1|%2")
        .arg(QString::fromUtf8(start->getBookmark().utf8())) 
        .arg(QString::fromUtf8(end->getBookmark().utf8()));
}

bool QAdobeDocView::bookmarkToHighlight(const QString& ref, dp::ref<dpdoc::Location>* start, dp::ref<dpdoc::Location>* end) const
{
    QStringList args = ref.split(QChar('|'));
    if (args[0] == "#HL") {
        if (start) *start = m_doc->getLocationFromBookmark(dp::String(args[1].toUtf8().constData()));
        if (end) *end = m_doc->getLocationFromBookmark(dp::String(args[2].toUtf8().constData()));
        return true;
    } else {
        if (start) *start = 0;
        if (end) *end = 0;
        return false;
    }
}

void QAdobeDocView::releaseHighlightTracking()
{
	m_hiliBegin = 0;
	m_hiliEnd = 0;
	m_hiliOri = 0;
	m_hiliMove = 0;
	m_hiliWordStart = 0;
	m_hiliWordEnd = 0;
	m_hiliLeft = 0;
    m_hiliRight = 0;
}

void QAdobeDocView::setFileExist(bool isExist)
{
	m_isFileExist = isExist;
}

// TODO: remove
bool QAdobeDocView::isOpenPWDialog()
{
        return false ;
}

bool QAdobeDocView::isHighlightSupported() const
{
    return true;
}

int QAdobeDocView::trackHighlight(HighlightMode mode, int x, int y)
{
    if (!m_renderer) return -1;

    if (mode == HIGHLIGHT_COMMIT) {
        releaseHighlightTracking();
        return -1;
    }

    m_transInv.map(x, y, &x, &y);
    m_hiliMove = 0;
    m_hiliMove = m_renderer->hitTest(x, y, dpdoc::HF_SELECT | dpdoc::HF_FORCE);
    if (!m_hiliMove)
	{
		//mouse release in the range that out of content
		if (mode == HIGHLIGHT_END && m_isHili)
		{
            handleHiliUnion();
            m_isHili = false;
            repaint();
	    }
		return -1;
    }

	m_isHili = true;
	bool isGetWord = false;
	
    if(mode > HIGHLIGHT_BEGIN)
    {
        if(m_wordRect.contains(x, y))
        {
            qDebug() << "hit inside the same word";
            if(mode == HIGHLIGHT_END)
            {
               if(m_hiliBegin != 0 && m_hiliEnd != 0)
               {
                handleHiliUnion();
                goto END;
               }
               else{
                   releaseHighlightTracking();
                   return -1;
               }

            }
            else
            {
                if(mode == HIGHLIGHT_TRACK) m_tempHighlightHeight = m_wordRect.height();

                return -1;
			}
		}
	}

	//find whole word for epub
    //if(!m_isPdf)
    if (1) // enable finding whole word for pdf
	{
		QString word = wordAt(x, y);
        qDebug() << "get word: " << word << " in (" << x << "," << y << ")";
        if(word == QString()) return NOTHING_TRACK_ERROR;
		dpdoc::PageNumbers pagenum;
		m_renderer->getPageNumbersForScreen(&pagenum);		
		dp::ref<dpdoc::Location> searchStart = m_isPdf ? m_doc->getLocationFromPagePosition(pagenum.beginning) : m_renderer->getScreenBeginning();
		dp::ref<dpdoc::Location> searchEnd = 0;
		if(m_isPdf)
		{
			if(pagenum.beginning == m_doc->getPageCount() - 1)
				searchEnd = m_doc->getEnd();
			else {
				searchEnd = m_doc->getLocationFromPagePosition(pagenum.beginning + 1);
			}
		}
        else
             searchEnd = m_renderer->getScreenEnd();

		//prevent adobe bug :getScreenEnd() won't get the real end 
		if(m_hiliMove->compare(searchEnd) == 1)
			searchEnd = m_hiliMove;
		
		bool isNotFind = true;
		while(isNotFind)
		{
			
			dpdoc::Range range;
			if(!m_doc->findText(searchStart, searchEnd, dpdoc::SF_MATCH_CASE || dpdoc::SF_WHOLE_WORD, word.toUtf8().constData(), &range))
				return -1;
			m_hiliWordStart =  range.beginning;
			m_hiliWordEnd = range.end;
			
			//wordStart <= move <= wordEnd
			if(m_hiliMove->compare(m_hiliWordStart) != -1 && m_hiliMove->compare(m_hiliWordEnd) != 1 )
			{
				isNotFind = false;
				dpdoc::RangeInfo* rInfo = m_renderer->getRangeInfo(m_hiliWordStart, m_hiliWordEnd);
                                if (rInfo)
                                {
                                    dpdoc::Rectangle rect;
                                    rInfo->getBox(0, false, &rect);
                                    int x = static_cast<int>(rect.xMin);
                                    int y = static_cast<int>(rect.yMin);
                                    rInfo->getBox(rInfo->getBoxCount() - 1, false, &rect);
                                    int w = static_cast<int>(rect.xMax) - x;
                                    int h = static_cast<int>(rect.yMax) - y;
                                    m_wordRect = QRect(x, y, w, h);
                                }
			}
			else
			{
				searchStart = m_hiliWordEnd;
				m_hiliWordStart = 0;
				m_hiliWordEnd = 0;
			}
		}

        m_tempHighlightHeight = m_wordRect.height();
		isGetWord = true;
	}

    if (mode == HIGHLIGHT_BEGIN) {
        m_hiliId = -1;
		m_hiliOri = m_renderer->hitTest(x, y, dpdoc::HF_SELECT | dpdoc::HF_FORCE);

        //if(!m_isPdf)
        if (1) // enable finding whole word for pdf
		{
			m_hiliLeft = m_hiliWordStart;
			m_hiliRight = m_hiliWordEnd;
			m_hiliBegin = m_hiliLeft;
			m_hiliEnd = m_hiliRight;
	        if(isGetWord)
				m_hiliEdited = true;
			repaint();
		}
		else
			m_hiliBegin = m_hiliOri;

    } else if (mode == HIGHLIGHT_TRACK) {
        if (m_hiliBegin) {
			m_hiliEdited = true;
			 if (m_hiliMove->compare(m_hiliOri) == 1)
			{
              //if(!m_isPdf)
                if (1) // enable finding whole word for pdf
				{
					m_hiliBegin = m_hiliLeft;
				   	m_hiliEnd = m_hiliWordEnd;
				}
				else
				{
					m_hiliBegin = m_hiliOri;
					m_hiliEnd = m_hiliMove;
				}
			}
			else
			{
              //if(!m_isPdf)
                if (1) // enable finding whole word for pdf
				{
					m_hiliEnd = m_hiliRight;
					m_hiliBegin = m_hiliWordStart;
				}
				else
				{
					m_hiliEnd = m_hiliOri;
					m_hiliBegin = m_hiliMove;
				}
			}
			repaint();
        }
    } else if (mode == HIGHLIGHT_END) {
	    m_rectList.clear();
        if(m_hiliBegin != 0 && m_hiliEnd != 0)
            handleHiliUnion();
		m_isHili = false;		
        releaseHighlightTracking();
    }
	END:
    qDebug() << "Return Highlight ID: " << m_hiliId;
	if (mode == HIGHLIGHT_BEGIN) return -2;
    return m_hiliId;
}

void QAdobeDocView::handleHiliUnion()
{
	m_hiliId = m_renderer->addHighlight(dpdoc::HT_ANNOTATION, m_hiliBegin, m_hiliEnd);
	if(m_hiliId < 0) return;
	m_renderer->setHighlightColor(dpdoc::HT_ANNOTATION, m_hiliId, Q_HILI_NORMAL);
	dp::ref<dpdoc::Location> neighborStart = 0;
	dp::ref<dpdoc::Location> neighborEnd = 0;
	QList<int> unionList;
	int hiliNum = m_renderer->getHighlightCount(dpdoc::HT_ANNOTATION);
	bool blockUnion = false;

	//scan overlap highlight
	//case 1:
	//  ______      _____                    _____   _____
	// |__1__|     |__3_|                  |_4__| |_5__|
	//     __________________________________
	//    |___________add_2_________________|
	//
	//case 2:
	//     ___________________________________
	//    |_____________6____________________|
	//                   ___________
	//                  |__add_7____|    

	if(m_hiliId > 0)
	{
		dpdoc::Range range;
		m_renderer->getHighlight(dpdoc::HT_ANNOTATION, m_hiliId - 1, &range);
		neighborStart = range.beginning;
		neighborEnd = range.end;
		// block 1, 6
		//hiliBegin <= neightborEnd
		if(m_hiliBegin->compare(neighborEnd) < 1)		
		{
			blockUnion = true;
			m_hiliBegin = neighborStart;
			unionList.append(m_hiliId);
			unionList.append(m_hiliId - 1);
			//block 6
			//hiliEnd > neighborEnd
			if(m_hiliEnd->compare(neighborEnd)!= 1)
				m_hiliEnd = neighborEnd;
		}
	}

	if(m_hiliId < hiliNum)
	{
		int i = 1;
		bool scan = true;
		//handle bloc 3, 4, 5 uinion block 2
		while(scan && m_hiliId + i < hiliNum)
		{	
			dpdoc::Range range;
			m_renderer->getHighlight(dpdoc::HT_ANNOTATION, m_hiliId + i, &range);
			neighborStart = range.beginning;
			neighborEnd = range.end;
			//block 3, 4
			//neighborStart <= hiliEnd
			if(neighborStart->compare(m_hiliEnd) < 1)
			{				
				if(!blockUnion)
					unionList.append(m_hiliId);
				unionList.prepend(m_hiliId + i);
				// block 5
				//neighborEnd > hiliEnd
				if(neighborEnd->compare(m_hiliEnd) >= 1)
				{
					m_hiliEnd = neighborEnd;
					scan = false;
				}
			}
			else
				scan = false;
			++i;
		}
	}

	//remove overlap highlight
	//Re-Highlight
	if(unionList.length() > 0)
	{		
		for(int i = 0; i < unionList.length(); ++i)
			m_renderer->removeHighlight(dpdoc::HT_ANNOTATION, unionList.at(i));

		if( (m_hiliId = m_renderer->addHighlight(dpdoc::HT_ANNOTATION, m_hiliBegin, m_hiliEnd)) >= 0)
			m_renderer->setHighlightColor(dpdoc::HT_ANNOTATION, m_hiliId, Q_HILI_NORMAL);
	}	
}

bool QAdobeDocView::isHiliEdited()
{
	return m_hiliEdited;
}

void QAdobeDocView::setEdited(bool edited)
{
	m_hiliEdited = edited;
}

QStringList QAdobeDocView::highlightList() const
{
    QStringList list;
    if (!m_renderer) return list;

    int n = m_renderer->getHighlightCount(dpdoc::HT_ANNOTATION);
    for (int i = 0; i < n; ++i)
    {
        dpdoc::Range range;
        if (m_renderer->getHighlight(dpdoc::HT_ANNOTATION, i, &range))
        {
            list += highlightToBookmark(range.beginning, range.end);
        }
    }

    return list;
}

void QAdobeDocView::setHighlightList(const QStringList& list)
{
    qDebug() << Q_FUNC_INFO << "Locking mutex";
    QMutexLocker locker(&s_adobeConcurrentMutex);
    qDebug() << Q_FUNC_INFO << "After locking mutex";

    if (!m_renderer) return;

    releaseHighlightTracking();
    m_renderer->removeAllHighlights(dpdoc::HT_ANNOTATION);
    
    int size = list.size();
    for (int i = 0; i < size; ++i)
    {
	 dp::ref<dpdoc::Location> start = 0;
	 dp::ref<dpdoc::Location> end = 0;

        bookmarkToHighlight(list.at(i), &start, &end);
        if (start && end) {
            int id = m_renderer->addHighlight(dpdoc::HT_ANNOTATION, start, end);
			m_renderer->setHighlightColor(dpdoc::HT_ANNOTATION, id, Q_HILI_NORMAL);
        }
    }
}

int QAdobeDocView::highlightCount() const
{
    if (!m_renderer) return 0;
    return m_renderer->getHighlightCount(dpdoc::HT_ANNOTATION);
}

int QAdobeDocView::highlightAt(QPoint p) const
{
    return highlightAt(p.x(), p.y());
}

int QAdobeDocView::highlightAt(int x, int y) const
{
    if (!m_renderer) return -1;
    m_transInv.map(x, y, &x, &y);
    
    
    dp::ref<dpdoc::Location> loc = 0;
    loc = m_renderer->hitTest(x, y, dpdoc::HF_SELECT | dpdoc::HF_FORCE);

    if (!loc) return -1;

    int hili = -1;
    int n = m_renderer->getHighlightCount(dpdoc::HT_ANNOTATION);

    for (int i = 0; i < n; i++)
    {
        dpdoc::Range range;
        if (!m_renderer->getHighlight(dpdoc::HT_ANNOTATION, i, &range)) {
            continue;
        }

        if (range.beginning->compare(loc) <= 0 && loc->compare(range.end) <= 0) {
            hili = i;
        }

        if (hili != -1) break;
    }

    // Check notes
    if ( hili == -1 )
    {
        QRectF rect = documentRect();
        int originX = rect.left(), originY = rect.top();

        int count = m_notesLocation.count();
        for (int i = 0; i < count; ++i)
        {
            const QPair<double, double> loc = m_notesLocation.at(i);

            QString label = QString::number(m_previousNotesCount + m_notes.at(i) + 1);
            int labelW = m_noteFontMetrics.width(label) + 8;
            int labelH = m_noteFontMetrics.height() + 8;
            int labelX = loc.first + originX - (labelW>>1);
            int labelY = loc.second + originY;

            if ( labelX <= x && x <= labelX + labelW && labelY <= y && y <= labelY + labelH )
            {
                hili = m_notes.at(i);
                break;
            }
        }
    }

    return hili;
}

void QAdobeDocView::removeHighlight(int hili)
{
    if (!m_renderer) return;
    releaseHighlightTracking();
    if (hili < 0) return;
    m_renderer->removeHighlight(dpdoc::HT_ANNOTATION, hili);
}

bool QAdobeDocView::eraseHighlight(int x,int y)
{
	int id = highlightAt(x, y);
	qDebug() << "Highlight ID: " << id;
	if(id >= 0)
	{
		removeHighlight(id);
		m_hiliEdited = true;
		return true;
	}
	return false;
}

void QAdobeDocView::clearHighlightList()
{
    if (!m_renderer) return;
    releaseHighlightTracking();
    m_renderer->removeAllHighlights(dpdoc::HT_ANNOTATION);
}

QDocView::Location* QAdobeDocView::highlightLocation(int hili) const
{
    if (!m_renderer || hili < 0) return 0;

    dpdoc::Range range;
    if (!m_renderer->getHighlight(dpdoc::HT_ANNOTATION, hili, &range)) {
        return 0;
    }

    QDocView::Location* result = new QDocView::Location();
    result->page = int(range.beginning->getPagePosition());
    result->pos = range.beginning->getPagePosition() / m_doc->getPageCount();
    result->ref = highlightToBookmark(range.beginning, range.end);
    result->preview = QString::fromUtf8(m_doc->getText(range.beginning, range.end).utf8());
    
    return result;
}

QRect QAdobeDocView::getHighlightBBox(int hili) const
{
    QRect bbox;
    if (!m_renderer || hili < 0) return bbox;

    dpdoc::Range range;
    if( !m_renderer->getHighlight(dpdoc::HT_ANNOTATION, hili, &range) )
    {
        return bbox;
    }

    dpdoc::RangeInfo* rInfo = m_renderer->getRangeInfo(range.beginning, range.end);
    if(rInfo)
    {
        int rectCount = rInfo->getBoxCount();
        if(rectCount)
        {
            bbox.setCoords(100000, 100000, -100000, -100000);// NOTE (epaz): Set up the values to xMax, yMax, xMin, yMin so that qMin and qMax may work fine.
            dpdoc::Rectangle rect;
            //scan all box and merge box in the same line
            for(int i = 0; i < rectCount; ++i)
            {
                rInfo->getBox(i, false, &rect);

                if(i == 0)
                {
                    bbox.setTop(rect.yMin);
                    bbox.setLeft(rect.xMin);
                }

                if(i == rectCount-1)
                {
                    bbox.setBottom(rect.yMax);
                    bbox.setRight(rect.xMax);
                }
            }
        }
        rInfo->release();
    }
    return bbox;
}

void QAdobeDocView::setNoteList(const QList<int>& notes)
{
    qDebug() << Q_FUNC_INFO << ", notes size: " << notes.size();

    m_notes = notes;

    updatePageNotes();

    update();
}

void QAdobeDocView::updatePageNotes()
{
    if (!m_renderer) return;

    QList<ViewerFloatingNote*>::iterator it = m_floatingNotes.begin();
    QList<ViewerFloatingNote*>::iterator itEnd = m_floatingNotes.end();
    for(; it != itEnd; ++it)
    {
        delete *it;
    }
    m_floatingNotes.clear();

    m_notesLocation.clear();
    m_previousNotesCount = 0;

    double startRange, endRange;
    getBookmarkRange(&startRange, &endRange);

    qDebug() << Q_FUNC_INFO << "StartRange: " << startRange << ", endRange: " << endRange;

    int size = m_notes.size();
    for(int i = 0; i < size; ++i)
    {
        QDocView::Location* loc = highlightLocation(m_notes.at(i));
        if(loc)
        {
            dp::ref<dpdoc::Location> startLoc = 0;
            dp::ref<dpdoc::Location> endLoc = 0;

            bookmarkToHighlight(loc->ref, &startLoc, &endLoc);
            if(startLoc && endLoc)
            {
                double startPos = startLoc->getPagePosition() / m_doc->getPageCount();
                double endPos = endLoc->getPagePosition() / m_doc->getPageCount();

                //qDebug() << "--->" << Q_FUNC_INFO << "Highligh " << i << "startPos: " << startPos << ", endPos: " << endPos;

                if( startPos < startRange && endPos < startRange )
                {
                    ++m_previousNotesCount;
                }
                if( startRange < startPos && startPos < endRange )
                {
                    //qDebug() << "We're in!";
                    if( endPos < endRange )
                    {
                        //qDebug() << "The whole highlight inside the view";
                    }
                    else
                    {
                        //qDebug() << "Just a portion of the highlight inside the view";
                        endLoc = m_renderer->getScreenEnd();
                    }
                }
                else if ( startRange < endPos && endPos < endRange )
                {
                    //qDebug() << "We're in!";
                    if( startRange < startPos )
                    {
                        //qDebug() << "The whole highlight inside the view";
                    }
                    else
                    {
                        //qDebug() << "Just a portion of the highlight inside the view";
                        startLoc = m_renderer->getScreenBeginning();
                    }
                }
            }

            if(startLoc && endLoc)
            {
                dpdoc::RangeInfo* rInfo = m_renderer->getRangeInfo(startLoc, endLoc);
                if(rInfo)
                {
                    int rectCount = rInfo->getBoxCount();
                    //qDebug() << "BOX COUNT : " << rectCount;
                    if(rectCount > 0)
                    {
                        dpdoc::Rectangle rect;
                        rInfo->getBox(0, false, &rect);

                        //qDebug() << "MaxX: " << rect.xMax << ", MaxY: " << rect.yMax;

                        double xMax = rect.xMax;
                        double yMin = rect.yMin;
                        double yMax = rect.yMax;

                        //scan all box and merge box in the same line
                        for(int j = 1; j < rectCount; ++j)
                        {
                            rInfo->getBox(j, false, &rect);
                            //qDebug() << "MaxX: " << rect.xMax << ", MaxY: " << rect.yMax;

                            if(rect.yMax > yMax)
                            {
                                xMax = rect.xMax;
//                                yMin = rect.yMin;
                                yMax = rect.yMax;
                            }
                            else if(rect.yMax == yMax)
                            {
                                // It's a lower rectangle. Update the values
                                xMax = qMax(xMax, rect.xMax);
//                                yMin = rect.yMin;
                                yMax = rect.yMax;
                            }
                        }
                        rInfo->release();

                        qDebug() << Q_FUNC_INFO << "Adding a new Note Location at MaxX: " << xMax << ", MaxY: " << yMax;

//                        m_notesLocation.append(QPair<double,double>(xMax, yMin)); DISABLED

                        ViewerFloatingNote* note = new ViewerFloatingNote(this);
                        note->setNumber(i);
                        note->show();
                        note->raise();
                        note->move(width()-note->width(), yMin+1);// NOTE: the magic number +1 is for the note icon not to be hidden in the top part
//                        connect(note, SIGNAL(noteClicked(int)), this, SIGNAL(noteClicked(int)));
                        m_floatingNotes.append(note);
                    }
                }
            }
            delete loc;
        } // end if(loc)
    } // end for
}

void QAdobeDocView::clearNoteList()
{
    m_notes.clear();
}

int QAdobeDocView::noteAt(int x, int y) const
{
    if (!m_renderer) return -1;
    m_transInv.map(x, y, &x, &y);

    dp::ref<dpdoc::Location> loc = 0;
    loc = m_renderer->hitTest(x, y, dpdoc::HF_SELECT | dpdoc::HF_FORCE);

    if (!loc) return -1;

    int hili = -1;
    int n = m_renderer->getHighlightCount(dpdoc::HT_ANNOTATION);

    for (int i = 0; i < n; ++i)
    {
        dpdoc::Range range;
        if (!m_renderer->getHighlight(dpdoc::HT_ANNOTATION, i, &range)) {
            continue;
        }

        if (range.beginning->compare(loc) <= 0 && loc->compare(range.end) <= 0) {
            hili = i;
        }

        if (hili != -1) break;
    }

    return hili;
}

bool QAdobeDocView::setHighlightListRect(dpdoc::RangeInfo *rInfo)
{	
    if (!rInfo)
        return false;

	int rectCount = rInfo->getBoxCount();
	qDebug() << "BOX COUNT : " << rectCount;
	if(rectCount > 0)
	{
		dpdoc::Rectangle rect;
		//QList<QRect> m_rectList;
		rInfo->getBox(0, false, &rect);
		m_rectList.append(
			QRect(static_cast<int>(rect.xMin)
				, static_cast<int>(rect.yMin)
				, static_cast<int>(rect.xMax) - static_cast<int>(rect.xMin)
				, static_cast<int>(rect.yMax) - static_cast<int>(rect.yMin)));
		//scan all box and merge box in the same line
		for(int i = 1; i < rectCount; ++i)
		{
			rInfo->getBox(i, false, &rect);
			qDebug() << "MinY: " << rect.yMin << ", MaxY: " << rect.yMax;
			//if(qAbs(rect.yMin - m_rectList.last().y()) < 3)
			if(rect.yMin == m_rectList.last().y())
			{
				m_rectList.replace(m_rectList.size() - 1
					, QRect(m_rectList.last().x()
						, m_rectList.last().y()
						, static_cast<int>(rect.xMax) - m_rectList.last().x()
						, m_rectList.last().height()));
			}	
			else
			{
				m_rectList.append(
					QRect(static_cast<int>(rect.xMin)
						, static_cast<int>(rect.yMin)
						, static_cast<int>(rect.xMax) - static_cast<int>(rect.xMin)
						, static_cast<int>(rect.yMax) - static_cast<int>(rect.yMin)));
			}
		}
		rInfo->release();
	}
	return rectCount > 0;
}

bool QAdobeDocView::isHorizontal() const
{
    return m_isHorizontal;
}

bool QAdobeDocView::setHorizontal(bool on)
{
    if (m_isHorizontal == on) return false;
    m_isHorizontal = on;

    // TODO: handle margins during these process    

    uint memZoomLevel = m_pdfZoomLevel;

    if (m_renderer)
    {
        delete m_surface;
        m_surface = 0;
        updateViewport();
        updateSurface(m_surface->rect());
    }

    m_pdfZoomLevel = memZoomLevel;

    return true;
}

int QAdobeDocView::pageCount() const
{
    return (int)std::ceil(m_doc->getPageCount());
}

int QAdobeDocView::getPageEnd() const{
    int pos_end = m_renderer->getScreenEnd()->getPagePosition();
    if (m_isPdf && isHardPageMode() && pos_end) --pos_end;
    return pos_end;
}

int QAdobeDocView::getPageBegin() const{
    int pos_end = m_renderer->getScreenBeginning()->getPagePosition() - 1;
    return pos_end;
}

int QAdobeDocView::pageNumberForScreen(int* end) const
{
    int pos_start = 0;
    int pos_end = 0;

    if (m_renderer) {
        if (m_pageMode == MODE_HARD_PAGES) {
            dpdoc::PageNumbers pagenum;
            m_renderer->getPageNumbersForScreen(&pagenum);        
            pos_start= pagenum.beginning;
            pos_end= pagenum.end;
        } else {
            dp::ref<dpdoc::Location> loc = m_renderer->getScreenBeginning();
            if (loc) {
                pos_start = loc->getPagePosition();
            }

            if (end) {
                pos_end = pos_start;
                loc = m_renderer->getScreenEnd();
                if (loc) {
                    pos_end = qMax<int>(loc->getPagePosition() - 1, pos_start);
                }
            }
        }
    }

    if (end) *end = pos_end;
    return pos_start;
}


/** PDF PAGE CHANGE HANDLE *****************************/

bool QAdobeDocView::turnZoomedPdfPage(bool next)
{
    bool result = false;

    double delta_y = 0;
    if (next) delta_y =  0.9;
    else      delta_y = -0.9;

    Screen::getInstance()->queueUpdates();
    // Save current zoom.
    double auxScale = m_scale;
    // Set zoom at minimun.
    m_scale = autoFitFactor(AUTO_FIT_PAGE);
    setScaleFactor(m_scale, 0, delta_y);
    // Go to the screen to take screenshot of the whole page.
    if (next) result = m_renderer->nextScreen();
    else      result = m_renderer->previousScreen();

    // FIXME: ensure exist screen ¿result?
    screenshot();

    // Hold the old zoom.
    m_scale = auxScale;
    if (next) setScaleFactorOnNextZoomedPage(m_scale, 0, delta_y);
    else      setScaleFactorOnPreviousZoomedPage(m_scale, 0, delta_y);

    Screen::getInstance()->flushUpdates();

    return result;
}

bool QAdobeDocView::turnWholePdfPage(bool next)
{
    bool result = false;
    if (next) result = m_renderer->nextScreen();
    else      result = m_renderer->previousScreen();

    screenshot();

    return result;
}

bool QAdobeDocView::goToZoomedPdfPage(LocationRef& loc, bool next)
{
    // FIXME: it's necesary delta aproximation?
    double delta_y = 0;
    if (next) delta_y =  0.9;
    else      delta_y = -0.9;

    Screen::getInstance()->queueUpdates();
    // Save current zoom.
    double auxScale = m_scale;
    // Set zoom at minimun.
    m_scale = autoFitFactor(AUTO_FIT_PAGE);
    setScaleFactor(m_scale, 0, delta_y); // updateViewport();
    // Go to the screen to take screenshot of the whole page.
    m_renderer->navigateToLocation(loc);

    // FIXME: ensure exist screen ¿result?
    screenshot();

    // Hold the old zoom.
    m_scale = auxScale;
    setScaleFactorGoToZoomedPage(m_scale);
    Screen::getInstance()->flushUpdates();

    return true; // Ever true?
}

bool QAdobeDocView::goToWholePdfPage(LocationRef &loc)
{
    m_renderer->navigateToLocation(loc);
    screenshot();

    return true; // Ever true?
}

bool QAdobeDocView::pdfNextScreen()
{
    bool result = false;

    // PDF: Next screen in zoomed view, render full screen to allow miniature view correctly.
    if (m_autoFit == AUTO_FIT_NONE) result = turnZoomedPdfPage(true);
    else                            result = turnWholePdfPage(true);

    positioningMiniature();
    update(); // FIXME: allow refresh in SD.
    return result;
}

bool QAdobeDocView::pdfPreviousScreen()
{
    bool result = false;

    // PDF: Previous screen in zoomed view, render full screen to allow miniature view correctly.
    if (m_autoFit == AUTO_FIT_NONE) result = turnZoomedPdfPage(false);
    else                            result = turnWholePdfPage(false);

    positioningMiniature();
    update(); // FIXME: allow refresh in SD.
    return result;
}

bool QAdobeDocView::pdfGoToPage(LocationRef& loc)
{
    bool result = false;

    // PDF: Next screen in zoomed view, render full screen to allow miniature view correctly.
    if (m_autoFit == AUTO_FIT_NONE) result = goToZoomedPdfPage(loc);
    else                            result = goToWholePdfPage(loc);

    positioningMiniature();
    update(); // FIXME: allow refresh in SD.
    return result;
}

void QAdobeDocView::takeMiniatureScreenshot()
{
    Screen::getInstance()->queueUpdates();

    if (m_autoFit == AUTO_FIT_NONE) // zoomed page
    {
        // Save current zoom.
        double auxScale = m_scale;
        // Set zoom at minimun.
        m_scale = autoFitFactor(AUTO_FIT_PAGE);

        bool auxBlockPaintEvents = m_blockPaintEvents;
        m_blockPaintEvents = false;
        updateViewport();

        screenshot();

        m_blockPaintEvents = auxBlockPaintEvents;
        // Hold the old zoom.
        m_scale = auxScale;
        updateViewport();

    }
    else screenshot();

    update(); // FIXME: allow refresh in SD.

    Screen::getInstance()->flushUpdates();
}

#define FINE_ADJUST_PERCENT_X 1.02

bool QAdobeDocView::screenshot()
{
    QPixmap screenshot = QPixmap::grabWidget((QWidget*)this);

    if (screenshot.isNull()) return false;


    QRect docRect = documentRect().toRect();
    docRect = m_trans.mapRect(docRect);
    screenshot = screenshot.copy(docRect.left(), docRect.top(), docRect.width()*FINE_ADJUST_PERCENT_X, docRect.height());

    if (screenshot.isNull()) return false;

    QBookApp::instance()->getViewer()->updatePdfMiniatureScreenshot(screenshot);

    return true;
}

void QAdobeDocView::scrolling()
{
    positioningMiniature();
    update(); // FIXME: allow refresh in SD.
}

void QAdobeDocView::positioningMiniature()
{
    /// drawScrolls(QPainter& painter, const QRect& wr, const QRect& clip)
    QRect wr = rect();//, clip = event->rect();
    double xoP = 0, yoP = 0, xfP = 1, yfP = 1;
    QRectF dev = documentRect();
    dev = m_trans.mapRect(dev);

    if (!m_isHorizontal || isHardPageMode())
    {
        if (dev.left()  < wr.left()  - Q_SCROLL_FUZZY_GAP) xoP = -dev.left()/(double)dev.width();
        if (dev.right() > wr.right() + Q_SCROLL_FUZZY_GAP) xfP = (wr.right()-dev.left())/(double)dev.width();
    }

    if (m_isHorizontal || isHardPageMode())
    {
        if (dev.top()    < wr.top()    - Q_SCROLL_FUZZY_GAP) yoP = -dev.top()/(double)dev.height();
        if (dev.bottom() > wr.bottom() + Q_SCROLL_FUZZY_GAP) yfP = (wr.bottom()-dev.top())/(double)dev.height();
    }

    QBookApp::instance()->getViewer()->updatePdfMiniatureLocation(xoP, yoP, xfP, yfP);
}

/** END OF PDF PAGE CHANGE HANDLE *************************/

bool QAdobeDocView::nextScreen()
{
    if (!m_renderer) return false;

    qDebug() << Q_FUNC_INFO;

    // TODO: take PDF screenshot in all cases.
    // TODO: try to don't take a screenshot if miniature is hide.

    if (m_pageMode == MODE_SCROLL_PAGES) return setScaleFactor(m_scale, 0, 0.9);

    if (m_isPdf && isHardPageMode() && m_scale > autoFitFactor(AUTO_FIT_PAGE))
    {
        if (setScaleFactor(m_scale, 0, 0.9))
        {
            update(); // FIXME: allow refresh in SD.
            return true;
        }
    }

    if (m_isPdf && m_pageMode != MODE_REFLOW_PAGES) return pdfNextScreen();
    else                                            return m_renderer->nextScreen();
}

bool QAdobeDocView::previousScreen()
{
    if (!m_renderer) return false;

    qDebug() << Q_FUNC_INFO;

    if (m_pageMode == MODE_SCROLL_PAGES) return setScaleFactor(m_scale, 0, -0.9);

    if (m_isPdf && isHardPageMode() && m_scale > autoFitFactor(AUTO_FIT_PAGE))
    {
        if (setScaleFactor(m_scale, 0, -0.9))
        {
            update(); // FIXME: allow refresh in SD.
            return true;
        }
    }

    if (m_isPdf && m_pageMode != MODE_REFLOW_PAGES) return pdfPreviousScreen();
    else                                            return m_renderer->previousScreen();
}

bool QAdobeDocView::gotoPage(int pos)
{
    if (!m_renderer) return false;
    dp::ref<dpdoc::Location> loc = m_doc->getLocationFromPagePosition(pos);
    if (!loc) return false;

    qDebug() << Q_FUNC_INFO << loc->getPagePosition() << loc->getBookmark().utf8();

    if (m_isPdf && m_pageMode != MODE_REFLOW_PAGES) pdfGoToPage(loc);
    else                                            m_renderer->navigateToLocation(loc);

    return true; // Ever true?
}

/*-------------------------------------------------------------------------*/

QDocView::Location* QAdobeDocView::bookmark()
{
    if (!m_renderer) return 0;

    qDebug() << Q_FUNC_INFO << "Locking mutex";
    QMutexLocker locker(&s_adobeConcurrentMutex);
    qDebug() << Q_FUNC_INFO << "After locking mutex";

    dp::ref<dpdoc::Location> start = m_renderer->getScreenBeginning();
    if (!start) return 0;

    dp::ref<dpdoc::Location> end = m_renderer->getScreenBeginning();

    if (!end)
        return 0;

    Location* loc = new Location;
    loc->page = pageNumberForScreen();
    loc->pos = start->getPagePosition() / m_doc->getPageCount();
    loc->ref = QString::fromUtf8(start->getBookmark().utf8());

    dp::String text = m_doc->getText(start, end);
    loc->preview = QString::fromUtf8(text.utf8()).simplified();

    if (loc->preview.length() > 200) {
        loc->preview.truncate(200);
        loc->preview += tr(" ...");
    }

    return loc;
}

double QAdobeDocView::getInitialPosFromRenderer()
{
    if (!m_renderer) return 0;

    dp::ref<dpdoc::Location> start = m_renderer->getScreenBeginning();
    if (!start) return 0;

    double pos = start->getPagePosition() / m_doc->getPageCount();

    return pos;
}

double QAdobeDocView::getFinalPosFromRenderer()
{
    if (!m_renderer) return 0;

    dp::ref<dpdoc::Location> start = m_renderer->getScreenEnd();
    if (!start) return 0;

    double pos = start->getPagePosition() / m_doc->getPageCount();

    return pos;
}

void QAdobeDocView::gotoBookmark(const QString& ref)
{
    if (!m_renderer) return;

    dp::ref<dpdoc::Location> loc;
    qDebug() << Q_FUNC_INFO << "Locking mutex";
    QMutexLocker locker(&s_adobeConcurrentMutex);
    qDebug() << Q_FUNC_INFO << "After locking mutex";

    QTime time;
    time.start();
    if (!bookmarkToHighlight(ref, &loc, 0))
    {
        loc = m_doc->getLocationFromBookmark(ref.toUtf8().constData());

        qDebug() << Q_FUNC_INFO << "m_doc getLocationFromBookmark finished. time=" << time.elapsed();
        time.restart();
        if (!loc) return;
    }
    qDebug() << Q_FUNC_INFO << "before navigateToLocation.";

    m_renderer->navigateToLocation(loc);

    if (m_isPdf) positioningMiniature();

    qDebug() << Q_FUNC_INFO << "navigateToLocation finished. time=" << time.elapsed();
}

void QAdobeDocView::getBookmarkRange(double* start, double* end)
{
    *start = 0;
    *end = 0;
    if (!m_renderer) return;

    dp::ref<dpdoc::Location> loc = m_renderer->getScreenBeginning();
    if (!loc) return;
    double pos_start = loc->getPagePosition();
    double n = m_doc->getPageCount();
    *start = pos_start / n;

    loc = m_renderer->getScreenEnd();
    if (!loc) return;
    double pos_end = loc->getPagePosition();
    if (pos_start >= pos_end) pos_end = pos_start + 1;
    *end = pos_end / n;
}

double QAdobeDocView::getPosFromBookmark(const QString& bookmark)
{
    qDebug() << Q_FUNC_INFO << "bookMark: " << bookmark;

    if (!m_renderer)
        return 0;

    dp::ref<dpdoc::Location> loc = 0;
    loc = m_doc->getLocationFromBookmark(dp::String(bookmark.toUtf8().constData()));
    double startPos = 0.0;
    if(loc)
        startPos= loc->getPagePosition() / m_doc->getPageCount();
    qDebug() << Q_FUNC_INFO <<"startPos:" << startPos;
    return startPos;
}

double QAdobeDocView::getPosFromHighlight(const QString& high){

    qDebug() << Q_FUNC_INFO << "high: " << high;
    QStringList markList = high.split("|");
    QString mark = markList[1];

    if (!m_renderer)
        return 0;

    dp::ref<dpdoc::Location> loc = 0;
    loc = m_doc->getLocationFromBookmark(dp::String(mark.toUtf8().constData()));
    double startPos = 0.0;
    if(loc)
        startPos= loc->getPagePosition() / m_doc->getPageCount();

    return startPos;
}

int QAdobeDocView::getPageFromMark(const QString& high){

    qDebug() << Q_FUNC_INFO << "high: " << high << " Entra";

    QStringList markList = high.split("|");
    QString mark;
    if(markList.size() > 1)
        mark = markList[1];
    else
        mark = high;
    if (!m_renderer)
        return 0;
    dp::ref<dpdoc::Location> loc = 0;
    loc = m_doc->getLocationFromBookmark(dp::String(mark.toUtf8().constData()));
    int pageMark = 0;

    if(loc)
        pageMark= loc->getPagePosition();

    qDebug() << Q_FUNC_INFO << "high: " << high << " Sale";
    return pageMark;
}

QString QAdobeDocView::getBookmarkFromPosition(double pos)
{
    qDebug() << Q_FUNC_INFO << "pos: " << pos;
    double pages = m_doc->getPageCount();
    double finalPos = pos * pages;
    dp::ref<dpdoc::Location> loc = 0;
    if(pos > 0)
        loc =  m_doc->getLocationFromPagePosition(finalPos);
    else
        return "";

    if (!loc) {
        return "";
    }

    return QString::fromUtf8(loc->getBookmark().utf8());
}

bool QAdobeDocView::thumbnail(QPaintDevice* map)
{
    if (!m_renderer || !m_surface) return false;

    QPainter painter(map);
    QAdobeSurface surface(map->width(), map->height(), m_backColor);

    QRect r = m_surface->rect();
    QRect t = surface.rect();
    double scale = qMin(double(map->width()) / r.width(), double(map->height()) / r.height());
    r.setWidth(int(r.width() * scale));
    r.setHeight(int(r.height() * scale));
    r.moveCenter(t.center());
    painter.fillRect(t, m_backColor);
    painter.drawImage(r, *m_surface);

    return true;
}

/*-------------------------------------------------------------------------*/

QString QAdobeDocView::wordAt(int x, int y)
{
    if (!m_renderer) return QString();
    m_transInv.map(x, y, &x, &y);

	dpdoc::PageNumbers pagenum;
	m_renderer->getPageNumbersForScreen(&pagenum);
	dp::ref<dpdoc::Location> start = m_isPdf ? m_doc->getLocationFromPagePosition(pagenum.beginning) : m_renderer->getScreenBeginning();
	dp::ref<dpdoc::Location> end = 0;
	if(m_isPdf)
	{
		if(pagenum.beginning == m_doc->getPageCount() - 1)
			end = m_doc->getEnd();
		else {
			end = m_doc->getLocationFromPagePosition(pagenum.beginning + 1);
		}
	}
	else
		end = m_renderer->getScreenEnd();
	
    dp::ref<dpdoc::Location> loc = 0;
    loc = m_renderer->hitTest(x, y, dpdoc::HF_SELECT | dpdoc::HF_FORCE);
    if (!loc || !start || !end)
        return QString();

    dp::String before = m_doc->getText(start, loc);
    //dp::String after = m_doc->getText(loc, end);
    //qDebug() << loc->compare(end, 0);
    QString text = QString::fromUtf8(before.utf8());
    int pos = text.length() - 1;
	if(pos < 0) return QString();
    // text += QString::fromUtf8(after.utf8());    
    dp::String allPageText = m_doc->getText(start, end);
	//prevent adobe bug: getScreenEnd() won't get the end char
	if(QString::fromUtf8(allPageText.utf8()).length() > pos)
		text = QString::fromUtf8(allPageText.utf8());
    

    if (pos > 0 && pos < text.length() && isHanzi(text.at(pos))) {
        // special tweak for hanzi hit test
        // hanzi is selected by single char, thus the traditional hit testing may select wrong (next) char
        // traditional hit testing try to determine the cursor insert pos, not char pos
        // it determine the pos by tell whether the hit point is in left or right half side of char
        // we offset x to left 10 pixels (assume hanzi is not wider than this), if it report different pos
        // that usually means user hit the right half part of char, we try to fix that
        loc = 0;
        loc = m_renderer->hitTest(x + scaleFactor(), y, dpdoc::HF_SELECT | dpdoc::HF_FORCE);
        if (loc) {
            dp::String prev = m_doc->getText(start, loc);
            QString prev_text = QString::fromUtf8(prev.utf8());
			pos = prev_text.length() - 1;
            //if (pos != prev_text.length()) pos--;
        }
    }
    qDebug() << "Text is " << text.at(pos);
    return extractWord(text, pos, m_isHili);
}

QStringList QAdobeDocView::wordAt(int x,int y,int context_length)
{
        if(context_length<=0){
                context_length = 0;
        }
    if (!m_renderer) return QStringList();
    m_transInv.map(x, y, &x, &y);

        dpdoc::PageNumbers pagenum;
        m_renderer->getPageNumbersForScreen(&pagenum);
        dp::ref<dpdoc::Location> start = m_isPdf ? m_doc->getLocationFromPagePosition(pagenum.beginning) : m_renderer->getScreenBeginning();
        dp::ref<dpdoc::Location> end = 0;
        if(m_isPdf)
        {
                if(pagenum.beginning == m_doc->getPageCount() - 1)
                        end = m_doc->getEnd();
                else {
                        end = m_doc->getLocationFromPagePosition(pagenum.beginning + 1);
                }
        }
        else
                end = m_renderer->getScreenEnd();

    dp::ref<dpdoc::Location> loc = 0;
        loc = m_renderer->hitTest(x, y, dpdoc::HF_SELECT | dpdoc::HF_FORCE);
    if (!loc || !start || !end) {
        return QStringList();
    }

    dp::String before = m_doc->getText(start, loc);
    QString text = QString::fromUtf8(before.utf8());
    int pos = text.length() - 1;
        if(pos < 0) return QStringList();
        else qDebug() << Q_FUNC_INFO << "text" << text.at(pos);
    dp::String allPageText = m_doc->getText(start, end);

        if(QString::fromUtf8(allPageText.utf8()).length() > pos)
                text = QString::fromUtf8(allPageText.utf8());

    if (pos > 0 && pos < text.length() && isHanzi(text.at(pos))) {
        // special tweak for hanzi hit test
        // hanzi is selected by single char, thus the traditional hit testing may select wrong (next) char
        // traditional hit testing try to determine the cursor insert pos, not char pos
        // it determine the pos by tell whether the hit point is in left or right half side of char
        // we offset x to left 10 pixels (assume hanzi is not wider than this), if it report different pos
        // that usually means user hit the right half part of char, we try to fix that

        loc = m_renderer->hitTest(x - 10, y, dpdoc::HF_SELECT | dpdoc::HF_FORCE);
        if (loc) {
            dp::String prev = m_doc->getText(start, loc);
            QString prev_text = QString::fromUtf8(prev.utf8());
            if (pos != prev_text.length()) pos--;
        }
    }

        QString word="";
        QString first_str="";
        QString second_str="";

        // Fix for the beggining of the page. Sometimes seg faulted because of trying to perform
        // an .at(-1)
        QChar ch_tmp;
        if(pos > 0)
            ch_tmp = text.at(pos-1);
        else
            ch_tmp = '.';

        int wordsToFind = context_length+1;
        int currentWords = 0;
        int lastChar = 0; /*0: no char; 1: char*/
    if (ch_tmp.isSpace() || (ch_tmp.isPunct() && ch_tmp != '-') || isHanzi(ch_tmp)){
                wordsToFind = context_length;
    }

    for (int i = pos - 1; i >= 0; i--) {
        QChar ch = text.at(i);
        if (ch.isSpace() || (ch.isPunct() && ch != '-') || isHanzi(ch)){
                        if(lastChar==1 && currentWords==wordsToFind){
                                break;
                        }
                        lastChar = 0;
        }
                else{
                        if(lastChar==0){
                                currentWords++;
                        }
                        lastChar = 1;
                }
                if(wordsToFind == context_length+1 && currentWords==1 && lastChar==1){
                        word.prepend(ch);
                }
                else{
                first_str.prepend(ch);
                }
    }

        wordsToFind = context_length+1;
        currentWords = 0;
        lastChar = 0; /*0: no char; 1: char*/

    for (int i = pos; i < text.length(); i++) {
        QChar ch = text.at(i);
        if (ch.isSpace() || (ch.isPunct() && ch != '-') || isHanzi(ch)){
                        if(lastChar==1 && currentWords==wordsToFind){
                                break;
                        }
                        lastChar = 0;
        }
                else{
                        if(lastChar==0){
                                currentWords++;
                        }
                        lastChar = 1;
                }
        if(currentWords==1 && lastChar==1){
                        word.append(ch);
                }
                else{
                second_str.append(ch);
                }
    }
        QStringList output;

    return output<<word<<first_str<<second_str;

}


/*-------------------------------------------------------------------------*/

struct QAdobeTOC : public QDocView::TableOfContent
{
    QAdobeTOC(dpdoc::Document* doc, dpdoc::TOCItem* toc, int);
    virtual ~QAdobeTOC();

    virtual QString title();
    virtual QDocView::Location* location();
    virtual int itemCount();
    virtual QDocView::TableOfContent* itemAt(int i);
    virtual int getDepth();
    virtual void setDepth(int);

private:
    dpdoc::Document* m_doc;
    dpdoc::TOCItem* m_toc;
    int depth;
};

QAdobeTOC::QAdobeTOC(dpdoc::Document* doc, dpdoc::TOCItem* toc, int _depth = 0)
    : m_doc(doc)
    , m_toc(toc)
    , depth(_depth)
{
}

QAdobeTOC::~QAdobeTOC()
{
    m_toc->release();
}

int QAdobeTOC::getDepth()
{
    return depth;
}

void QAdobeTOC::setDepth(int _depth)
{
    depth = _depth;
}

QString QAdobeTOC::title()
{
    qDebug() << m_toc->getTitle().utf8();
    return QString::fromUtf8(m_toc->getTitle().utf8());
}

QDocView::Location* QAdobeTOC::location()
{
    dp::ref<dpdoc::Location> loc = m_toc->getLocation();
    if (!loc) return 0;

	double pos = loc->getPagePosition();
    QDocView::Location* result = new QDocView::Location();
	result->page = int(pos);
    result->pos = pos / m_doc->getPageCount();
    result->ref = QString::fromUtf8(loc->getBookmark().utf8());
    result->preview = title();
    result->depth = getDepth();

    return result;
}

int QAdobeTOC::itemCount()
{
    return m_toc->getChildCount();
}

QDocView::TableOfContent* QAdobeTOC::itemAt(int i)
{
    dpdoc::TOCItem* item = m_toc->getChild(i);
    if (!item) return 0;
    return new QAdobeTOC(m_doc, item, depth + 1);
}

QDocView::TableOfContent* QAdobeDocView::tableOfContent()
{
    dpdoc::TOCItem* item = m_doc->getTocRoot();    
    if (!item) return 0;
    return new QAdobeTOC(m_doc, item, -1);
}

/*-------------------------------------------------------------------------*/

struct QAdobeSearchResult : public QDocView::SearchResult
{
    QAdobeSearchResult(dpdoc::Document* doc, const QString& text, const QString& tag_fmt, bool word_only);
    virtual ~QAdobeSearchResult();

    virtual bool hasNext();
    virtual QDocView::Location* next();    
    QDocView::Location* current();

private:
    dpdoc::Document* m_doc;
    dp::ref<dpdoc::Location> m_pos;
    dp::ref<dpdoc::Location> m_end;
    QDocView::Location* m_loc;
    QString m_text;
    QString m_tag_fmt;
    int m_flags;
    bool m_eof;
};

QAdobeSearchResult::QAdobeSearchResult(dpdoc::Document* doc, const QString& text, const QString& tag_fmt, bool word_only)
    : m_doc(doc)
    , m_pos(doc->getBeginning())
    , m_end(doc->getEnd())
    , m_loc(NULL)
    , m_text(text)
    , m_tag_fmt(tag_fmt)
    , m_flags(word_only ? dpdoc::SF_WHOLE_WORD | dpdoc::SF_IGNORE_ACCENTS : dpdoc::SF_IGNORE_ACCENTS)
    , m_eof(false)
{
}

QAdobeSearchResult::~QAdobeSearchResult()
{
    delete m_loc;
    m_loc = NULL;
}

QDocView::Location* QAdobeSearchResult::current()
{
    qDebug() << Q_FUNC_INFO;

    qDebug() << Q_FUNC_INFO << "Locking mutex";
    QMutexLocker locker(&s_adobeConcurrentMutex);
    qDebug() << Q_FUNC_INFO << "After locking mutex";

    if (m_loc) return m_loc;
    if (m_eof) return 0;

    dpdoc::Range range;

    qDebug() << Q_FUNC_INFO << "find text";
    m_eof = !m_doc->findText(m_pos, m_end, m_flags, m_text.toUtf8().constData(), &range);
    qDebug() << Q_FUNC_INFO << "after find text";

    if (!m_eof)
    {
        dp::ref<dpdoc::Location> start = range.beginning;
        dp::ref<dpdoc::Location> end = range.end;

        double pos_start = start->getPagePosition();
        double pos_end = end->getPagePosition();
        double pos_max = m_doc->getPageCount();

        m_loc = new QDocView::Location;
		m_loc->page = int(pos_start);
        m_loc->pos = pos_start / m_doc->getPageCount();
        m_loc->ref = QString("#HL|%1|%2")
            .arg(QString::fromUtf8(start->getBookmark().utf8()))
            .arg(QString::fromUtf8(end->getBookmark().utf8()));


        dp::ref<dpdoc::Location> pv_start = m_doc->getLocationFromPagePosition(qMax(pos_start - 0.25, 0.0));
		dp::ref<dpdoc::Location> pv_end = m_doc->getLocationFromPagePosition(qMin(pos_end + 1.25, pos_max));

        if (pv_start)
        {
            dp::String text = m_doc->getText(pv_start, start);
            if (text.length() > 0)
            {
//                m_loc->preview += QString("...");// NOTE: (epaz) I commented it because it's added again later
                m_loc->preview += QString::fromUtf8(text.utf8()).right(50);
            }
        }

        dp::String textWithAccents = m_doc->getText(start, end);

        m_loc->preview = QDocView::qlabelEntityEncode(m_loc->preview);
        if (m_tag_fmt.isEmpty())
        {
            m_loc->preview += QDocView::qlabelEntityEncode(QString::fromUtf8(textWithAccents.utf8()));
        }
        else
        {
            m_loc->preview += m_tag_fmt.arg(QDocView::qlabelEntityEncode(QString::fromUtf8(textWithAccents.utf8())));
        }

        if (pv_end)
        {
            dp::String text = m_doc->getText(end, pv_end);
            if (text.length() > 0)
            {
                m_loc->preview += QDocView::qlabelEntityEncode(QString::fromUtf8(text.utf8()).left(50));
//                m_loc->preview += QString("...");// NOTE: (epaz) I commented it because it's added again later
            }
        }

        m_loc->preview = m_loc->preview.simplified();
        m_pos = end;
        end = 0;
    }

    qDebug() << Q_FUNC_INFO << "End";
    return m_loc;
}

bool QAdobeSearchResult::hasNext()
{
    return current() != 0;
}

QDocView::Location* QAdobeSearchResult::next()
{
    QDocView::Location* loc = current();
    m_loc = NULL;
    return loc;
}

/*-------------------------------------------------------------------------*/
QDocView::SearchResult* QAdobeDocView::search(const QString& text, const QString& tag_fmt, bool word_only)
{
    if (!m_doc) return 0;
    return new QAdobeSearchResult(m_doc, text, tag_fmt, word_only);
}

QString QAdobeDocView::getText(int start, int end) const
{
    if (!m_doc || !m_renderer) return QString();

    dp::ref<dpdoc::Location> loc_start = 0;
    dp::ref<dpdoc::Location> loc_end = 0;

    if (start == -1 && end == -1) {
        loc_start = m_renderer->getScreenBeginning();
    } else if (start == 0) {
        loc_start = m_doc->getBeginning();
    } else {
        loc_start = m_doc->getLocationFromPagePosition(start);
    }

    if (start == -1 && end == -1) {
        loc_end = m_renderer->getScreenEnd();
    } else if (end == -1 || end >= m_doc->getPageCount()) {
        loc_end = m_doc->getEnd();
    } else {
        loc_end = m_doc->getLocationFromPagePosition(end + 1);
    }

    QString text;
    if (loc_start && loc_end) {
        text = QString::fromUtf8(m_doc->getText(loc_start, loc_end).utf8());
    }

    return text;
}

/*-------------------------------------------------------------------------*/

bool QAdobeDocView::goBack()
{
    if (!m_renderer) return false;
    QAdobeEvent event(dpdoc::EK_NAVIGATE, dpdoc::NAVIGATE_PREVIOUS);
    m_renderer->handleEvent(&event);
    return !event.isRejected();
}

bool QAdobeDocView::goForward()
{
    if (!m_renderer) return false;
    QAdobeEvent event(dpdoc::EK_NAVIGATE, dpdoc::NAVIGATE_NEXT);
    m_renderer->handleEvent(&event);
    return !event.isRejected();
}

/*-------------------------------------------------------------------------*/

void QAdobeDocView::updateState(int state)
{
    switch (state) {
    case LOAD_STARTED:
        qDebug() << Q_FUNC_INFO << "LOAD_STARTED";
        updateTitle();
        updateRenderer();
        break;
    case LOAD_FINISHED:
        qDebug() << Q_FUNC_INFO << "LOAD_FINISHED";

        checkErrors();

        break;
    case LOAD_FAILED:
        qDebug() << Q_FUNC_INFO << "LOAD_FAILED";
        if (m_tryPassHash) return; // we will try reload pass hash doc

        checkErrors();

        break;
    }
    emit stateChanged(state);
}

void QAdobeDocView::checkErrors()
{
    qDebug() << Q_FUNC_INFO;

    int errorMask = EDVLE_NONE;
    QString errorMsg;
    QStringList warnings;

    dp::ref<dp::ErrorList> errs = m_doc->getErrorList();

    if (errs->hasFatalErrors())
    {
        errorMask |= EDVLE_FATAL_ERROR;
        m_host->stopProcessing();
        emit fatalOccurred();
        return;
    }

    size_t length = errs->length();

    qDebug() << Q_FUNC_INFO << "Number of errors:" << length << ". Warnings:" << errs->hasWarnings() << ", Errors:" << errs->hasErrors() << ", FatalErrors:" << errs->hasFatalErrors();

    for (size_t i = 0; i < length; ++i)
    {
        QString msg = QString::fromUtf8(errs->item(i).utf8());
        qDebug() << Q_FUNC_INFO << msg;

        if (msg.startsWith("E_PDF_STD_SECURITY_PASSWORD"))
        {
            errorMsg = tr("Wrong password.");
            errorMask |= EDVLE_FATAL_ERROR;
        }
        else if (msg.startsWith("E_ADEPT_") || msg.startsWith("W_ADEPT_") )
        // Possible errors E_ADEPT_NO_VALID_LICENSE, E_ADEPT_CORE_USER_NOT_ACTIVATED, W_ADEPT_CORE_EXPIRED, W_ADEPT_CORE_LOAN_NOT_ON_RECORD, W_ADEPT_CORE_NO_LOAN_TOKENS_FOUND
        {
            if(!(errorMask & EDVLE_LICENSE_ERROR))
            {
                errorMsg = tr("Access Denied. Check your adobe DRM license.");
                errorMask |= EDVLE_LICENSE_ERROR;
            }
        }
        else if(msg.startsWith("E_"))
        {
            errorMsg = tr("Error detected when trying to open the book.");
            errorMask |= EDVLE_GENERIC_ERROR;
        }
        else if( msg.startsWith("W_PDF_BKMK_INVALID") || msg.startsWith("W_PKG_ERROR_IN_SUBDOC") || msg.startsWith("W_PKG_BKMK_INVALID"))
        {
            //    else if(err.startsWith("W_PDF_BKMK_INVALID"))
            //    {
            //        QStringList warnings;
            //        warnings.append(err.split(" ").at(0));
            //        emit warningOccurred(warnings);
            //    }
            warnings.append(msg.split(" ").at(0));
        }
    }

    // Clear errors
    errs->clear();

    if(warnings.size())
    {
        qDebug() << Q_FUNC_INFO << "Emitting Warnings: " << warnings;

        emit warningOccurred(warnings);
    }

    if (errorMask != EDVLE_NONE)
    {
        qDebug() << Q_FUNC_INFO << "Emitting Error: " << errorMsg << ", mask:" << errorMask;
        emit errorOccurred(errorMsg, errorMask);
    }
}

void QAdobeDocView::updateSurface(const QRect& box)
{
    if (m_dirty.isEmpty()) m_dirty = box;
    else m_dirty = m_dirty.united(box);
    update(m_trans.mapRect(box).intersected(rect()));
}

void QAdobeDocView::updateUrl(const QUrl& url)
{
    if (m_url.isParentOf(url)) {
		// intra-document navigation
        QByteArray new_part = url.toEncoded();
        QByteArray old_part = m_url.toEncoded();
		dp::ref<dpdoc::Location>  loc = m_doc->getLocationFromBookmark(new_part.data() + old_part.length());
		if (loc) {
			m_renderer->navigateToLocation(loc);
		}
	}
}

void QAdobeDocView::updateNavigation()
{
    qDebug() << Q_FUNC_INFO;

    if (m_isBusy || !m_renderer) return;


    // update metrics if document size is changed
    if (updateNaturalSize()) {
		bool view_changed;
        updateOffsetXY();

        view_changed = updateMetrics();
		if(false == view_changed)
			updatePageOffset();
        return;
    }

    // some navigation may adjust scale automatically, we want to revert it
    if (!m_clickPos.isNull() && !m_isFontScalable) {
        dpdoc::Matrix mat;
        m_renderer->getNavigationMatrix(&mat);
        if (mat.a != m_scale) {
            updateOffsetXY();
            updateMetrics();
            return;
        }
    }

    updateOffsetXY();
    updatePageOffset();
}

void QAdobeDocView::updateError(const QString& err)
{
    qDebug() << Q_FUNC_INFO << err << m_doc->getErrorList()->length();

    if (err.startsWith("E_ADEPT_CORE_PASSHASH_NOT_FOUND"))
    {
        QString op = err.mid(32);
        if (m_tryPassHash)
        {
            // pass is wrong, remove last pass and report error
            m_tryPassHash = false;
            AdobeDRM::getInstance()->removeHashPass(op, m_hashUser, m_hashPass);
        }
        else
        {
            handleHashPassword(op);
            return;
        }
    }
    else
        checkErrors();
}

void QAdobeDocView::handleLicense(const QString& type, const QString& res, const uchar*, size_t)
{
    qDebug() << Q_FUNC_INFO << "type: " << type << ", res: " << res;

    dp::String dp_type(type.toUtf8());
    dp::String dp_res(res.toUtf8());

    QString path = qgetenv("ADEPT_ACTIVATION_FILE");
    if (!path.isEmpty()) {
        QFile file(path);
        if (file.open(QFile::ReadOnly)) {
            QByteArray buf = file.readAll();
            file.close();
            m_doc->setLicense(dp_type, dp_res, dp::TransientData(reinterpret_cast<const uchar*>(buf.data()), buf.length()));
            return;
        }
    }

    m_doc->setLicense(dp_type, dp_res, dp::Data());
}

void QAdobeDocView::handlePassword()
{
    qDebug() << Q_FUNC_INFO;

    QString pass = handleInput(tr("Password:"));

    if(!m_isFileExist)
	{
		emit closePWDlg();
		return;
    }

    if (!pass.isEmpty())
        m_doc->setDocumentPassword(dp::String(pass.toUtf8()));
    else
    {
        m_host->stopProcessing();
        emit errorOccurred(tr("Password protected documents are not supported."), EDVLE_FATAL_ERROR | EDVLE_PASSWORD_ERROR);
    }
}

void QAdobeDocView::handleHashPassword(const QString& op)
{
    qDebug() << Q_FUNC_INFO;

    QString err = tr("Cancelled by user.");
    QString user = handleInput(tr("Username:"));

    if (!user.isEmpty()) {
        QString pass = handleInput(tr("Password:"));

        if (!pass.isEmpty()) {
            if (AdobeDRM::getInstance()->addHashPass(op, user, pass)) {
                err.clear();
                m_tryPassHash = true;
                m_hashUser = user;
                m_hashPass = pass;
                setUrl(m_url);
                return;
            } else {
                err = tr("Wrong username or password.");
            }
        }
    }

    m_host->stopProcessing();
    m_tryPassHash = false;
    emit errorOccurred(err, EDVLE_FATAL_ERROR | EDVLE_PASSWORD_ERROR);
}

// TODO: remove
QString QAdobeDocView::handleInput(const QString& prompt)
{
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    Q_UNUSED(prompt);
    return "";
}

/*-------------------------------------------------------------------------*/

void QAdobeDocView::updateTitle()
{
    dp::String title;
	dp::ref<dpdoc::MetadataItem> meta = m_doc->getMetadata("DC.title", 0);
	if(meta)
		title = meta->getValue();
    QString old_title = m_title;

    if (title.length() > 0) {
        m_title = QString::fromUtf8(title.utf8());
    } else {
        m_title = m_url.path();
        int idx = m_title.lastIndexOf(QChar('/'));
        if (idx >= 0) m_title.remove(0, idx + 1);
    }

    if (m_title != old_title) emit titleChanged(m_title);
}

void QAdobeDocView::setMargin(double top, double right, double bottom, double left)
{
    m_topMargin = top;
    m_bottomMargin = bottom;
    m_leftMargin = left;
    m_rightMargin = right;
}

QPoint QAdobeDocView::getTopLeftMargin(void) const
{
    return QPoint(m_leftMargin, m_topMargin);
}

void QAdobeDocView::setBookInfo(const BookInfo& bookInfo)
{
    QDocView::setBookInfo(bookInfo);
    if (bookInfo.orientation == BookInfo::ORI_LANDSCAPE) setHorizontal(true);
    else                                                 setHorizontal(false);
    bool editorFonts = QBook::settings().value("setting/reader/editorFonts", true).toBool();
    m_host->setEditorFonts(editorFonts);
}

void QAdobeDocView::updateRenderer()
{
    if (m_renderer) m_renderer->release();
    m_renderer = m_doc->createRenderer(m_host);
    m_renderer->setNavigationMatrix(dpdoc::Matrix());
    m_renderer->showPageNumbers(false);
    qDebug() << Q_FUNC_INFO << "parentWidget = " << parentWidget ();
    m_renderer->setViewport(parentWidget ()->width(), parentWidget ()->height()/* - mapToGlobal(QPoint(0,0)).y()*/, true);

    double scale = QBook::settings().value("margin/scale",1).toInt();
    qDebug() << Q_FUNC_INFO << "scale factor " << scale;
    m_renderer->setMargins(m_topMargin * scale, m_rightMargin * scale, m_bottomMargin * scale, m_leftMargin * scale);

    double old_size = m_renderer->getDefaultFontSize();
    double new_size = Q_FONT_SIZE_BEST;
    if (qFuzzyCompare(old_size, new_size)) new_size++;
    m_renderer->setDefaultFontSize(new_size);
    new_size = m_renderer->getDefaultFontSize();
    m_isFontScalable = !qFuzzyCompare(old_size, new_size);
    if (m_isFontScalable) m_scale = new_size;

    updateNaturalSize();
}

bool QAdobeDocView::updateViewport()
{
    if (!m_surface) {
        int w = width();
        int h = height();
        if (m_isHorizontal) qSwap(w, h);
        qDebug() << Q_FUNC_INFO << w << h << m_backColor;
        m_surface = new QAdobeSurface(w, h, m_backColor);
        m_dirty = m_surface->rect();
        update();
    }
    if (m_renderer) {
        double f = 1.0;
        if (m_pageMode == MODE_REFLOW_PAGES) f /= autoFitFactor(m_autoFit);
        QSize view_size(m_surface->width() * f, m_surface->height() * f);
        qDebug() << Q_FUNC_INFO << view_size;
        if (view_size != m_viewSize) {
            m_renderer->setViewport(view_size.width(), view_size.height(), true);
            updateNaturalSize();
            m_viewSize = view_size;
        }
    }

    m_trans.reset();
    if (m_isHorizontal) {
        m_trans.translate(width(), 0);
        m_trans.rotate(90);
    }
    m_transInv = m_trans.inverted();
    return updateMetrics();
}

static inline bool qFuzzyCompareFixed(double a, double b)
{
    return QDocView::fuzzyCompare(a, b) == 0;
}

static inline bool qFuzzyCompareInt(double a, double b)
{
    return qAbs((int)(a) - (int)(b)) <= 1;
}

bool QAdobeDocView::updateMetrics()
{
    if (!m_renderer || !m_surface || m_isBusy) return false;
    m_isBusy = true;

    int sw = m_surface->width();
    int sh = m_surface->height();

    double newScale = autoFitFactor(m_autoFit);
    if (m_scale != newScale)
    {
        emit zoomChange(newScale);
        updateScale(newScale);
    }

    double f = m_isFontScalable ? 1.0 : m_scale;    
    double w = f * m_docSize.width();
    double h = f * m_docSize.height();
    double x = m_offsetX * w;
    double y = m_offsetY * h;

    if (w < sw) {
        x = (sw - w) / 2;
    } else {
        x = (int)-qBound(0.0, x, w - sw);
    }

    if (h < sh) {
        y = (sh - h) / 2;
    } else {
        y = (int)-qBound(0.0, y, h - sh);
    }

    dpdoc::Matrix mat;
    m_renderer->getNavigationMatrix(&mat);

    bool font_changed = false;
    bool scale_changed = mat.a != f;
    bool zoom_changed = !qFuzzyCompareFixed(mat.a, f);
    bool x_changed = !qFuzzyCompareInt(mat.e, x);
    bool y_changed = !qFuzzyCompareInt(mat.f, y);
    bool view_changed = zoom_changed || x_changed || y_changed;

    if (view_changed) {
        if (zoom_changed) {
            mat.a = f;
            mat.d = f;
        }
        if (x_changed) mat.e = x;
        if (y_changed) mat.f = y;
        qDebug() << Q_FUNC_INFO << mat.a << mat.b << mat.c << mat.d << mat.e << mat.f;
        m_renderer->setNavigationMatrix(mat);
        m_renderer->getNavigationMatrix(&mat);
    }

    if (m_isFontScalable) {
        double old_size = m_renderer->getDefaultFontSize();
        double new_size = m_scale;
        if (!qFuzzyCompareFixed(old_size, new_size)) {
            m_renderer->setDefaultFontSize(new_size);
            font_changed = true;
        }
    } else {
	    if(qAbs(m_scale - mat.a) > 0.01)
            updateScale(mat.a);
    }

    updateOffsetXY();

    if ( !m_isDoingSetURL && 
	(scale_changed || font_changed)) {
        updatePageOffset();
    }

    m_isBusy = false;

    if ( !m_isDoingSetURL && 
		(scale_changed || font_changed)) {

        emit scaleFactorChanged(m_scale);
    }

    return view_changed;
}

bool QAdobeDocView::updateNaturalSize()
{
    if (!m_renderer) return false;

    double w, h;
    dpdoc::Rectangle rec;
    m_renderer->getNaturalSize(&rec);
    w = rec.xMax- rec.xMin;
    h = rec.yMax- rec.yMin;

    bool changed = w != m_docSize.width() || h != m_docSize.height();
    m_docSize.setWidth(w);
    m_docSize.setHeight(h);

    qDebug() << Q_FUNC_INFO << m_docSize;
    return changed;
}

QRectF QAdobeDocView::documentRect() const
{
    qDebug() << Q_FUNC_INFO;

    if (!m_renderer) return QRectF(rect());

    dpdoc::Matrix mat;
    m_renderer->getNavigationMatrix(&mat);

    double w = mat.a * m_docSize.width();
    double h = mat.d * m_docSize.height();

    if (m_pageMode == MODE_REFLOW_PAGES) {
        w /= m_scale;
        h /= m_scale;
    }

    return QRectF(mat.e, mat.f, w, h);
}

void QAdobeDocView::updateOffsetXY()
{
    if (!m_renderer) return;

    QRectF r = documentRect();
    m_offsetX = -r.x() / r.width();
    m_offsetY = -r.y() / r.height();
}

void QAdobeDocView::updatePageOffset()
{
    qDebug() << Q_FUNC_INFO;

    if (!m_renderer)
        return;
    int count = m_doc->getPageCount() - 1;
    if (count <= 0)
    {
        emit pageNumberChanged(0, 0, 0);
    }
    else
    {
        int end = 0;
        int start = pageNumberForScreen(&end);
        emit pageNumberChanged(start, end, count);
    }
}

bool QAdobeDocView::processEventInPoint ( const QPoint& p )
{
    qDebug() << Q_FUNC_INFO;

    if (m_renderer)
    {
        int x = p.x();
        int y = p.y();

        int pos_start = 0;
        dp::ref<dpdoc::Location> locStart = m_renderer->getScreenBeginning();
        if (locStart) {
            pos_start = locStart->getPagePosition();
        }

        m_transInv.map(x, y, &x, &y);
        QAdobeMouseEvent ae(dpdoc::MOUSE_CLICK, x, y);
        qDebug() << Q_FUNC_INFO << "Before handling event";
        m_renderer->handleEvent(&ae);
        qDebug() << Q_FUNC_INFO << "After handling event";

        int pos_start_new = 0;
        dp::ref<dpdoc::Location> locStartNew = m_renderer->getScreenBeginning();
        if (locStartNew)
        {
            pos_start_new = locStartNew->getPagePosition();
        }

        qDebug() << Q_FUNC_INFO << "Pages: " << pos_start << pos_start_new;

        return pos_start != pos_start_new;
    }
    return false;
}

void QAdobeDocView::mousePressEvent(QMouseEvent* e)
{
    m_clickPos = e->pos();
    QDocView::mousePressEvent(e);
}

void QAdobeDocView::mouseReleaseEvent(QMouseEvent* e)
{
    if (!m_clickPos.isNull()) {
        int len = (e->pos() - m_clickPos).manhattanLength();
        if (m_renderer && e->button() == Qt::LeftButton && len < Q_CLICK_FUZZY_LEN) {
            int x = e->x();
            int y = e->y();
            m_transInv.map(x, y, &x, &y);
            QAdobeMouseEvent ae(dpdoc::MOUSE_CLICK, x, y);
            m_renderer->handleEvent(&ae);
            if (!ae.isRejected()) e->accept();
        }
        m_clickPos = QPoint();
    }

    QDocView::mouseReleaseEvent(e);
}

/*-------------------------------------------------------------------------*/

void QAdobeDocView::resizeEvent(QResizeEvent*)
{
	if(!m_isFormActivated)
	return ;
    delete m_surface;
	m_surface = 0;
    updateViewport();
	updateNavigation();
}

void QAdobeDocView::drawMyNotes( QPainter& painter )
{
    if (!m_notesLocation.isEmpty())
    {
        qDebug() << Q_FUNC_INFO;
        painter.setFont(m_noteFont);

        QRectF rect = documentRect();
        int originX = rect.left(), originY = rect.top();

        qDebug() << Q_FUNC_INFO << ", documentRect: " << rect;

        int count = m_notesLocation.count();
        for (int i = 0; i < count; ++i)
        {
            const QPair<double, double> loc = m_notesLocation.at(i);

            QString label = QString::number(m_previousNotesCount + i + 1);
            int w = m_noteFontMetrics.width(label);
            int h = m_noteFontMetrics.height();
            int x = loc.first + originX - (w>>1);
            int y = loc.second + originY;

            qDebug() << Q_FUNC_INFO << ", Note at (" << loc.first << "," << loc.second << ")";

            painter.fillRect(x , y , w+2 , h+2, Qt::black);
            painter.fillRect(x+1 , y+1 , w , h, Qt::white);
            painter.drawText(x+1 , y+1 , w , h, Qt::AlignCenter, label);// Notes start at 1
        }
    }
}


void QAdobeDocView::drawScrolls(QPainter& painter, const QRect& wr, const QRect& clip)
{
    QRectF dev = documentRect();
    dev = m_trans.mapRect(dev);
	QList<bool> arrowFlagList;
	for(int i = 0; i<4; i++)
		arrowFlagList<<false;
    if (!m_isHorizontal || isHardPageMode()) {
        if (dev.left() < wr.left() - Q_SCROLL_FUZZY_GAP) {
            // draw left scroll
            QRect r = m_leftIcon.rect();
            r.moveCenter(QPoint(wr.x() + (r.width() >> 1) + Q_SCROLL_INTERVAL, (wr.top() + wr.bottom()) >> 1));
            if (clip.intersects(r)) {
				painter.drawPixmap(r, m_leftIcon);
				arrowFlagList[0]=true;
            }
        } 
        
        if (dev.right() > wr.right() + Q_SCROLL_FUZZY_GAP) {
            // draw right scroll
            QRect r = m_rightIcon.rect();
            r.moveCenter(QPoint(wr.right() - (r.width() >> 1) - Q_SCROLL_INTERVAL, (wr.top() + wr.bottom()) >> 1));
            if (clip.intersects(r)) {
				painter.drawPixmap(r, m_rightIcon);
				arrowFlagList[1]=true;
            }
        }
    }

    if (m_isHorizontal || isHardPageMode()) {
        if (dev.top() < wr.top() - Q_SCROLL_FUZZY_GAP) {
            // draw up scroll
            QRect r = m_upIcon.rect();
            r.moveCenter(QPoint((wr.left() + wr.right()) >> 1, wr.y() + (r.height() >> 1) + Q_SCROLL_INTERVAL));
            if (clip.intersects(r)){
				painter.drawPixmap(r, m_upIcon);
				arrowFlagList[2]=true;
            }
        } 
        
        if (dev.bottom() > wr.bottom() + Q_SCROLL_FUZZY_GAP) {
            // draw down scroll
            QRect r = m_downIcon.rect();
            r.moveCenter(QPoint((wr.left() + wr.right()) >> 1, wr.bottom() - (r.height() >> 1) - Q_SCROLL_INTERVAL));
            if (clip.intersects(r)){
				painter.drawPixmap(r, m_downIcon);        
				arrowFlagList[3]=true;
			}
        }
    }
	emit arrowShow(arrowFlagList);
}

void QAdobeDocView::paintEvent(QPaintEvent* event)
{
    qDebug() << Q_FUNC_INFO << event;

    if (m_blockPaintEvents)
        return;

    qDebug() << Q_FUNC_INFO << "Locking mutex";
    QMutexLocker locker(&s_adobeConcurrentMutex);
    qDebug() << Q_FUNC_INFO << "After locking mutex";

    QPainter painter(this);

    m_dirty = m_dirty.intersected(m_surface->rect());
    if (m_dirty.isValid())
    {
        m_surface->clear(m_dirty);
        QRect r(m_dirty.intersected(documentRect().toAlignedRect()));

        if (m_pageMode == MODE_REFLOW_PAGES) // NO EPUB
        {
            r = m_dirty;
        }

        if (r.isValid() && r.width() > 1 && r.height() > 1)
        {
            QTime t;
            t.start();
            qDebug() << Q_FUNC_INFO << "going to render in " << r;
            m_renderer->paint(r.left(), r.top(), r.right() + 1, r.bottom() + 1, m_surface);
            qDebug() << "paint surface time(paintEvent): " << t.elapsed();
        }
    }
    m_dirty.setCoords(0, 0, 0, 0);

    if (m_isHorizontal)
    {
        QRect r = m_transInv.mapRect(event->rect());
        painter.save();
        painter.setTransform(m_trans);
        painter.drawImage(r, *m_surface, r);
        painter.restore();
    }
    else
    {
        painter.drawImage(event->rect(), *m_surface, event->rect());
    }

    m_rectList.clear();
    if(m_isHili && m_hiliBegin != 0 && m_hiliEnd != 0)
    {
        dpdoc::RangeInfo* rInfo = m_renderer->getRangeInfo(m_hiliBegin, m_hiliEnd);
        setHighlightListRect(rInfo);
    }

    //paint the box
    if(!m_rectList.isEmpty())
    {
        painter.setCompositionMode(QPainter::CompositionMode_Exclusion);
        int size = m_rectList.size();
        m_tempHighlightRect = QRect(0, 0, 0, 0);

        for(int i = 0; i < size; ++i)
        {
            const QRect& rect = m_rectList.at(i);
            m_selectingImage = QImage(rect.width(), rect.height(), QImage::Format_RGB32);
            m_selectingImage.fill(qRgb(255, 255, 255));
            painter.drawImage(QPoint(rect.x(), rect.y()), m_selectingImage);

            // Memorize the current highlightning rect and height.
            if(i == 0)
            {
                m_tempHighlightRect.setLeft(rect.left());
                m_tempHighlightRect.setTop(rect.top());

                m_leftHighlightHeight = rect.bottom() - rect.top();
            }

            if(i == size-1)
            {
                m_tempHighlightRect.setRight(rect.right());
                m_tempHighlightRect.setBottom(rect.bottom());

                m_rightHighlightHeight = rect.bottom() - rect.top();
            }

        }
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }

    drawMyNotes(painter);
    drawScrolls(painter, rect(), event->rect());

    qDebug() << Q_FUNC_INFO << "End";
}

int QAdobeDocView::getFontSize() const
{
    return m_scale;
}

int QAdobeDocView::sizeLevel() const
{
    /*if (m_isFontScalable) return (m_scale - Q_FONT_SIZE_MIN) / EPUB_INTERNAL_SIZE;
    else*/                  return m_pdfZoomLevel;
}

void QAdobeDocView::updateScaleByLevel()
{
    setScaleFactor(scaleStep()*m_pdfZoomLevel + minScaleFactor());
}

double QAdobeDocView::getFontSizeListAt(int pos) const
{
    return (pos > m_fontSizeList.count()-1)? 0 : m_fontSizeList[pos];
}

void QAdobeDocView::zoomIn()
{
	/*if(m_isFontScalable)
	{
        if (m_scale < Q_FONT_SIZE_MAX)
            setScaleFactor(m_scale + EPUB_INTERNAL_SIZE);
	}
    else*/
    {
        if (m_pdfZoomLevel + 1 <= MAX_PDF_ZOOM_LEVEL - 1)
            setScaleFactor(pdfScaleStep()*(++m_pdfZoomLevel) + minScaleFactor());
        else if (m_scale < maxScaleFactor())
            setScaleFactor(maxScaleFactor());
    }

    if (m_isPdf) scrolling();

	qDebug() << "set scale factor: " << m_scale;
}

void QAdobeDocView::zoomOut()
{
	/*if(m_isFontScalable)
	{
        if (m_scale > Q_FONT_SIZE_MIN)
            setScaleFactor(m_scale - EPUB_INTERNAL_SIZE);
	}
    else*/
    {
        if (m_pdfZoomLevel - 1 >= 0)
            setScaleFactor(pdfScaleStep()*(--m_pdfZoomLevel) + minScaleFactor());
        else if (m_scale > minScaleFactor())
            setScaleFactor(minScaleFactor());
    }

    if (m_isPdf) scrolling();

	qDebug() << "set scale factor: " << m_scale;
}

void QAdobeDocView::setFormActivated(bool isActivated)
{
	m_isFormActivated = isActivated;
}

void QAdobeDocView::abortProcessing()
{
    qDebug() << Q_FUNC_INFO;
    m_host->stopProcessing();
}

void QAdobeDocView::clearAbortProcessing()
{
    m_host->clearAbortProcessing();
}

double QAdobeDocView::getDocViewXOffsetPercent() const
{
    return m_offsetX;
}

double QAdobeDocView::getDocViewYOffsetPercent() const
{
    return m_offsetY;
 }
 
 QRect QAdobeDocView::getTempHighlightRect()
{
    return m_tempHighlightRect;
}

QPoint QAdobeDocView::getHighlightHeight(int hiliId)
{
    if (!m_renderer) return QPoint(0, 0);

    QPoint initPoint, finishPoint;

    QRect bbox = getHighlightBBox(hiliId);

    initPoint = bbox.topLeft();
    finishPoint = bbox.bottomRight();

    while(highlightAt(finishPoint) != hiliId &&
          finishPoint.x() > 0 &&
          finishPoint.y() > 0)
        finishPoint -= QPoint(1, 1);

    while(highlightAt(initPoint) != hiliId &&
          initPoint.x() < Screen::getInstance()->screenWidth() &&
          initPoint.y() < Screen::getInstance()->screenHeight())
        initPoint += QPoint(1, 1);

    m_transInv.map(initPoint.x(), initPoint.y(), &initPoint.rx(), &initPoint.ry());
    m_transInv.map(finishPoint.x(), finishPoint.y(), &finishPoint.rx(), &finishPoint.ry());

    dp::ref<dpdoc::Location> initLoc = 0, finishloc = 0;
    initLoc = m_renderer->hitTest(initPoint.x(), initPoint.y(), dpdoc::HF_SELECT | dpdoc::HF_FORCE);
    finishloc = m_renderer->hitTest(finishPoint.x(), finishPoint.y(), dpdoc::HF_SELECT | dpdoc::HF_FORCE);

    if (!initLoc || !finishloc) return QPoint(0, 0);

    if(hiliId >= 0)
    {
        dpdoc::RangeInfo* rInfo = m_renderer->getRangeInfo(initLoc, finishloc);
        int boxCount = rInfo->getBoxCount();

        if (rInfo && boxCount > 0)
        {
            dpdoc::Rectangle rect;
            rInfo->getBox(0, false, &rect);
            m_leftHighlightHeight = static_cast<int>(rect.yMax) - static_cast<int>(rect.yMin);
            rInfo->getBox(rInfo->getBoxCount()-1, false, &rect);
            m_rightHighlightHeight = static_cast<int>(rect.yMax) - static_cast<int>(rect.yMin);
        }
        else
        {
            m_leftHighlightHeight = 0;
            m_rightHighlightHeight = 0;
        }
    }
    else
    {
        m_leftHighlightHeight = 0;
        m_rightHighlightHeight = 0;
    }


    return QPoint(m_leftHighlightHeight, m_rightHighlightHeight);
}

QPoint QAdobeDocView::getTempHighlightHeight()
{
    return QPoint(m_leftHighlightHeight, m_rightHighlightHeight);
}
