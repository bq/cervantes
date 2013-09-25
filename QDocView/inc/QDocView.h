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

#ifndef QDOCVIEW_H
#define QDOCVIEW_H

#include <QWidget>
#include <QUrl>
#include <QList>
#include <QStringList>
#include <QDateTime>

// TRACK ERRORS
#define DEFAULT_TRACK_ERROR      -1
#define NOTHING_TRACK_ERROR      (DEFAULT_TRACK_ERROR - 18)
#define NUM_AVAILABLE_SIZES     8

class BookInfo;

class QDocView : public QWidget
{
    Q_OBJECT

public:
    enum LoadState
    {
        LOAD_NONE,
        LOAD_STARTED,
        LOAD_FINISHED,
        LOAD_FAILED
    };

    enum AutoFitMode
    {
        AUTO_FIT_NONE,
        AUTO_FIT_BEST,
        AUTO_FIT_PAGE,
        AUTO_FIT_WIDTH,
        AUTO_FIT_HEIGHT
    };

    enum PageMode
    {
        MODE_HARD_PAGES,
        MODE_HARD_PAGES_2UP,
        MODE_SCROLL_PAGES,
        MODE_SCROLL,
        MODE_REFLOW_PAGES
    };

    enum HighlightMode
    {
        HIGHLIGHT_BEGIN,
        HIGHLIGHT_TRACK,
        HIGHLIGHT_END,
        HIGHLIGHT_COMMIT
    };
     
    enum ARROWSTATUS
    {
        ARROW_NONE,
        ARROW_UP,
        ARROW_DOWN,
        ARROW_LEFT,
        ARROW_RIGHT
    };

    struct Location
    {
        int         page;
        double      pos;
        QString     ref;
        QString     preview;
        int         depth;

	Location()
		: page(0)
		, pos(0)
		{}

	Location(const Location& other)
        : page(other.page)
        , pos(other.pos)
        , ref(other.ref)
		, preview(other.preview)
        {}

        void updateLoc(const Location& other)
		{
			page = other.page;
			pos = other.pos;
			ref = other.ref;
			preview = other.preview;
		}
    };

    struct TableOfContent
    {
        virtual ~TableOfContent() {}
        virtual QString title() = 0;
        virtual Location* location() = 0;
        virtual int itemCount() = 0;
        virtual TableOfContent* itemAt(int i) = 0;
        virtual int getDepth() = 0;
        virtual void setDepth(int) = 0;
    };

    struct SearchResult
    {
        virtual ~SearchResult() {}
        virtual bool hasNext() = 0;
        virtual Location* next() = 0;
    };

public:
    static void staticInit();
    static void staticDone();

    static QString qlabelEntityEncode(const QString & text);

    static QString guessMimeType(const QUrl& url);
    static bool isMimeTypeSupported(const QString& mimeType);
    static QStringList supportedFileTypes();
    static QDocView* create(const QString& mimeType, const QString& path, QWidget* parent = 0);
    static QString coverPage(const QString& path, const QString & destination = "");
    static int getMimeTypeViewer(const QString& mimeType);
    static QPixmap *coverPage(const QString & path, QSize& size);

    virtual ~QDocView();

    virtual QString backend() const = 0;
    virtual QString title() const = 0;
    virtual bool isDrmFile() const = 0;

    virtual QUrl url() const = 0;
    virtual void setUrl(const QUrl& url) = 0;

    virtual PageMode pageMode() const = 0;
    virtual bool isPageModeSupported(PageMode mode) = 0;
    virtual bool setPageMode(PageMode mode) = 0;

    virtual double autoFitFactor(AutoFitMode mode) const = 0;
    virtual AutoFitMode autoFitMode() const = 0;
    virtual void setAutoFitMode(AutoFitMode mode) = 0;

    virtual double scaleStep() const = 0;
    virtual double scaleFactor() const = 0;
    virtual double maxScaleFactor() const = 0;
    virtual bool   setScaleFactor(double factor, double delta_x = 0, double delta_y = 0) = 0;
    virtual void   setMargin(double top, double right, double bottom, double left) = 0;
    virtual QPoint getTopLeftMargin(void) {return QPoint();}
    virtual int    sizeLevel() const = 0;
    virtual int    pageCount() const = 0;
    virtual int    pageNumberForScreen(int* end = 0) const = 0;
    virtual int    getPageEnd() const = 0;
    virtual int getPageBegin() const =0;

    virtual double getDocViewXOffsetPercent() const = 0;
    virtual double getDocViewYOffsetPercent() const = 0;

    virtual QSize sizeHint() const;

    virtual bool thumbnail(QPaintDevice* map) = 0;

    virtual Location* bookmark() = 0;
    virtual void gotoBookmark(const QString& ref) = 0;
    virtual void getBookmarkRange(double* start, double* end) = 0;
    virtual double getPosFromBookmark(const QString& ref) = 0;
    virtual double getPosFromHighlight(const QString& ref) = 0;
    virtual int getPageFromMark(const QString& ref) = 0;
    virtual QString getBookmarkFromPosition(double pos) = 0;

    virtual QList<int> noteList() const;
    virtual void setNoteList(const QList<int>& notes);
    virtual void clearNoteList();
    virtual int noteAt(int x, int y) const;

    virtual QString wordAt(int x, int y);
    virtual QStringList wordAt(int x, int y, int context_length);
    virtual TableOfContent* tableOfContent();

    virtual SearchResult* search(const QString& text, const QString& tag_fmt = QString(), bool whole_word = false) = 0;

    virtual bool isHorizontal() const;
    virtual bool setHorizontal(bool on);
    virtual void setFileExist(bool isExist);
    virtual bool isOpenPWDialog(void);
    virtual bool isHighlightSupported() const;
    virtual int trackHighlight(HighlightMode mode, int x, int y);
    virtual QStringList highlightList() const;
    virtual void setHighlightList(const QStringList& list);
    virtual void setHighlightList(const QStringList& list, const QStringList& testList);
    virtual int highlightCount() const;
    virtual int highlightAt(int x, int y) const;
    virtual void removeHighlight(int hili);
    virtual bool eraseHighlight(int x, int y );
    virtual bool isHiliEdited();
    virtual void setEdited(bool edited);
    virtual void clearHighlightList();
    virtual Location* highlightLocation(int hili) const;
    virtual QRect getHighlightBBox(int hili) const;

    virtual QString getText(int start, int end = -1) const = 0;

    static bool isHanzi(const QChar& ch);
    static int fuzzyCompare(double a, double b);
    static QString extractWord(const QString& text, int pos, bool isHiliMode = false);
    static QDateTime fromIsoDate(const QString& text);
    static QString toIsoDate(const QDateTime& date);

    virtual void setDictionaryMode(bool state) ;
    virtual void setHighlightMode(bool state);

    virtual void setBookInfo(const BookInfo& bookInfo);
    virtual int getFontSize() const = 0;
    virtual double minScaleFactor() const = 0;
    virtual void setFormActivated(bool isActivated);
    virtual bool isSearchSupported() const;
    QString getHwrPath(const QString& path, const double& size, const double& pos);
    void setBlockPaintEvents(bool block) { m_blockPaintEvents = block; }
    virtual double getInitialPosFromRenderer() = 0;
    virtual double getFinalPosFromRenderer() = 0;

    virtual bool processEventInPoint ( const QPoint& ) { return false; }

    virtual void abortProcessing () = 0;
    virtual void clearAbortProcessing() = 0;

    virtual void takeMiniatureScreenshot() = 0;
    virtual void scrolling() = 0;

    inline bool getCanOverrideFonts() const { return m_canOverrideFonts; }

    enum EDocViewLoadingError
    {
        EDVLE_NONE =                0,
        EDVLE_LICENSE_ERROR =       (1u << 1),
        EDVLE_PASSWORD_ERROR =      (1u << 2),
        EDVLE_GENERIC_ERROR =       (1u << 3),
        EDVLE_FATAL_ERROR =         (1u << 4)
    };


    virtual QRect  getTempHighlightRect()    {return QRect();}
    virtual QPoint getHighlightHeight(int)   {return QPoint();}
    virtual QPoint getTempHighlightHeight()  {return QPoint();}


public slots:
    virtual bool goBack();
    virtual bool goForward();
    virtual bool nextScreen() = 0;
    virtual bool previousScreen() = 0;
    virtual bool gotoPage(int pos) = 0;	
    virtual void setFontSizeOrScalePercentage(int size) = 0;
    virtual void zoomIn() = 0;
    virtual void zoomOut() = 0;
    
signals:
    void                                stateChanged                        ( int state );
    void                                fatalOccurred                       ();
    void                                errorOccurred                       ( QString msg, int errorMask );
    void                                warningOccurred                     ( QStringList warnings );

    void titleChanged(const QString& title);
    void urlChanged(const QUrl &url);
    void pageNumberChanged(int start, int end, int count);
    void scaleFactorChanged(double factor);
    void arrowChanged(ARROWSTATUS status);
    void closePWDlg(void);
//    void                                noteClicked                         ( int index );

protected:
    explicit QDocView(QWidget* parent = 0);

    void drawNotes(QPainter& painter, const QRect& wr, const QRect& clip);

    double m_topMargin;
    double m_bottomMargin;
    double m_leftMargin;
    double m_rightMargin;


protected:
    QList<int> m_notes;
    QPixmap* m_noteIcon;
    QFont m_noteFont;
    int m_noteGap;
    bool m_dictionaryMode;
    bool m_highlightMode;
    bool m_blockPaintEvents;
    bool m_canOverrideFonts;
    static QString s_path;
    const BookInfo *m_bookInfo;

private:
    static const QString antiword;
	static const QString utf8;
};

inline bool QDocView::isHanzi(const QChar& ch)
{
    int u = ch.unicode();
    return (u >= 0x3000 && u <= 0x9fff) 
        || (u >= 0xf900 && u <= 0xfaff)
        || (u >= 0xff00 && u <= 0xff5e);
}

inline int QDocView::fuzzyCompare(double a, double b)
{
    int d = (int)(a * 65536) - (int)(b * 65536);
    return (d > -10 && d < 10) ? 0 : d;
}

#endif
