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

#ifndef QBOOKWEBVIEW_H
#define QBOOKWEBVIEW_H

#include <QWebPage>
#include "QBookForm.h"
#include "PowerManagerLock.h"
#include <QPoint>
#include <QPointer>
#include <QSslError>
#include <QNetworkReply>
#include <QFile>
#include <QTimer>
#include <QElapsedTimer>
#include "ui_Browser.h"

class QAdobeDrm;

class QWebView;
class QLineEdit;
class Keyboard;
class BookInfo;
class QBookApp;
class InfoDialog;
class ConfirmDialog;
class BrowserInputContext;
class BrowserFavo;
class BrowserNetworkAccessManager;
class QWebPage;
class FullScreenWidget;
class BrowserAdobeDrm;
class ProgressDialog;

class Browser : public QBookForm, protected Ui::Browser
{
    Q_OBJECT

public:
    Browser(QWidget *parent = 0);
    virtual ~Browser();
    virtual void activateForm();
    virtual void deactivateForm();
    QWebHistory * history;

    virtual bool eventFilter(QObject*, QEvent*);
    bool b_blocked;
    BookInfo* _book;
    void loadURL(QString);


public slots:

    void                                            processUrl                              ( );
    void                                            showKeyboard                            ( );
    void                                            download                                ( const QUrl& );
    void                                            doWheel                                 ( bool, int );
    void                                            showfavo                                ( );

    void                                            addtofavo                               ( );
    void                                            setSimpleView                           ( );
    void                                            forceUrl                                ( const QString& );
    void                                            activateFormWithConnection              ( );
    void                                            showElement                             ( QWidget* );
    void                                            hideElement                             ( );

protected slots:
    void                                            goBack                                  ( );
    void                                            clearText                               ( );
    void                                            updateStatus                            ( );
    void                                            adjustTitle                             ( );
    void                                            setProgress                             ( int p );
    void                                            errorListener                           ( QNetworkReply*, const QList<QSslError>& );
    void                                            unsupportedContentProcess               ( QNetworkReply* );
    void                                            getFile                                 ( const QString& fileName );
    void                                            handleVScrollBarValueChanged            ( int nNewScrollValue );
    void                                            handleHScrollBarValueChanged            ( int nNewScrollValue );
    void                                            handleLoadStart                         ( );
    void                                            updateBars                              ( QSize sWebViewSize );
    void                                            handleZoomIn                            ( );
    void                                            handleZoomOut                           ( );
    void                                            updateZoomBtnStates                     ( );
    void                                            handleViewPressPos                      ( QPoint pt );
    void                                            handleViewReleasePos                    ( );
    void                                            showKeyBoardDialog                      ( );
    void                                            sendKey                                 ( const QString& );
    void                                            sendBackSpace                           ( );
    void                                            sendCursorLeft                          ( );
    void                                            sendCursorRight                         ( );
    void                                            sendReturn                              ( );
    void                                            closeKeyBoardDialog                     ( );
    void                                            handleURLChanged                        ( const QUrl & url );

    void                                            handleScrollUpBtnPress                  ( );
    void                                            handleScrollDownBtnPress                ( );
    void                                            handleScrollLeftBtnPress                ( );
    void                                            handleScrollRightBtnPress               ( );
    void                                            handleHomeBtnPress                      ( );
    void                                            handleMyfavoPress                       ( QString clickurl );
    void                                            setTextCursor                           ( bool ok = false );
    void                                            checkConnectionToExit                   ( );
    void                                            stopDownload                            ( );
    void                                            downloadCanceled                        ( );
    void                                            downloadComplete                        ( );
    void                                            readData                                ( );
    void                                            handleDownloadProgress                  (qint64 bytesReceived, qint64 bytesTotal);
    void                                            handleDownLoadError                     (QNetworkReply::NetworkError code);

private:

    int                                             mScrollMoveTimes;
    int                                             progress;
    int                                             mVPageStep;
    int                                             mVMax;
    int                                             mHPageStep;
    int                                             mHMax;

    bool                                            mDisableBigBarMode;
    bool                                            bContentDisposition;
    bool                                            b_alreadyVisitedBrowser;
    bool                                            mSelectWebEdit;
    bool                                            mHShow;
    bool                                            mVShow;

    QString                                         primitiveUrl;
    QString                                         cd_fileName;
    QString                                         mDefaultURL;
    QString                                         mCurrectURL;
    QString                                         mHomeURL;
    QString                                         s_forced;
    QString                                         path;

    qreal                                           mScale;
    QFile                                           m_file;
    QUrl                                            m_url;

    QPoint                                          mLTViewPos;
    QPoint                                          mPrevScrollPos;
    QPoint                                          mNewScrollPos;
    QPoint                                          m_selectedElementPos;
    ProgressDialog*                                 m_downloadDialog;
    QNetworkReply*                                  replyNew;
    PowerManagerLock*                               m_powerLock;
    BrowserAdobeDrm*                                browserDrm;

    Keyboard*                                       newKeyboard;
    BrowserFavo*                                    m_favo;
    QPointer<InfoDialog>                            m_waitingDialog;
    InfoDialog*                                     m_infoDialogAccessFailed;
    InfoDialog*                                     m_connectionDialog;
    QTimer*                                         progressTimer;
    BrowserNetworkAccessManager*                    m_namgr;
    BrowserInputContext*                            mInputContext;
    QWebPage                                        m_page;
    QElapsedTimer                                   timer;
    qint64                                          bytesToDownload;
    qint64                                          bytesDownloaded;

    void                                            setURLHint                              ( const QString& );
    void                                            populateBookInfo                        ( BookInfo* );
    void                                            resetScrolls                            ( );
    void                                            adjustScrollItemPos                     ( );
    void                                            adjustHorizontalScrollItemPos           ( );
    void                                            adjustVerticalScrollItemPos             ( );
    void                                            updateButtonsState                      ( );
    void                                            initialWebViewBars                      ( );
    void                                            initBrowserParams                       ( );
    void                                            processDownload                         ( );

protected:
    virtual void                                    keyReleaseEvent                         ( QKeyEvent* event );

signals:
    void                                            closeDialog                             ( );
    void                                            openBook                                ( const BookInfo* );
    void                                            downloadProgress                        ( int );


    friend class QBookApp;
};

#endif // QBOOKWEBVIEW_H

