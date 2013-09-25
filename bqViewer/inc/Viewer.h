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

#ifndef VIEWER_H
#define VIEWER_H

#include "ui_Viewer.h"
#include "QDocView.h"
#include "QBookForm.h"

#include <QBasicTimer>
#include <QTime>
#include <QStack>
#include <QDateTime>
#include <QTimer>
#include <QList>

class QWidget;
class QString;
class QKeyEvent;
class QLabel;
class QMouseEvent;
class QTime;
class QProcess;
class QTimer;

class QDocView;
class MouseFilter;
class TouchEvent;
class SharePopup;
class BookInfo;
class BookLocation;
class PowerManagerLock;
class FloatingNote;
class Keyboard;
class ViewerMenu;
class ViewerMenuPopUp;
class ViewerSearchPopup;
class ViewerAppearancePopup;
class ViewerGotoPopup;
class ViewerBookSummary;
class ViewerContentsPopup;
class ViewerMarkHandler;
class ViewerDictionary;
class ViewerStepsManager;
class QDateTime;

#define SCROLL_AREA_WIDTH 120

enum HighlightPopupType {
    INIT_SINGLE_WORD_POPUP,
    DEFAULT_POPUP
};

class Viewer : public QBookForm,
               public Ui::Viewer
{
    Q_OBJECT

public:
    enum SupportedExt {
        EXT_NO_SUPPORTED,
        EXT_PDF,
        EXT_EPUB,
        EXT_FB2,
        EXT_MOBI,
        EXT_DOC,
        EXT_CHM,
        EXT_TXT,
        EXT_RTF,
        EXT_HTML,
        EXT_TCR,
        EXT_PDB,
        EXT_ZIP
    };

protected:
    enum ScreenArea {
        TOP_LEFT,
        BOTTOM_LEFT,
        TOP_RIGHT,
        BOTTOM_RIGHT,
        CENTER
    };

    enum Scroll {
        NO_SCROLL,
        SCROLL_LEFT,
        SCROLL_RIGHT,
        SCROLL_UP,
        SCROLL_DOWN
    };

public:
    Viewer(QWidget* parent = 0);
    virtual ~Viewer();

    void                                                    activateForm                                ();
    void                                                    deactivateForm                              ();
    Keyboard*                                               showKeyboard                                ( const QString& text, bool showUp = false );
    Keyboard*                                               hideKeyboard                                ();

    inline bool                                             hasOpenBook                                 () const { return !qs_docPath.isEmpty(); }
    inline int                                              getCurrentDocExt                            () const { return i_docExtension; }
    inline int                                              getFontSize                                 () const;
    int                                                     pageMode                                    () const;
    void                                                    endHiliMode                                 () { b_hiliMode = false; }
    ViewerMarkHandler*                                      markHandler                                 () { return m_markHandler;}
    QDocView*                                               docView                                     () { return m_docView; }
    QString                                                 docPath                                     () const { return qs_docPath; }
    QString                                                 getCurrentChapter                           () const { return m_chapterTitle; }
    int                                                     getPageStart                                () const { return m_pageStart; }
    BookInfo*                                               getCurrentBookInfo                          () const {return m_bookInfo;}
    void                                                    previousScreen                              ();
    bool                                                    nextScreen                                  ();

    static int                                              getFileExtension                            ( const QString& );
    static bool                                             isSupportedFile                             ( const QString& );
    static SupportedExt                                     isCR3SupportedFile                          ( const QString& );
    static bool                                             isUsingCR3                                  ( const QString& );
    static bool                                             isUsingCR3                                  ( SupportedExt extension );

    void                                                    gotoLastBookMark                            ();
    void                                                    resetDocView                                ();
    void                                                    setMargin                                   ( int topPercentage, int rightPercentage, int bottomPercentage, int leftPercentage );
    void                                                    setTimestamp                                (qint64 timestamp);
    void                                                    prepareViewerBeforeSleep                    ();
    void                                                    calculateTimeForStep                        ();
    void                                                    updateTimeOnPage                            ();

    inline bool                                             isMenuShown                                 () const { return m_viewerMenu->isVisible(); }
    QString                                                 getChapterTitle                             () {return m_chapterTitle;}
    void                                                    calculateLocationsChapter                   ();
    void                                                    deleteCurrentBookInfo                       ();
    void                                                    setMark                                     ( BookLocation* );
    void                                                    repaintContentList                          ();
    void                                                    setInitialDefinitionSearch                  (const QString&, const QString&);
    void                                                    resetInitialDefinitionSearch                ();

    void                                                    forceReloading                              () { delete m_docView; m_docView = NULL; b_reloading = true; }
    void                                                    resetViewerBookInfo                         ();
    int                                                     getUpperMargin                              ();
    bool                                                    isOpeningDoc                                () { return b_opening; }

    bool                                                    isScrollable                                () const;

    // PDF miniature
    void                                                    updatePdfMiniatureScreenshot                ( QPixmap& screenshot );
    void                                                    updatePdfMiniatureLocation                  ( double viewXOffsetPercent, double viewYOffsetPercent, double scale );
    void                                                    enablePdfMode                               ( bool enable = true);

protected:
    QWidget*                                                textBodyInstance                            () { return textBody; }

    virtual void                                            paintEvent                                  ( QPaintEvent* );
    virtual void                                            keyReleaseEvent                             ( QKeyEvent* );
    virtual void                                            mouseMoveEvent                              ( QMouseEvent* );
    virtual void                                            customEvent                                 ( QEvent* );
    void                                                    handleLongPressEnd                          ( TouchEvent* );
    ScreenArea                                              pointedArea                                 ( const QPoint& );
    void                                                    handleLongPressStart                        ( TouchEvent* );
    bool                                                    checkPdfScroll                              ( TouchEvent* );
    bool                                                    offsetPage                                  ( int, int );
    bool                                                    isZoomed                                    ();
    bool                                                    isOtherBook                                 ( const QString& ) const;
    bool                                                    noWidgetShown                               () const;
    bool                                                    hasChangedOptions                           ();
    void                                                    resetScreen                                 ();
    void                                                    setTextBodyGeometry                         ();
    void                                                    setRefreshModeOnTurningPage                 ();
    void                                                    loadDocument                                ();
    void                                                    pushHistory                                 ();
    void                                                    showTimeTitleLabels                         ();
    void                                                    setUpperMargin                              ();
    bool                                                    isPdfBtnAllowed                             ();
    bool                                                    isTopScrollArea                             (QPoint& tapPoint);
    bool                                                    isBottomScrollArea                          (QPoint& tapPoint);
    bool                                                    isLeftScrollArea                            (QPoint& tapPoint);
    bool                                                    isRightScrollArea                           (QPoint& tapPoint);
    bool                                                    checkSecondaryAxisSwipe                     ( TouchEvent* );

    int                                                     i_loadState;
    int                                                     i_hiliId;
    int                                                     i_docExtension;
    int                                                     i_fontType;
    int                                                     i_refreshCounter;
    int                                                     i_maxQuickRefresh;
    int                                                     i_minPxSwipeLenght;
    bool                                                    b_hiliMode;
    bool                                                    b_pdfModeSupported;
    bool                                                    b_reloading;
    bool                                                    updatedViewerBook;
    bool                                                    b_opening;
    bool                                                    b_indexReady;
    bool                                                    currentChapter;
    bool                                                    m_chapterMode;
    bool                                                    m_errorAfterLoading;
    bool                                                    m_shownYet;
    bool                                                    m_showTitle;
    bool                                                    m_showDateTime;

    int                                                     m_loadDocumentError;
    int                                                     m_screenSteps;

    QString                                                 qs_bookMark;
    QString                                                 qs_docPath;
    QString                                                 m_chapterTitle;
    int                                                     m_pageStart;
    int                                                     m_pagesLeft;
    int                                                     m_totalChapterPages;
    QString                                                 qs_mime;
    QStack<QString>                                         history;
    QString                                                 m_errorMsg;
    QStringList                                             m_warningMsgs;

    QDocView*                                               m_docView;
    ViewerMenu*                                             m_viewerMenu;
    ViewerMarkHandler*                                      m_markHandler;

    ViewerSearchPopup*                                      m_searchPopup;
    ViewerAppearancePopup*                                  m_viewerAppearancePopup;
    ViewerGotoPopup*                                        m_gotoPopup;
    ViewerDictionary*                                       m_viewerDictionary;
    ViewerContentsPopup*                                    m_bookIndex;
    ViewerBookSummary*                                      m_bookSummary;
    MouseFilter*                                            m_pMouseFilter;
    PowerManagerLock*                                       m_powerLock;
    int                                                     timePerView;
    QDateTime                                               timeGone;
    QDateTime                                               m_timeOnPage;
    int                                                     currentView;
    double                                                  initialStepPos;
    double                                                  finalStepPos;
    double                                                  mSecsNeedToReadStep;
    qint64                                                  initialStepTimestamp;
    qint64                                                  finalStepTimestamp;

signals:
    void                                                    loadDocError                                ();
    void                                                    openingFinished                             ();
    void                                                    upperMarginUpdate                           (int);

    void                                                    zoomOut                                     ();
    void                                                    zoomIn                                      ();

public slots:
    void                                                    modelChanged                                ( QString, int );
    void                                                    openDoc                                     ( const BookInfo* );
    void                                                    hideAllElements                             ();
    void                                                    showElement                                 ( QWidget* );
    void                                                    gotoSearchWordPage                          ( const QString& );
    void                                                    goToMarkup                                  ( const QString& );
    void                                                    setPageChanged                              (int, int, int);
    void                                                    updateBookInfo                              ( bool writeToFile = true );
    void                                                    showContents                                ();
    void                                                    reloadCurrentBook                           ();
    void                                                    setCurrentChapterInfo                       ();
    void                                                    showBookmark                                ( bool );
    void                                                    checkAndExtractCover                        ();
    void                                                    resetViewerWidgets                          ();

protected slots:
    void                                                    handleLoadState                             (int);
    void                                                    handleFontBtnPress                          ();
    void                                                    handleGoToPagePress                         ();
    void                                                    handleSearchReq                             ();
    void                                                    handleSummaryReq                            ();
    void                                                    goPageBack                                  ();
    void                                                    pdfMenuBtnClicked                           ();
    void                                                    handleLandscapeMode                         ();
    void                                                    closePdfToolsWindow                         ();
    void                                                    screenAdjust                                ();
    void                                                    widthAdjust                                 ();
    void                                                    heightAdjust                                ();
    void                                                    goToPage                                    (int);
    void                                                    goToMark                                    (const QString& ref);

    void                                                    performSearchRequest                        ( const QString& text );

    void                                                    numPageMarks                                ( int, int );
    void                                                    handleFatal                                 ();
    void                                                    handleError                                 ( QString, int );
    void                                                    handleWarnings                              ( QStringList warnings );
    void                                                    handleScrollArrows                          ( QList<bool> );
    void                                                    handleFatalAfterLoading                     ();
    void                                                    handleErrorAfterLoading                     ( QString, int );
    void                                                    handleWarningsAfterLoading                  ( QStringList warnings );
    void                                                    removeBook                                  ( BookInfo* );
    //void                                                    showBookmark                                ( bool );
    void                                                    processPressEvent                           ( TouchEvent*, bool eventFromViewerPageHandler = false);
    void                                                    menuPopUpShow                               ( ViewerMenuPopUp* popup, bool showStatusBar = true );
    void                                                    menuPopUpHide                               ();

    void                                                    updateTime                                  ();

#ifndef HACKERS_EDITION
    void                                                    buyBook();
#endif
    void                                                    resumeWifi();
    void                                                    handleBookmark();
    void                                                    disconnectWifi();
    void                                                    registerStepRead();
    void                                                    clearStepsProcess();
    void                                                    handleViewerPageHandlerPressEvent           ( TouchEvent* );

private:
    QWidget*                                                m_currentWidget;
    ViewerMenuPopUp*                                        m_currentMenuPopup;
    BookInfo*                                               m_bookInfo;
    ViewerStepsManager*                                     m_viewerStepsManager;
    QString                                                 setChapterForLocation                       ( double );
    QTimer                                                  m_updateTimeTimer;
    QList<bool>                                             scrollArrows;
};

#endif // VIEWER_H
