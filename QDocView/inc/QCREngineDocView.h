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

#ifndef QCRENGINEDOCVIEW_H
#define QCRENGINEDOCVIEW_H


#include "QDocView.h"

#include <QVector>
#include <QFontMetrics>
#include <QBasicTimer>
#include "QBook.h"

#include "lvdocview.h"

class QCREngineDocView : public QDocView
{
    Q_OBJECT

public:
    explicit QCREngineDocView(QWidget* parent = 0);
    virtual ~QCREngineDocView();

    virtual QString backend() const;
    virtual QString title() const;

    virtual QUrl url() const;
    virtual void setUrl(const QUrl& url);

    virtual bool isDrmFile() const { return false; }

    virtual PageMode pageMode() const;
    virtual bool isPageModeSupported(PageMode mode);
    virtual bool setPageMode(PageMode mode);

    virtual double autoFitFactor(AutoFitMode mode) const;
    virtual AutoFitMode autoFitMode() const;
    virtual void setAutoFitMode(AutoFitMode mode);

    virtual double scaleStep() const;
    virtual double scaleFactor() const;
    virtual double maxScaleFactor() const;
    virtual void setMargin(double, double, double, double);
    virtual double minScaleFactor() const;
    virtual void setDefaultSpacing() const;
    virtual void setDefaultJustification() const;

    virtual int getFontSize() const;
    virtual void setDocFont() const;

    void setupAppearance();

    virtual bool setScaleFactor(double factor, double delta_x = 0, double delta_y = 0);
	virtual int sizeLevel() const;
	QDocView::Location* HLLoc(ldomXRange *);
	virtual bool isHiliEdited();
	virtual void setHighlightList(const QStringList& list);
	virtual int highlightCount() const;
	virtual void setEdited(bool);
	virtual void clearHighlightList();
	bool eraseHighlight(int, int);
	int highlightId(int, int);
	void removeHighlight(int);
    ldomXRange * wordAtRange(int, int);
    virtual QDocView::Location* highlightLocation(int) const;
	ldomXRange* bookmarkToHighlight(const QString&) const;
    virtual int pageCount() const;
    virtual int pageNumberForScreen(int* end = 0) const;
    virtual int getPageEnd() const;
    virtual int getPageBegin() const;

    virtual double getDocViewXOffsetPercent() const {return 0;}
    virtual double getDocViewYOffsetPercent() const {return 0;}

    virtual bool thumbnail(QPaintDevice* map);

    virtual Location* bookmark();
    virtual void gotoBookmark(const QString& ref);
    virtual void getBookmarkRange(double* start, double* end);

    virtual QString wordAt(int x, int y);

    virtual SearchResult* search(const QString& text, const QString& tag_fmt = QString(), bool whole_word = false);

    static QString coverPage(const QString & path, const QString & destination = "");

   virtual double getPosFromBookmark(const QString& ref);
   virtual double getPosFromHighlight(const QString& ref);
   virtual QString getBookmarkFromPosition(double pos);
   virtual int getPageFromMark(const QString& ref);
   void setViewPortSize(QSize size) { viewPortSize=size;}
   void setRotateViewPortSize(QSize size) { rotViewPortSize=size;}
   void startResize();
   void stopResize();



   virtual double getInitialPosFromRenderer();
   virtual double getFinalPosFromRenderer();
   virtual void abortProcessing(){}
   virtual void clearAbortProcessing(){}

   virtual void takeMiniatureScreenshot(){}
   virtual void scrolling(){}


public slots:
    virtual bool    nextScreen                      ();
    virtual bool    previousScreen                  ();
    virtual bool    gotoPage                        (int pos);
    virtual void    setFontSizeOrScalePercentage    (int size);
    virtual int     trackHighlight                  (HighlightMode, int, int);
    virtual void    zoomIn                          ();
    virtual void    zoomOut                         ();

private slots:
    void            jumpToPosition                  ();
    void            updateLoadState                 ();
    void            updatePageNumber                ();

protected:
    virtual void    resizeEvent                     (QResizeEvent* event);
    virtual void    paintEvent                      (QPaintEvent* event);
    QString         getText                         (int start, int end) const;

    void doCommand( LVDocCmd cmd, int param=0 );
    LVDocView* getDocView() { return text_view;}		
	
   QDocView::TableOfContent* tableOfContent();

private:
    QUrl m_url;
    double m_scale;
    double m_jump;
    int m_page;

    // HIGHLIGHT
	ldomXRange* intersect(ldomXRange*, ldomXRange*);
	ldomXRange* highlightUnion(ldomXRange*);
	QVector<Location*> m_locList;
	ldomXPointerEx m_rangeStart;
	ldomXPointerEx m_rangeEnd;
	bool m_hiliEdited;
	bool m_touchSpace;
	ldomXPointerEx m_lastTrackStart;
	ldomXPointerEx m_lastTrackEnd;

    static LVDocView * text_view;
    LVFontManager * fontMan;
    int dx;
    int dy;
    int lvdoc_fullHeight;
    int lvdoc_curPageStart;
    int lvdoc_curPageHeight;
    bool bStopResize;
    QSize viewPortSize;
    QSize rotViewPortSize;

    bool docReady;
    QList<int> m_fontSizeList;
   double m_fontSizeMin;
   double m_fontSizeStep;

    friend class QTextSearchResult;
};

#endif
