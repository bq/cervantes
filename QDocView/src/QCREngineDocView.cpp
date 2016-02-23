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

#include "QBook.h" 

#include <QSettings>
#include <QPainter>
#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <QTextLayout>
#include <QTimer>
#include <QDebug>
#include <QDir>
#include <cmath>
#include "sys/types.h"
#include "sys/sysinfo.h"

#include "Screen.h"
#include "Fb2MetaDataExtractor.h"
#include "PowerManager.h"
#include "QBookApp.h"
#include "ConfirmDialog.h"
#include "InfoDialog.h"

/// min ram free, after which swapping to disk should occur
#ifndef DOCUMENT_CACHING_MIN_RAM_FREE
#define DOCUMENT_CACHING_MIN_RAM_FREE 0xF00000 // 15Mb
#endif

#define CR_FONT_SIZE_DEFAULT 22.5
#define CR_FONT_SIZE_MAX_LEVEL 7
#define CR_FONT_SIZE_MIN_LEVEL 0
#define CR_FONT_SIZE_BEST_LEVEL 3
const double CR_FONT_SIZE_600_DOC[CR_FONT_SIZE_MAX_LEVEL+1]    = {12.0, 16.0, 19.0, 22.0, 26.0, 31.0, 36.0, 42.0};
const double CR_FONT_SIZE_600_NO_DOC[CR_FONT_SIZE_MAX_LEVEL+1] = {16.0, 20.0, 24.0, 27.0, 31.0, 36.0, 42.0, 51.0};
const double CR_FONT_SIZE_758_DOC[CR_FONT_SIZE_MAX_LEVEL+1]    = {14.0, 18.0, 22.0, 26.0, 30.0, 37.0, 43.0, 49.0};
const double CR_FONT_SIZE_758_NO_DOC[CR_FONT_SIZE_MAX_LEVEL+1] = {21.0, 27.0, 32.0, 37.0, 42.0, 50.0, 58.0, 66.0};

const double* CR_FONT_SIZE = 0;

#define thumbnail_width 150
#define thumbnail_height 180


#define TXT_LAYOUT_MARGIN       5
#define TXT_PAGE_CALC_UNITS     25
#define TXT_PAGE_UPDATE_UNITS   20

#ifdef MX508  
#define CR_FONT_PATH        "/usr/lib/fonts"
#define CR_CSS_FILE_PATH    "/app/etc/fb2.css"
#define CR_HYPH_FILE_PATH   "/app/etc/Russian_EnUS_hyphen_(Alan).pdb"
#define CR_NO_SPACING_FILE  "file:///app/res/noSpacing.css"
#endif

#ifndef Q_WS_QWS
#define CR_FONT_PATH        ROOTFS"/usr/lib/fonts"
#define CR_CSS_FILE_PATH    "QHome/etc/fb2.css"
#define CR_HYPH_FILE_PATH   "QHome/etc/Russian_EnUS_hyphen_(Alan).pdb"
#define CR_NO_SPACING_FILE  "QHome/res/noSpacing.css"
#endif

#define CR_DEFAULT_FONT_FAMILY      "Noticia Text"
#define CR_DEFAULT_JUSTIFY_VALUE    2

#include <QFontDatabase>
#include <QLayout>
#include "QCREngineDocView.h"

static lString8 readFileToString( const char * fname ) 
{
	lString8 buf;
	LVStreamRef stream = LVOpenFileStream(fname, LVOM_READ);
	if (!stream)
		return buf;
	int sz = stream->GetSize();
	if (sz>0)
	{
		buf.insert( 0, sz, ' ' );
		stream->Read( buf.modify(), sz, NULL );
	}
	return buf;
}

static void initHyph(const char * fname)
{
	HyphMan hyphman;
	LVStreamRef stream = LVOpenFileStream( fname, LVOM_READ);
	if (!stream)
		return;
	//FIXME crengine3
	//HyphMan::activateDictionaryFromStream( stream );

}

lString16 QStringTolString16(QString str)
{
    return lString16(str.toUtf8().constData());
}

QString lString16ToQString(lString16 str)
{
    lString8 s8 = UnicodeToUtf8(str);
    return QString::fromUtf8(s8.c_str(), s8.length());
}

LVDocView * QCREngineDocView::text_view = NULL;

QCREngineDocView::QCREngineDocView(QWidget* parent) : QDocView(parent)
    , m_scale(CR_FONT_SIZE_DEFAULT)
    , m_jump(-1)
    , m_page(0)
    , m_hiliEdited(false)
    , fontMan(0)
    ,dx(0)
    ,dy(0)
    , lvdoc_fullHeight(0)
    , lvdoc_curPageStart(0)
    , lvdoc_curPageHeight(0)
    , bStopResize(true)
    , viewPortSize(QSize(0,0))
    , rotViewPortSize(QSize(0,0))
    , docReady(false)
{
    bool isDoc = s_path.toLower().endsWith(".doc");

    if(QBook::getInstance()->getResolution() == QBook::RES758x1024)
    {
        CR_FONT_SIZE = isDoc ? CR_FONT_SIZE_758_DOC : CR_FONT_SIZE_758_NO_DOC;
    }
    else //QBook::RES600x800
    {
        CR_FONT_SIZE = isDoc ? CR_FONT_SIZE_600_DOC : CR_FONT_SIZE_600_NO_DOC;
    }

    m_fontSizeStep = 1.5;
    m_fontSizeMin = CR_FONT_SIZE[CR_FONT_SIZE_MIN_LEVEL];
    m_scale = CR_FONT_SIZE[CR_FONT_SIZE_BEST_LEVEL];

    for(int i = CR_FONT_SIZE_MIN_LEVEL; i <= CR_FONT_SIZE_MAX_LEVEL; i++)
    {
        m_fontSizeList << CR_FONT_SIZE[i];
    }

    QMargins layoutMargins = parent->layout()->contentsMargins();

    viewPortSize.setHeight(parent->height() - layoutMargins.top() - layoutMargins.bottom());
    viewPortSize.setWidth(parent->width() - layoutMargins.left() - layoutMargins.right());
    qDebug() << "Initializing font manager with path: " << QBook::fontsDirPath();
  
    if(!InitFontManager( lString8(QBook::fontsDirPath().toUtf8().constData()) ))
    {
    	qDebug("InitFontManager failed");
    }
    
    fontMan = ::fontMan;
    initHyph(CR_HYPH_FILE_PATH);

    fontMan->gc();

    qDebug() << "Installing font in cr3: " << fontMan->RegisterFont(lString8("IstokWeb-Regular.ttf"));
    qDebug() << "Installing font in cr3: " << fontMan->RegisterFont(lString8("Cabin-Regular.ttf"));
    qDebug() << "Installing font in cr3: " << fontMan->RegisterFont(lString8("Vera.ttf"));
    qDebug() << "Installing font in cr3: " << fontMan->RegisterFont(lString8("Vollkorn-Regular.ttf"));
    qDebug() << "Installing font in cr3: " << fontMan->RegisterFont(lString8("NoticiaText-Regular.ttf"));
    qDebug() << "Installing font in cr3: " << fontMan->RegisterFont(lString8("DroidSerif-Regular.ttf"));
    qDebug() << "Installing font in cr3: " << fontMan->RegisterFont(lString8("Tiresias-Regular.ttf"));

    text_view = new LVDocView();

    // stylesheet can be placed to file fb2.css
    // if not found, default stylesheet will be used
    /*lString8 css = readFileToString(CR_CSS_FILE_PATH);

    if (css.length() > 0)
        text_view->setStyleSheet( css );*/

    text_view->setBackgroundColor(0xFFFFFF);
    text_view->setTextColor(0);
    text_view->setViewMode(DVM_PAGES, 1);
    text_view->setFontSize((int)m_scale);
    /* FIXME: try for crengine3
    CRPropRef props = LVCreatePropsContainer();
    props->setString("crengine.highlight.selection.color", "0");
    text_view->propsApply(props);
    */
    LVArray <int>sizesArray;

    for(int i = 0; i<8 ; i++)
        sizesArray.add(int(m_fontSizeList[i]));

    text_view->setFontSizes(sizesArray,false);
    text_view->setPageHeaderInfo(0);
}

QCREngineDocView::~QCREngineDocView(){
    if ( text_view )
    {
        delete text_view;
        text_view = NULL;
    }

    if( fontMan )
    {
        ShutdownFontManager();
        fontMan = NULL;
    }
}




void QCREngineDocView::startResize()
{
	bStopResize = false;
}

void QCREngineDocView::stopResize()
{
	bStopResize = true;
}

QUrl QCREngineDocView::url() const
{
    return m_url;
}

void QCREngineDocView::setupAppearance()
{
    setDocFont();
    setDefaultSpacing();
    setDefaultJustification();
}

void QCREngineDocView::restartApp()
{
    qDebug() << Q_FUNC_INFO << "*********************************RESTART APP*********************************";;
    QApplication::quit();
}

void QCREngineDocView::doRestartApp()
{
    QBookApp::instance()->enableUserEvents();
    QBookApp::instance()->getStatusBar()->setBusy(false);

    ConfirmDialog* confirmDialog = new ConfirmDialog(this,tr("The device must be rebooted."));
    Screen::getInstance()->setMode(Screen::MODE_SAFE,true,FLAG_FULLSCREEN_UPDATE,Q_FUNC_INFO);
    confirmDialog->exec();
    delete confirmDialog;

    QBookApp::instance()->disableUserEvents();

    qDebug() << Q_FUNC_INFO << "*********************************RESTART*********************************";
    PowerManagerLock* lock = PowerManager::getNewLock(this);
    lock->activate();    
    QBookApp::instance()->cancelSync();
    QBookApp::instance()->showRestoringImage();
    Screen::getInstance()->lockScreen();
    QBookApp::instance()->syncModel();
    QBook::settings().sync();
    restartApp();
}

bool QCREngineDocView::enoughMemoryAvailable()
{
    qDebug() << Q_FUNC_INFO;

    struct sysinfo memInfo;
    sysinfo (&memInfo);
    long long freeRam;
    sysinfo (&memInfo);
    freeRam = memInfo.freeram;
    freeRam *= memInfo.mem_unit;

    qDebug() << Q_FUNC_INFO << "********************************************************************** FREE RAM: " << freeRam;

    if (freeRam < DOCUMENT_CACHING_MIN_RAM_FREE)
    {
        system("sync && sysctl -w vm.drop_caches=3");
        sysinfo (&memInfo);
        freeRam = memInfo.freeram;
        freeRam *= memInfo.mem_unit;
        qDebug() << Q_FUNC_INFO << "**********************************************************************TRY TO FREE UP MEMORY: " << freeRam;
    }

    return (freeRam > DOCUMENT_CACHING_MIN_RAM_FREE);
}

void QCREngineDocView::setUrl(const QUrl& url)
{
    m_url = url;
    m_canOverrideFonts = true;

    QFile file(url.toLocalFile());
    
    emit stateChanged(LOAD_STARTED);
    
    if (file.size() <=0 ||file.size() > TXT_MAX_FILE_SIZE || !file.open(QFile::ReadOnly)) 
    {    
        emit stateChanged(LOAD_FAILED);
        emit errorOccurred(tr("Can't open the book!"), EDVLE_GENERIC_ERROR);
        return;
    }

    if(!fontMan || !fontMan->GetFontCount())
    {
    	qDebug("[QCREngineDocView] GetFontCount failed"); 
        emit stateChanged(LOAD_FAILED);
        emit errorOccurred(tr("Can't open the book!"), EDVLE_GENERIC_ERROR);
    	return;
    }

    setupAppearance();

    // Prevent memory overload.
    if (!enoughMemoryAvailable()) doRestartApp();

    qDebug("[QCREngineDocView]  LoadDocument .... ");
    if(!text_view|| !text_view->LoadDocument((const char *) url.toLocalFile().toUtf8()))
    {
    	qDebug("[QCREngineDocView]  LoadDocument failed");
        emit stateChanged(LOAD_FAILED);
        emit errorOccurred(tr("Can't open the book!"), EDVLE_GENERIC_ERROR);
    	return;
    }

    docReady = true;
    QTimer::singleShot(0, this, SLOT(updateLoadState()));
    qDebug("[QCREngineDocView]  setUrl end ");
}

void QCREngineDocView::updateLoadState()
{
    emit stateChanged(LOAD_FINISHED);

    if(dx!=text_view->GetWidth() || dy!=text_view->GetHeight() )
    {
        if(viewPortSize.width() !=0 && viewPortSize.height()!= 0)
		{
            text_view->Resize(viewPortSize.width(), viewPortSize.height());
			dx = viewPortSize.width() ;
            dy = viewPortSize.height();
        }
        else
        {
            text_view->Resize(600, 681);
            dx = 600;
            dy = 681;
        }
	 }
         qDebug("[QCREngineDocView]  updateLoadState ");
}

/*-------------------------------------------------------------------------*/

QDocView::PageMode QCREngineDocView::pageMode() const
{
    return MODE_SCROLL;
}

bool QCREngineDocView::isPageModeSupported(PageMode mode)
{
    return mode == MODE_SCROLL;
}

bool QCREngineDocView::setPageMode(PageMode mode)
{
    return mode == MODE_SCROLL;
}

QString QCREngineDocView::backend() const
{
    return "fb2";
}

QString QCREngineDocView::title() const
{
    return QString::fromWCharArray(text_view->getTitle().c_str(), text_view->getTitle().length() );
}

/*-------------------------------------------------------------------------*/

void QCREngineDocView::displayFit(AutoFitMode mode)
{
    setScaleFactor(autoFitFactor(mode));
}

double QCREngineDocView::autoFitFactor(AutoFitMode mode) const
{
    return (mode == AUTO_FIT_BEST) ? CR_FONT_SIZE[CR_FONT_SIZE_BEST_LEVEL] : m_fontSizeMin;
}

QDocView::AutoFitMode QCREngineDocView::autoFitMode() const
{
    return AUTO_FIT_WIDTH;
}

void QCREngineDocView::setAutoFitMode(AutoFitMode mode)
{
    if (mode == AUTO_FIT_PAGE || mode == AUTO_FIT_WIDTH) {
        setScaleFactor(m_fontSizeMin);
    }
}

double QCREngineDocView::scaleStep() const
{
    return m_fontSizeStep;
}

double QCREngineDocView::maxScaleFactor() const
{
    return CR_FONT_SIZE[CR_FONT_SIZE_MAX_LEVEL];
}

double QCREngineDocView::minScaleFactor() const 
{
    return m_fontSizeMin;
}


double QCREngineDocView::scaleFactor() const
{
    return m_scale;
}

bool QCREngineDocView::setScaleFactor(double factor, double delta_x, double delta_y)
{
    qDebug() << Q_FUNC_INFO << "M_SCALE: " << m_scale;
    qDebug() << Q_FUNC_INFO << "FACTOR : " << factor;

    if (m_scale > factor)
        text_view->ZoomFont(factor - m_scale);
    else if(factor > m_scale)        
    	text_view->ZoomFont(factor -m_scale);

    if( m_scale != factor)
    {   
     	m_scale = text_view->getFontSize();
     	update();
	emit scaleFactorChanged(m_scale);	
        updatePageNumber();
    }
    
    return !delta_x && !delta_y;
}

double QCREngineDocView::getFontSizeListAt(int pos) const
{
    return (pos > m_fontSizeList.count()-1)? 0 : m_fontSizeList[pos];
}

/*-------------------------------------------------------------------------*/
extern lString16 fitTextWidthWithEllipsis( lString16 text, LVFontRef font, int maxwidth );
QDocView::Location* QCREngineDocView::bookmark()
{
	//LVRendPageList* pagelist = text_view->getPageList();
     	//int view_start =(*pagelist)[text_view->getCurPage()]->start;
	Location* loc = new Location;
	loc->page = m_page;   
    	loc->pos = (double) lvdoc_curPageStart / lvdoc_fullHeight;	
    	//loc->ref = QString("pos=%1").arg(loc->pos);
    	//loc->ref = QString("pos=%1,scale=%2").arg(loc->pos).arg( m_scale);
    	ldomXPointer bm= text_view->getBookmark();
    	lString16	bmstr = bm.toString();
    	loc->ref = QString::fromWCharArray(bmstr.c_str(), bmstr.length() );
    	//lString16 titleText;
    	lString16 posText;
	//lString16	bmstr = bm.toString();

    	//text_view->getBookmarkPosText(bm, titleText, posText);    	

	//posText= fitTextWidthWithEllipsis(posText, text_view->getInfoFont(), 350*3);
	if(m_page==0)
	{
		posText = text_view->getAuthors();
		posText += text_view->getTitle();
		posText += text_view->getSeries();
		loc->preview = QString::fromWCharArray( posText.c_str());
	}
	else
	{
		posText = text_view->getPageText(true);		
	    	loc->preview = QString::fromWCharArray( posText.c_str());
	}
		
		
/*
    if (pos + 200 >= m_text.length()) {
        loc->preview = m_text.mid(pos);
    } else {
        loc->preview = m_text.mid(pos, 200);
        loc->preview += tr(" ...");
    }
	
    loc->preview = loc->preview.simplified();
	*/
    return loc;
}

double QCREngineDocView::getPosFromBookmark(const QString& /*ref*/){
    return 0.0;
}

double QCREngineDocView::getPosFromHighlight(const QString& /*ref*/){
    return 0.0;
}

int QCREngineDocView::getPageFromMark(const QString& ref)
{
    if(!docReady)
        return -1;

    ldomXPointer bm;
    QStringList args = ref.split(QChar('|'));
    if (args[0] == "HL_START")
    {
        wchar_t* startPStr = new wchar_t[args[1].size()+1];
        memset(startPStr, 0, sizeof(wchar_t)*(args[1].size()+1));
        args[1].toWCharArray(startPStr);
        lString16 startString(startPStr);
        bm = text_view->getDocument()->createXPointer(startString);
        delete startPStr;
    }else
    {
        wchar_t* w_ref = new wchar_t[ref.size()+1];
        memset(w_ref, 0, sizeof(wchar_t)*(ref.size()+1));
        ref.toWCharArray(w_ref);
        lString16 s_ref(w_ref);
        bm = text_view->getDocument()->createXPointer(s_ref);
        delete w_ref;
    }

    if(bm.isNull())
        return -1;

    return text_view->getBookmarkPage(bm);
}

QString QCREngineDocView::getBookmarkFromPosition(double /*pos*/){
    return "";
}

void QCREngineDocView::gotoBookmark(const QString& ref) 
{
    qDebug() << Q_FUNC_INFO << ": ref " << ref;
    //if (!ref.startsWith("pos=")) return;
    //m_jump = ref.mid(4).toDouble();
    if(!docReady)
    	return ;
    ldomXPointer bm;
    QStringList args = ref.split(QChar('|'));
    if (args[0] == "HL_START") {
		wchar_t* startPStr = new wchar_t[args[1].size()+1];
		memset(startPStr, 0, sizeof(wchar_t)*(args[1].size()+1));
		args[1].toWCharArray(startPStr);
		lString16 startString(startPStr);
                bm = text_view->getDocument()->createXPointer(startString);
		delete startPStr;
    } else{
		wchar_t* w_ref = new wchar_t[ref.size()+1];
		memset(w_ref, 0, sizeof(wchar_t)*(ref.size()+1));
		ref.toWCharArray(w_ref);
		lString16 s_ref(w_ref);
                bm = text_view->getDocument()->createXPointer(s_ref);
		delete w_ref;
   }
   if(bm.isNull())
   	return;

   updateLoadState();
   text_view->goToBookmark(bm);
   updatePageNumber();
   update();
   
/*
    int coma = ref.indexOf("scale=");
	m_jump = ref.mid(4, coma-5).toDouble();
	m_scale= ref.mid(coma+6).toDouble();
*/
    //QTimer::singleShot(0, this, SLOT(jumpToPosition()));
}

void QCREngineDocView::getBookmarkRange(double* start, double* end)
{
    if (m_page < 0 || !text_view->IsRendered())
    {
        *start = 0;
        *end = 0;
        return;
    }

    int view_start =lvdoc_curPageStart;
    int view_end =lvdoc_curPageStart+lvdoc_curPageHeight;
    
    *start = (double) view_start/lvdoc_fullHeight;
    *end =  (double) view_end / lvdoc_fullHeight;
}

void QCREngineDocView::jumpToPosition()
{

    if (m_jump < 0) return;
    text_view->SetPos(lvdoc_fullHeight*m_jump);
    update();	
    
}

bool QCREngineDocView::thumbnail(QPaintDevice* map)
{
    QPixmap wid = QPixmap::grabWidget(this);
    QPainter painter(map);
    QRect r = wid.rect();
    double scale = qMin(double(map->width()) / r.width(), double(map->height()) / r.height());
    r.setWidth(int(r.width() * scale));
    r.setHeight(int(r.height() * scale));
    r.moveCenter(QPoint(map->width() / 2, map->height() / 2));
    painter.drawPixmap(r, wid);
    return true;
}

QString QCREngineDocView::coverPage(const QString & path, const QString & destination)
{
    QString filename = QString::fromUtf8(":/BookList/default_pdf");
    qDebug() << "QCREngineDocView::coverPage creating renderer engine for " << path;

    //FIXME
    return filename;

    QCREngineDocView* doc = new QCREngineDocView(/*this->parentWidget()*/);
    if (!doc) return filename;

    doc->setUrl(path);

    QSize size = QSize(thumbnail_width,thumbnail_height);
    // FIXME: does this matter?
    QSize oldSize = QSize(400,200);
    doc->setViewPortSize(size);
    QResizeEvent *event = new QResizeEvent(size, oldSize);
    doc->startResize();
    qDebug() << "[coverPage]: resizing rendering engine to:" << size;
    doc->resizeEvent(event);
    doc->stopResize();
    delete event;

    LVDocImageRef img = doc->text_view->getPageImage( 0 );
    if (!img) {
	delete doc;
	return filename;
    }
    qDebug() << "[coverPage]: getting draw buffer from rendering";
    LVDrawBuf * drawBuf = img->getDrawBuf();
    if (!drawBuf) {
	delete doc;
	return filename;
    }

    QImage image(size,QImage::Format_RGB32);
    QPainter painter(&image);

    int  _dx, _dy;
    unsigned char * _data =0;
    _dx = drawBuf->GetWidth();
    _dy = drawBuf->GetHeight(); 
    //FIXME bqReader
    //_data = drawBuf->getDrawBuffer();
    QRect t = image.rect();
    painter.fillRect(t, Qt::white);
    qDebug() << "[coverPage]: drawing image";
    painter.drawImage(t, QImage(_data ,_dx, _dy, drawBuf->GetRowSize(),QImage::Format_RGB32));   

    QFileInfo fi(path);

    filename = QBook::getThumbnailPath(path);
    filename = fi.absolutePath() + "/" + ".thumbnail" + "/" + fi.fileName();
    filename.append(".jpg");

    QDir dir = fi.dir();
    if (!dir.exists(".thumbnail"))
        dir.mkdir(".thumbnail");

    qDebug() << "[coverPage] saving image: "<< filename;
    bool ret = image.convertToFormat(QImage::Format_RGB32,Qt::AutoColor).scaled(thumbnail_width,thumbnail_height,Qt::KeepAspectRatio,Qt::SmoothTransformation).save(filename,"JPG",90);
    qDebug() << "[coverPage] Cover Page" << (ret ? "successfully!" : "wrong") << "(End)";

    return filename;
}
/*-------------------------------------------------------------------------*/
#define DEFAULT_PAGE_MARGIN	8

QString QCREngineDocView::wordAt(int x, int y)
{
	QString word;

	// skip cite text
	if(y > lvdoc_curPageHeight+DEFAULT_PAGE_MARGIN)
		return word;

	ldomXPointer ptr = text_view->getNodeByPoint( lvPoint( x, y ) );
	if ( !ptr.isNull() ) 
	{
		if ( ptr.getNode()->isText() ) 
		{
			ldomXRange * wordRange = new ldomXRange();
			if ( ldomXRange::getWordRange( *wordRange, ptr ) ) 
			{
				/*
				wordRange->setFlags( 0x10000 );
				text_view->getDocument()->getSelections().clear();
				text_view->getDocument()->getSelections().add( wordRange );
				text_view->updateSelections();
				update();
				*/

				QString selectedText = QString::fromWCharArray( wordRange->getRangeText().c_str(), wordRange->getRangeText().length() );    
				int start = 0;
				int end = selectedText.length()-1;
				for (int i = 0; i < selectedText.length(); i++) 
				{
				        QChar ch = selectedText.at(i);
				        if (!ch.isSpace() &&  !ch.isPunct() )
				        {
				        	start = i;
				        	break;
				        }					
				}				
    
				 for (int i = selectedText.length()-1; i>=0; i--) 
				 {
				        QChar ch = selectedText.at(i);
				        if (!ch.isSpace() &&  !ch.isPunct() )
				        {
				        	end = i;
							break;
				        }					
			     }

				if(end>=start)
				 	word = selectedText.mid(start, end-start+1);		
    		}
			delete wordRange;
    	}
	}	
	qDebug() << "========= word : " << word << "==========";
	return word;
 
}

/*-------------------------------------------------------------------------*/

struct QCREngineSearchResult : public QDocView::SearchResult
{
    QCREngineSearchResult(LVDocView* doc, const QString& text, const QString& tag_fmt, bool word_only);
    virtual ~QCREngineSearchResult();

    virtual bool hasNext();
    virtual QDocView::Location* next();
    QDocView::Location* current();
	
private:
    QString m_word;
    QString m_tag_fmt;
    int m_pos;
    int m_page;
    bool m_word_only;
    LVDocView* m_doc;
    QDocView::Location* m_loc;
};

QCREngineSearchResult::QCREngineSearchResult(LVDocView* doc, const QString& text, const QString& tag_fmt, bool word_only)
    : m_word(text)
    , m_tag_fmt(tag_fmt)
    , m_pos(0)
    , m_page(0)
    , m_word_only(word_only)
    , m_doc(doc)
    , m_loc(0)
{

}

QCREngineSearchResult::~QCREngineSearchResult()
{
    delete m_loc;
}

QDocView::Location* QCREngineSearchResult::current()
{
    QString page_text ;

    if (m_loc) return m_loc;

   if (m_pos < 0 && m_page >= m_doc->getPageCount()-1) return 0;   
    do
    {    
	     lString16 doc_page_text = m_doc->getPageText(true, m_page);
	     page_text = QString::fromWCharArray( doc_page_text.c_str(), doc_page_text.length());	     

	     if(m_pos<0) m_pos=0;

           for (;;) 
           {
	            m_pos = page_text.indexOf(m_word, m_pos, Qt::CaseInsensitive);
		     if (m_pos < 0) break;
	            if (!m_word_only) break;

	            if (m_pos > 0) 
	            {
		            QChar ch = page_text.at(m_pos - 1);
		            if (!ch.isSpace() && !ch.isPunct() ) continue;
	            }
		        if (m_pos + 1 < page_text.length()) 
	            {
		            QChar ch = page_text.at(m_pos + 1);
		            if (!ch.isSpace() && !ch.isPunct() ) continue;
	            }
	            break;
           }

		if(m_pos<0 && m_page >= m_doc->getPageCount()-1)
	    	return 0;
	    else if(m_pos<0 && m_page < m_doc->getPageCount()-1)	    
	       m_page++;	       
	}while( m_pos < 0);

    m_loc = new QDocView::Location;          
    
    m_loc->page = m_page;
    
    int lvdoc_fullHeight =  m_doc->GetFullHeight();
    LVRendPageList* pagelist = m_doc->getPageList(); 	
    int lvdoc_curPageStart =  (*pagelist)[m_doc->getCurPage()]->start;
//    int lvdoc_curPageHeight =  (*pagelist)[m_doc->getCurPage()]->height;

    m_loc->pos = (double) lvdoc_curPageStart / lvdoc_fullHeight;	
       
    ldomXPointer bm= m_doc->getPageBookmark(m_page);
    lString16	bmstr = bm.toString();
    m_loc->ref = QString::fromWCharArray(bmstr.c_str(), bmstr.length() );


	if(m_page < m_doc->getPageCount()-1)
	{
		lString16 doc_next_page_text = m_doc->getPageText(m_page+1);
		QString next_page_text = QString::fromWCharArray( doc_next_page_text.c_str(), doc_next_page_text.length());	    
	    	page_text += next_page_text;
    }    	


    int start = qMax(0, m_pos - 10);
    int end = qMin(page_text.length(), m_pos + m_word.length() + 150);

    if (start < m_pos) 
    {
        m_loc->preview += QString("...");
        // m_loc->preview += page_text.mid(start, m_pos - start);
        m_loc->preview += QDocView::qlabelEntityEncode(page_text.mid(start, m_pos - start));
    }

    if (m_tag_fmt.isEmpty()) 
    {
        //m_loc->preview += m_word;     
        m_loc->preview += QDocView::qlabelEntityEncode(m_word);
    } else {
        //m_loc->preview += m_tag_fmt.arg(m_word);        
        m_loc->preview += m_tag_fmt.arg(QDocView::qlabelEntityEncode(m_word));
	}

    m_pos += m_word.length();

    if (m_pos < end) 
{
        //m_loc->preview += page_text.mid(m_pos, end - m_pos);   
        m_loc->preview += QDocView::qlabelEntityEncode(page_text.mid(m_pos, end - m_pos));
        m_loc->preview += QString("...");
}

    m_loc->preview = m_loc->preview.simplified();

    return m_loc;
}

bool QCREngineSearchResult::hasNext()
{
    return current() != 0;
}

QDocView::Location* QCREngineSearchResult::next()
{
    QDocView::Location* loc = current();
    m_loc = 0;
    return loc;
}


QDocView::SearchResult* QCREngineDocView::search(const QString& text, const QString& tag_fmt, bool word_only)
{
    return new QCREngineSearchResult(this->text_view, text, tag_fmt, word_only);
}


/*-------------------------------------------------------------------------*/

QString QCREngineDocView::getText(int /*start*/, int /*end*/) const
{
	return QString();
}

int QCREngineDocView::pageCount() const
{
    return text_view->getPageCount();      
}

int QCREngineDocView::pageNumberForScreen(int* end) const
{
    int page = qMax(0, m_page);
    if (end) *end = page;
    return page;
}

int QCREngineDocView::getPageEnd() const
{
    int page = qMax(0, m_page);
    return page;
}

int QCREngineDocView::getPageBegin() const
{
    return 0;
}

bool QCREngineDocView::nextScreen()
{
   if (m_page >= pageCount()) return false;

    doCommand( DCMD_PAGEDOWN, 1 );
    return true;  
}

bool QCREngineDocView::previousScreen()
{
    if (m_page <= 0) return false;

   doCommand( DCMD_PAGEUP, 1 );
    return true;

}

bool QCREngineDocView::gotoPage(int pos)
{
    doCommand( DCMD_GO_PAGE, pos );
    return true;
}

void QCREngineDocView::updatePageNumber()
{
    lvdoc_fullHeight =  text_view->GetFullHeight();
    LVRendPageList* pagelist = text_view->getPageList();
    lvdoc_curPageStart =  (*pagelist)[text_view->getCurPage()]->start;
    lvdoc_curPageHeight =  (*pagelist)[text_view->getCurPage()]->height;

    int count = pageCount() - 1;
    m_page = text_view->getCurPage();

    qDebug() << Q_FUNC_INFO << ": pageStart = " << m_page << ", count " << count;
    emit pageNumberChanged(m_page, m_page, count);
}

void QCREngineDocView::setMargin(double top, double right, double bottom, double left)
{
    qDebug() << Q_FUNC_INFO << left << top << right << bottom;
    text_view->setPageMargins(lvRect(left, top, right, bottom));
}

void QCREngineDocView::setDefaultSpacing() const
{    
    // add css file spacing
#ifdef Q_WS_QWS
    QFile spacingSettings(QUrl::fromUserInput(QBook::settings().value("setting/reader/spacing/cr3", CR_NO_SPACING_FILE).toString()).path());
#else
    QFile spacingSettings(QBook::settings().value("setting/reader/spacing/cr3", CR_NO_SPACING_FILE).toString());
#endif

    if (!spacingSettings.open(QFile::ReadOnly))
    {
        qDebug() << Q_FUNC_INFO << "Error reading file" << spacingSettings.fileName();
        return;
    }

    qDebug() << Q_FUNC_INFO << "Copying file" << spacingSettings.fileName();

    QString text = spacingSettings.readAll().data();
    spacingSettings.close();

    QRegExp spacing("(\\d+)");

    if(text.contains(spacing))
    {
        qDebug() << Q_FUNC_INFO << "spacing is:" << spacing.cap(1) << "%";
        // set default interline space, percent (100..200)
        text_view->setDefaultInterlineSpace(spacing.cap(1).toInt());
    }
}

void QCREngineDocView::setDefaultJustification() const
{
    /*CRPropRef props = LVCreatePropsContainer();

    int justifyValue = QBook::settings().value("setting/reader/justify/cr3", CR_DEFAULT_JUSTIFY_VALUE).toInt();

    if(justifyValue == 0)
        props->setString("styles.def.align", "text-align: justify");
        //spacing.append("\ntext-align:justify; text-justify:auto;");
    else if(justifyValue == 1)
        props->setString("styles.def.align", "text-align: left");
        //spacing.append("\ntext-align:left;");

    CRPropRef fail = text_view->propsApply(props);*/

    /**********************************************/

    //text_view->setStyleSheet(lString8((char *)QString("text-align: left").toUtf8().data()));

    /**********************************************/

    //CRPropRef props = text_view->propsGetCurrent();
    /*CRPropRef props = LVCreatePropsContainer();
    int count = props->getCount();
    while(count--)
    {
        if(props->getName(count) == QString("styles.title.align"))
        {
            qDebug() << Q_FUNC_INFO << "ENCONTRADO" << props->getName(count) << lString16ToQString(props->getValue(count));
            break;
        }
    }

    if(count != -1)
    {
        props->setValue(count, lString16((char *)QString("text-align: left").toUtf8().data()));
        qDebug() << Q_FUNC_INFO << "NUEVO VALOR" << props->getName(count) << lString16ToQString(props->getValue(count));
        text_view->propsApply(props);
    }

    props->setString("styles.def.align", "text-align: left");
    props->setStringDef("styles.def.align", "text-align: left");
    text_view->propsApply(props);*/

    /*/// clear all items
    virtual void clear() = 0;
    /// returns property item count in container
    virtual int getCount() const = 0;
    /// returns property name by index
    virtual const char * getName( int index ) const = 0;
    /// returns property value by index
    virtual const lString16 & getValue( int index ) const = 0;
    /// sets property value by index
    virtual void setValue( int index, const lString16 &value ) = 0;*/

    /**********************************************/

}

void QCREngineDocView::resizeEvent(QResizeEvent* event)
{
    qDebug("[resizeEvent] width = %d, height = %d", event->size().width(), event->size().height() );

    dx = event->size().width();
    dy = event->size().height();

    if((dx!= viewPortSize.width() && dx!= rotViewPortSize.width() && dx != parentWidget ()->width() && dx != parentWidget ()->height() ) ||
            (dy!=viewPortSize.height() && dy!= rotViewPortSize.height() && dy != parentWidget ()->height() && dy != parentWidget ()->width())
      )
        return;

    
    //if(m_state >= LOAD_FINISHED && !bStopResize)
    if( !bStopResize)
    {
        if(dx!=text_view->GetWidth() || dy!=text_view->GetHeight() )
        {
             qDebug("text_view->Resize");
             if(!m_dictionaryMode)
                text_view->Resize(event->size().width(), event->size().height());
        }

        QDocView::resizeEvent(event);

    }

    updatePageNumber();



}

/*-------------------------------------------------------------------------*/


void QCREngineDocView::paintEvent(QPaintEvent* event)
{   		
    QPainter painter(this);

/*
    if((dx!=viewPortSize.width() && dx!= rotViewPortSize.width() && dx != parentWidget ()->width() && dx != parentWidget ()->height() ) ||
            (dy!=viewPortSize.height() && dy!= rotViewPortSize.height() && dy != parentWidget ()->height() && dy != parentWidget ()->width()) ||
       bStopResize)
    {
    	if(!m_dictionaryMode)
       	return ;
    }
*/

    qDebug("[QCREngineDocView]  paintEvent ");

    text_view->checkRender();
    if (!text_view->IsRendered()) {
	qDebug() << "Not rendered!";
	return;
    }
    

    QRect rc = rect();
    //FIXME crengine3 we make a copy because of a weird double free we get
    LVDocImageRef *ref = new LVDocImageRef(text_view->getPageImage(0));
    if ( ref->isNull() ) {
        return;
    }
    LVDrawBuf * buf = ref->get()->getDrawBuf();
    int dx = buf->GetWidth();
    int dy = buf->GetHeight();
    int bpp = buf->GetBitsPerPixel();
    qDebug() << "dx: " << dx << "dy: " << dy << "bpp: " << bpp;
    if (bpp == 4 || bpp == 3) {
        QImage img(dx, dy, QImage::Format_RGB16 );
        for ( int i=0; i<dy; i++ ) {
            unsigned char * dst = img.scanLine( i );
            unsigned char * src = buf->GetScanLine(i);
            for ( int x=0; x<dx; x++ ) {
                lUInt16 cl = *src; //(*src << (8 - bpp)) & 0xF8;
                cl = (cl << 8) | (cl << 3) | (cl >> 3);
                src++;
                *dst++ = (cl & 255);
                *dst++ = ((cl >> 8) & 255);
//                *dst++ = *src++;
//                *dst++ = 0xFF;
//                src++;
            }
        }
	painter.drawImage(QPoint(0,0), img, QRect(0,0,dx,dy));
        //painter.drawImage( rc, img );
    } else if (bpp == 2) {

        QImage img(dx, dy, QImage::Format_RGB16 );
        for ( int i=0; i<dy; i++ ) {
            unsigned char * dst = img.scanLine( i );
            unsigned char * src = buf->GetScanLine(i);
            int shift = 0;
            for ( int x=0; x<dx; x++ ) {
                lUInt16 cl = *src; //(*src << (8 - bpp)) & 0xF8;
                lUInt16 cl2 = (cl << shift) & 0xC0;
                cl2 = cl2 | (cl2 >> 2);
                cl2 = (cl2 << 8) | (cl2 << 3) | (cl2 >> 3);
                if ((x & 3) == 3) {
                    src++;
                    shift = 0;
                } else {
                    shift += 2;
                }
                *dst++ = (cl2 & 255);
                *dst++ = ((cl2 >> 8) & 255);
            }
        }

	painter.drawImage(QPoint(0,0), img, QRect(0,0,dx,dy));
	img.save("/tmp/image.png");
        //painter.drawImage( rc, img );

    } else if (bpp == 1) {
        QImage img(dx, dy, QImage::Format_RGB16 );
        for ( int i=0; i<dy; i++ ) {
            unsigned char * dst = img.scanLine( i );
            unsigned char * src = buf->GetScanLine(i);
            int shift = 0;
            for ( int x=0; x<dx; x++ ) {
                lUInt16 cl = *src; //(*src << (8 - bpp)) & 0xF8;
                lUInt16 cl2 = (cl << shift) & 0x80;
                cl2 = cl2 ? 0xffff : 0x0000;
                if ((x & 7) == 7) {
                    src++;
                    shift = 0;
                } else {
                    shift++;
                }
                *dst++ = (cl2 & 255);
                *dst++ = ((cl2 >> 8) & 255);
//                *dst++ = *src++;
//                *dst++ = 0xFF;
//                src++;
            }
        }
	painter.drawImage(QPoint(0,0), img, QRect(0,0,dx,dy));
        //painter.drawImage( rc, img );

    } else if (bpp == 16) {
        QImage img(dx, dy, QImage::Format_RGB16 );
        for ( int i=0; i<dy; i++ ) {
            unsigned char * dst = img.scanLine( i );
            unsigned char * src = buf->GetScanLine(i);
            for ( int x=0; x<dx; x++ ) {
                *dst++ = *src++;
                *dst++ = *src++;
//                *dst++ = *src++;
//                *dst++ = 0xFF;
//                src++;
            }
        }
        painter.drawImage( rc, img );
    } else if (bpp == 32) {
        QImage img(dx, dy, QImage::Format_RGB32 );
        for ( int i=0; i<dy; i++ ) {
            unsigned char * dst = img.scanLine( i );
            unsigned char * src = buf->GetScanLine(i);
            for ( int x=0; x<dx; x++ ) {
                *dst++ = *src++;
                *dst++ = *src++;
                *dst++ = *src++;
                *dst++ = 0xFF;
                src++;
            }
        }
	painter.drawImage(QPoint(0,0), img, QRect(0,0,dx,dy));
        //painter.drawImage( rc, img );
    }
    //FIXME crengine3
    //drawNotes(painter, rect(), event->rect());

}

void QCREngineDocView::doCommand( LVDocCmd cmd, int param)
{
    qDebug("[QCREngineDocView] doCommand cmd = %d, param = %d", cmd, param);
     text_view->doCommand( cmd, param );    	
     update();
     updatePageNumber();
}

  /*-------------------------------------------------------------------------*/
  
  struct QCREngineTOC : public QDocView::TableOfContent
  {
	  QCREngineTOC(LVDocView* doc, LVTocItem* toc);
	  virtual ~QCREngineTOC();
  
	  virtual QString title();
	  virtual QDocView::Location* location();
	  virtual int itemCount();
	  virtual QDocView::TableOfContent* itemAt(int i);
          virtual int getDepth();
          virtual void setDepth(int);
  
  private:
	  LVDocView* m_doc;
	  LVTocItem* m_toc;
          int depth;
  };
  
  QCREngineTOC::QCREngineTOC(LVDocView* doc, LVTocItem* toc)
	  : m_doc(doc)
	  , m_toc(toc)
          , depth(0)
  {
  }
  
  QCREngineTOC::~QCREngineTOC()
  {
	  
  }
  
  QString QCREngineTOC::title()
  {
	  return QString::fromWCharArray( m_toc->getName().c_str(), m_toc->getName().length() );
  }
  
  QDocView::Location* QCREngineTOC::location()
  {
	   QDocView::Location* result = new QDocView::Location();
	   result->page = m_toc->getPage()+1; 
	   result->pos = result->page / m_doc->getPageCount();
	   result->ref = QString::fromWCharArray( m_toc->getXPointer().toString().c_str(), m_toc->getXPointer().toString().length());
       result->preview = title();
       result->depth = getDepth();
	   return result;

  }
  
  int QCREngineTOC::getDepth(){
      return depth;
  }
  void QCREngineTOC::setDepth(int _depth){
      depth = _depth;
  }


  int QCREngineTOC::itemCount()
  {
	  return m_toc->getChildCount();
  }
  
  QDocView::TableOfContent* QCREngineTOC::itemAt(int i)
  {
	  LVTocItem* item = m_toc->getChild(i);
	  if (!item) return 0;
	  return new QCREngineTOC(m_doc, item);
  }
  
  QDocView::TableOfContent* QCREngineDocView::tableOfContent()
  {
	  LVTocItem* item = text_view->getToc();
	  if (!item) return 0;
	  return new QCREngineTOC(text_view, item);
  }

int QCREngineDocView::getFontSize() const
{
    return text_view->getFontSize();
}

 void QCREngineDocView::setDocFont () const
 {
    QString fontName = QBook::settings().value("setting/reader/font/cr3", CR_DEFAULT_FONT_FAMILY).toString();
    qDebug() << "Applying font: " << fontName;
 	lString8 font = lString8((char *)fontName.toUtf8().data());
    text_view->setDefaultFontFace(font);
 }

int QCREngineDocView::sizeLevel() const
{
	return m_fontSizeList.indexOf(m_scale);
}

void QCREngineDocView::zoomIn()
{
	if(sizeLevel() < m_fontSizeList.size() - 1){
		setScaleFactor(m_fontSizeList.at(sizeLevel() + 1));
	}
}

void QCREngineDocView::zoomOut()
{
	if(sizeLevel() > 0){
		setScaleFactor(m_fontSizeList.at(sizeLevel() - 1));
	}
}
int QCREngineDocView::trackHighlight(HighlightMode mode, int x, int y)
{
	ldomXPointer ptr = text_view->getNodeByPoint( lvPoint( x, y ) );
	ldomXRange *highlightRange;
	if ( !ptr.isNull() ) 
	{
		if ( ptr.getNode()->isText()) 
		{
			ldomXRange * wordRange = new ldomXRange();
			if ( ldomXRange::getWordRange( *wordRange, ptr ) ) 
			{
				QString selectedText = QString::fromWCharArray( wordRange->getRangeText().c_str(), wordRange->getRangeText().length());    
				if(selectedText.isEmpty())
				{
					if(mode == HIGHLIGHT_BEGIN){
						m_touchSpace = true;
					}
					else if(mode == HIGHLIGHT_END){
						highlightRange = new ldomXRange(m_lastTrackStart, m_lastTrackEnd);
						ldomXRange * range = highlightUnion(highlightRange);
						text_view->getDocument()->getSelections().remove(text_view->getDocument()->getSelections().length()-1);
							//FIXME: crengine3
							//range->setBgBlack(false);
							text_view->getDocument()->getSelections().add(range);
						m_locList.append(HLLoc(range));
					}
				}
				else
				{
					int length = text_view->getDocument()->getSelections().length();
					wordRange->setFlags( 0xFFFFF );
					//text_view->getDocument()->getSelections().clear();
					//if(mode == HIGHLIGHT_BEGIN)
					if(mode == HIGHLIGHT_BEGIN)
					{
						//FIXME: crengine3
						//wordRange->setBgBlack(true);
						text_view->getDocument()->getSelections().add(wordRange);
						m_rangeStart = wordRange->getStart();
						m_rangeEnd = wordRange->getEnd();
						m_touchSpace = false;
					}
					else if(length!= 0 && (!m_touchSpace))
					{
						if(m_rangeStart.compare(wordRange->getStart()) > -1)
							highlightRange = new ldomXRange(wordRange->getStart(), m_rangeEnd);
						else
							highlightRange = new ldomXRange(m_rangeStart, wordRange->getEnd());
						m_lastTrackStart = highlightRange->getStart();
						m_lastTrackEnd = highlightRange->getEnd();
						text_view->getDocument()->getSelections().remove(length-1);

						if(mode == HIGHLIGHT_TRACK){
							//FIXME: crengine3
							//highlightRange->setBgBlack(true);
							text_view->getDocument()->getSelections().add(highlightRange);
						}
						//delete highlightRange;
						else if(mode == HIGHLIGHT_END){
							ldomXRange * range = highlightUnion(highlightRange);
								//FIXME: crengine3
								//range->setBgBlack(false);
								text_view->getDocument()->getSelections().add(range);
							m_locList.append(HLLoc(range));
						}
					}			
				}
				//FIXME: crengine3
                                //text_view->updateSelectionsEx(false);
				update();
			}
			//delete wordRange;
		}
	}
	m_hiliEdited = true;
	return 0;
}


QDocView::Location* QCREngineDocView::HLLoc(ldomXRange *highlightRange)
{
	Location* loc = new Location;
	lvdoc_fullHeight = text_view->GetFullHeight();
	//ldomXPointer highlightPtr(startPtr.getStart(), endPtr.getEnd());
	ldomXPointerEx rangeStart = highlightRange->getStart();
	ldomXPointerEx rangeEnd = highlightRange->getEnd();

	loc->page = text_view->getBookmarkPage(rangeStart);   
	LVRendPageList* pagelist = text_view->getPageList();
	int HLPageStart =  (*pagelist)[text_view->getBookmarkPage(rangeStart)]->start;
   	loc->pos = (double) HLPageStart / lvdoc_fullHeight;	
   	lString16	startPstr = rangeStart.toString();
	lString16 endPstr = rangeEnd.toString();
   	loc->ref = "HL_START|" + QString::fromWCharArray(startPstr.c_str(), startPstr.length() );
	loc->ref = loc->ref + "|HL_END|" + QString::fromWCharArray(endPstr.c_str(), endPstr.length() );

   	lString16 posText;

	ldomXRange *pos = new ldomXRange(rangeStart, rangeEnd);

	QString wordRange = QString::fromWCharArray(pos->getRangeText().c_str(), pos->getRangeText().length());
	if(m_page==0)
	{
		posText = text_view->getAuthors();
		posText += text_view->getTitle();
		posText += text_view->getSeries();
		loc->preview = QString::fromWCharArray( posText.c_str());
	}
	else
	{
	    loc->preview = wordRange;
	}
	delete pos;
    return loc;
}


bool QCREngineDocView::eraseHighlight(int x, int y)
{
	int id = highlightId(x, y);
	if(id >= 0)
	{
		removeHighlight(id);
		m_hiliEdited = true;
		return true;
	}
	//FIXME: crengine3
        //text_view->updateSelectionsEx(false);
	update();
	return false;

}

int QCREngineDocView::highlightId(int x, int y)
{
	ldomXRange * wordRange = wordAtRange(x,y);
        for(int i = 0; i < text_view->getDocument()->getSelections().length(); i++)
	{
		ldomXRange *highlightRange = text_view->getDocument()->getSelections().get(i);
		if((highlightRange->getStart().compare(wordRange->getStart()) != 1) && (highlightRange->getEnd().compare(wordRange->getEnd()) != -1))
		{
			return i;
		}
	}
	
	return -1;
}

void QCREngineDocView::removeHighlight(int id)
{
	ldomXRangeList selections = text_view->getDocument()->getSelections();
	if (selections.empty()) {
		qDebug() << Q_FUNC_INFO << "selections empty, not removing id " << id;
		return;
	}
	
	selections.remove(id);	
	m_locList.remove(id);
}

ldomXRange * QCREngineDocView::wordAtRange(int x, int y)
{
	ldomXPointer ptr = text_view->getNodeByPoint( lvPoint( x, y ) );
	ldomXRange * wordRange = new ldomXRange();
	ldomXRange::getWordRange( *wordRange, ptr );
	return wordRange;
}

QDocView::Location* QCREngineDocView::highlightLocation(int id) const
{
	return m_locList.at(id);
}

bool QCREngineDocView::isHiliEdited()
{
	return m_hiliEdited;
}
void QCREngineDocView::setEdited(bool edited)
{
	m_hiliEdited = edited;
}
void QCREngineDocView::setHighlightList(const QStringList& list)
{
	clearHighlightList();
	for (int i = 0; i < list.size(); i++)
	{
		ldomXRange * highlightRange = bookmarkToHighlight(list.at(i));
		if (!highlightRange->isNull()){
			text_view->getDocument()->getSelections().add( highlightRange );
			m_locList.append(HLLoc(highlightRange));
		}
    }
	//FIXME: crengine3
        //text_view->updateSelectionsEx(false);
	update();
}


void QCREngineDocView::clearHighlightList()
{
    m_locList.clear();
	text_view->getDocument()->getSelections().clear();
}
int QCREngineDocView::highlightCount() const
{
	return m_locList.size();
}
ldomXRange* QCREngineDocView::bookmarkToHighlight(const QString &ref) const
{
	QStringList args = ref.split(QChar('|'));
	
    if (args[0] == "HL_START") {
		wchar_t* startPStr = new wchar_t[args[1].size()+1];
		wchar_t* endPStr = new wchar_t[args[3].size()+1];
		memset(startPStr, 0, sizeof(wchar_t)*(args[1].size()+1));
		memset(endPStr, 0, sizeof(wchar_t)*(args[3].size()+1));
		args[1].toWCharArray(startPStr);
		args[3].toWCharArray(endPStr);
		lString16 startString(startPStr);
		lString16 endString(endPStr);

		ldomXPointer startPointer = text_view->getDocument()->createXPointer(startPStr);
		ldomXPointer endPointer = text_view->getDocument()->createXPointer(endPStr);
		//delete startPStr;
		//delete endPStr;
		return new ldomXRange(startPointer, endPointer);
    } else {
		
        return new ldomXRange();
    }
}

ldomXRange* QCREngineDocView::intersect(ldomXRange * preRange, ldomXRange * newRange)
{
	ldomXPointerEx aStart = preRange->getStart();
	ldomXPointerEx aEnd = preRange->getEnd();
	ldomXPointerEx bStart = newRange->getStart();
	ldomXPointerEx bEnd = newRange->getEnd();

	if(aStart.compare(bStart) == 1)
	{
		if(aStart.compare(bEnd) == 1)
			return new ldomXRange();
		else
		{
			if(bEnd.compare(aEnd) == 1)
				return new ldomXRange(bStart, bEnd);
			else
				return new ldomXRange(bStart, aEnd);
		}
	}
	else
	{
		if(bStart.compare(aEnd) == 1)
			return new ldomXRange();
		else
		{
			if(aEnd.compare(bEnd) == 1)
				return new ldomXRange(aStart, aEnd);
			else
				return new ldomXRange(aStart, bEnd);
		}
	}

}
ldomXRange* QCREngineDocView::highlightUnion(ldomXRange * range)
{
	QList<int> record;
	ldomXRange *preHighlightRange = new ldomXRange();
	ldomXRange *tempRange = new ldomXRange();
	if(!m_locList.isEmpty()){
		for(int i = 0; i < m_locList.size(); i++){
			preHighlightRange = bookmarkToHighlight(m_locList.at(i)->ref);
			if(!preHighlightRange->isNull()){
				tempRange = intersect(preHighlightRange, range);
				if(!tempRange->isNull()){
					record.append(i);
					range = tempRange;										
				}
			}
		}
	}
	/*
	else{
		delete preHighlightRange;
		//delete tempRange;
		return new ldomXRange();		
	}
	*/
	for(int i = record.count()-1; i >= 0; i--)
	{
		removeHighlight(record.at(i));
	}
	delete preHighlightRange;
	//delete tempRange;
	return range;
}

double QCREngineDocView::getInitialPosFromRenderer()
{
    return 0;
}

double QCREngineDocView::getFinalPosFromRenderer()
{
    return 0;
}
