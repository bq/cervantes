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

#ifndef QADOBEDOCVIEW_H
#define QADOBEDOCVIEW_H

#include "QDocView.h"

#include <dp_core.h>
#include <dp_doc.h>

#define MAX_PDF_ZOOM            4 // n times the original PDF size

class QAdobeClient;
class QAdobeSurface;
typedef QList<QPoint> Points;
class ViewerFloatingNote;
class Viewer;

class QAdobeDocView : public QDocView
{
    Q_OBJECT

    typedef dp::ref<dpdoc::Location> LocationRef;

public:
    static bool staticInit();
    static void staticDone();

    static QAdobeDocView* create(const QString& mimeType, QWidget* parent = 0);

    virtual ~QAdobeDocView();

    virtual QString backend() const;
    virtual QString title() const;

    static QString coverPage(const QString & path, const QString & destination = "");

    virtual QUrl url() const;
    virtual void setUrl(const QUrl& url);

    virtual bool isDrmFile() const;

    virtual PageMode pageMode() const;
    virtual bool isPageModeSupported(PageMode mode);
    virtual bool setPageMode(PageMode mode);

    virtual double autoFitFactor(AutoFitMode mode) const;
    virtual AutoFitMode autoFitMode() const;
    virtual void setAutoFitMode(AutoFitMode mode);

    virtual double scaleStep() const;
    virtual double scaleFactor() const;
    virtual double maxScaleFactor() const;
    virtual double minScaleFactor() const;
    virtual bool setScaleFactor(double factor, double delta_x = 0, double delta_y = 0);
    virtual int sizeLevel() const;
    double pdfScaleStep() const;

    virtual double getDocViewXOffsetPercent() const;
    virtual double getDocViewYOffsetPercent() const;

    virtual int pageCount() const;
    virtual int pageNumberForScreen(int* end = 0) const;
    int getPageEnd() const;
    int getPageBegin() const;

    virtual bool thumbnail(QPaintDevice* map);

    virtual Location* bookmark();
    virtual void gotoBookmark(const QString& ref);
    virtual double getPosFromBookmark(const QString& ref);
    virtual double getPosFromHighlight(const QString& ref);
    virtual int getPageFromMark(const QString& ref);
    virtual void getBookmarkRange(double* start, double* end);
    virtual QString getBookmarkFromPosition(double pos);


    virtual QString wordAt(int x, int y);
    virtual QStringList wordAt(int x, int y, int context_length);

    virtual TableOfContent* tableOfContent();

    virtual SearchResult* search(const QString& text, const QString& tag_fmt = QString(), bool whole_word = false);

    virtual bool isHorizontal() const;
    virtual bool setHorizontal(bool on);

    virtual void setFileExist(bool isExist);
    virtual bool isOpenPWDialog(void);
    virtual bool isHighlightSupported() const;
    virtual int trackHighlight(HighlightMode mode, int x, int y);
    virtual QStringList highlightList() const;
    virtual void setHighlightList(const QStringList& list);
    virtual int highlightCount() const;
    virtual int highlightAt(int x, int y) const;
    virtual int highlightAt(QPoint p) const;
    virtual void removeHighlight(int hili);
    virtual bool eraseHighlight(int x,int y);
    virtual bool isHiliEdited();
    virtual void setEdited(bool edited);
    virtual void clearHighlightList();
    virtual Location* highlightLocation(int hili) const;
    virtual QRect getHighlightBBox(int hili) const;

    virtual void setNoteList(const QList<int>& notes);
    void updatePageNotes();
    virtual void clearNoteList();
    virtual int noteAt(int x, int y) const;

    virtual QString getText(int start, int end) const;

    static QDateTime fromPdfDate(const QString& text);
    static QString toPdfDate(const QDateTime& date);
    static QPixmap * coverPage(const QString & path, QSize& size);

    virtual int    getFontSize() const;

    virtual void   setFormActivated(bool isActivated);
    virtual void   setMargin(double, double, double, double);
    virtual QPoint getTopLeftMargin(void) const;
    virtual void   setBookInfo(const BookInfo &bookInfo);

    virtual double getInitialPosFromRenderer();
    virtual double getFinalPosFromRenderer();

    virtual bool processEventInPoint ( const QPoint& );

    virtual void abortProcessing ();
    virtual void clearAbortProcessing();

    virtual QRect  getTempHighlightRect();
    virtual QPoint getHighlightHeight(int hiliId);
    virtual QPoint getTempHighlightHeight();

    virtual void takeMiniatureScreenshot();
    virtual void scrolling();

signals:
    void arrowShow          (QList<bool>);
    void zoomChange         (double newZoom, double minZoom);
    void pdfZoomLevelChange (int level);

public slots:

    virtual bool goBack();
    virtual bool goForward();

    virtual bool nextScreen();
    virtual bool previousScreen();
    virtual bool gotoPage(int pos);
    virtual void setFontSizeOrScalePercentage(int size);

    virtual void zoomIn();
    virtual void zoomOut();

private slots:
    void updateSurface(const QRect& area);
    void updateUrl(const QUrl& url);
    void updateState(int state);
    void updateNavigation();
    void updateError(const QString& err);
    void handleLicense(const QString& type, const QString& res, const uchar* data, size_t len);
    void handlePassword();

private:
    void updateTitle();
    void updateRenderer();

    bool updateViewport();
    bool updateMetrics();
    bool updateNaturalSize();
    void updatePageOffset();
    void updateOffsetXY();
    QRectF documentRect() const;

    QString highlightToBookmark(dp::ref<dpdoc::Location> start, dp::ref<dpdoc::Location> end) const;
    bool bookmarkToHighlight(const QString& ref, dp::ref<dpdoc::Location>* start, dp::ref<dpdoc::Location>* end) const;
    void releaseHighlightTracking();

    QString handleInput(const QString& prompt);
    void handleHashPassword(const QString& op);
    void handleHiliUnion();
    bool setHighlightListRect(dpdoc::RangeInfo *rInfo);

    void                                checkErrors                         ();

protected:

    virtual void resizeEvent(QResizeEvent* event);
    virtual void paintEvent(QPaintEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

    void drawMyNotes(QPainter& painter);
    void drawScrolls(QPainter& painter, const QRect& wr, const QRect& clip);
    inline bool isHardPageMode() const;

    QAdobeDocView(dpdoc::Document* doc, QAdobeClient* client, QWidget* parent = 0);

    void extractCSS();
    QSet<QString> extractCSSFromEpub();
    void cleanExtractedCSS();
    void addExtraCSSFiles(QSet<QString>&);

    // PDF page change handle.
    bool turnZoomedPdfPage  (bool next);
    bool turnWholePdfPage   (bool next);
    bool goToZoomedPdfPage  (LocationRef &loc, bool next = true);
    bool goToWholePdfPage   (LocationRef& loc);
    bool pdfNextScreen      ();
    bool pdfPreviousScreen  ();
    bool pdfGoToPage        (LocationRef& loc);

    bool screenshot          ();
    void positioningMiniature();

    bool setScaleFactorOnNextZoomedPage     (double factor, double delta_x = 0, double delta_y = 0);
    bool setScaleFactorOnPreviousZoomedPage (double factor, double delta_x = 0, double delta_y = 0);
    bool setScaleFactorGoToZoomedPage       (double factor);
    bool updateScaleAndView                 (double factor);

private:
    QAdobeClient* m_host;
    QAdobeSurface* m_surface;
    dpdoc::Document* m_doc;
    dpdoc::Renderer* m_renderer;
    bool m_isBusy;
    bool m_isFontScalable;
    bool m_isHorizontal;
    bool m_isHili;
    bool m_hiliEdited;
    bool m_tryPassHash;
    bool m_isFileExist;
    bool m_isDoingSetURL;
    bool m_isFormActivated;
    bool m_isPdf;
    double m_scale;
    double m_offsetX;
    double m_offsetY;
    dp::ref<dpdoc::Location> m_hiliBegin;
    dp::ref<dpdoc::Location> m_hiliEnd;
    dp::ref<dpdoc::Location> m_hiliWordStart;
    dp::ref<dpdoc::Location> m_hiliWordEnd;
    dp::ref<dpdoc::Location> m_hiliOri;
    dp::ref<dpdoc::Location> m_hiliMove;
    dp::ref<dpdoc::Location> m_hiliLeft;
    dp::ref<dpdoc::Location> m_hiliRight;
    int m_hiliId;
    QColor m_backColor;
    AutoFitMode m_autoFit;
    PageMode m_pageMode;
    QPixmap m_upIcon;
    QPixmap m_downIcon;
    QPixmap m_leftIcon;
    QPixmap m_rightIcon;
    dp::ref<dpdoc::Location> m_locNext;
    dp::ref<dpdoc::Location> m_locPrev;
    QAdobeSurface* m_surfaceNext;
    QAdobeSurface* m_surfacePrev;
    int m_pushOpStateCount;
    double m_curPos;
    double m_curScale;
    QPoint m_clickPos;
    QRect m_dirty;
    QSize m_viewSize;
    QSizeF m_docSize;

    QTransform m_trans;
    QTransform m_transInv;
    QRect m_wordRect;
    QImage m_selectingImage;
    QImage m_paintImage;
    QString m_hashUser;
    QString m_hashPass;
    QUrl m_url;
    QString m_title;

    double m_curFitFactor;

    QList<int> m_indexList;

    enum {NEXT, PREV, CURR};

    QList<QRect> m_rectList;
    friend class QAdobeClient;

    QList<ViewerFloatingNote*>      m_floatingNotes;
    QList<QPair<double,double> >    m_notesLocation;
    int                             m_previousNotesCount;
    QFontMetrics                    m_noteFontMetrics;

    QRect m_tempHighlightRect;    // It's the highlight's rect under creation.
    int   m_leftHighlightHeight;  // It's the height of the begin of the highlight.
    int   m_rightHighlightHeight; // It's the height of the end of the highlight.
    int   m_tempHighlightHeight;
};

#endif
