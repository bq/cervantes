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

#ifndef WEBSTORE_H
#define WEBSTORE_H

#include <QWebPage>
#include "ui_Browser.h"

#include "QBookForm.h"
#include "PowerManagerLock.h"

#include <QPoint>
#include <QSslError>
#include <QNetworkReply>
#include <QTimer>

class Keyboard;
class BookInfo;
class QBookApp;
class bqDeviceServices;
class ProgressDialog;
class InfoDialog;
class BrowserInputContext;
class BrowserNetworkAccessManager;
class QWebView;
class QWebPage;

class WebStore : public QBookForm, protected Ui::Browser
{
    Q_OBJECT

public:
    WebStore(QWidget *parent);
    virtual ~WebStore();
    virtual void                                        activateForm                            ();
    virtual void                                        deactivateForm                          ();
    virtual bool                                        eventFilter                             ( QObject*, QEvent* );
    void                                                forceUrl                                ( const QString& );
    void                                                resetVisitedStore                       ();

protected slots:

    void                                                setProgress                             ( int );
    void                                                errorListener                           ( QNetworkReply*,const QList<QSslError>& );
    void                                                processHeaders                          ( QNetworkReply* );
    void                                                handleURLChanged                        ( const QUrl& );
    void                                                handleViewPressPos                      ( QPoint );
    void                                                loadFinished                            ( bool );
    void                                                handleLoadStart                         ( );
    void                                                handleViewReleasePos                    ( );
    void                                                showKeyBoardDialog                      ( );
    void                                                sendKey                                 ( const QString& );
    void                                                sendBackSpace                           ( );
    void                                                sendCursorLeft                          ( );
    void                                                sendCursorRight                         ( );
    void                                                sendReturn                              ( );
    void                                                hideKeyboard                            ( );
    void                                                checkConnectionToExit                   ( );
    void                                                activateFormWithConnection              ( );
    void                                                setTextCursor                           ( int position = 0 );
    void                                                connectionCanceled                      ( );
    void                                                connectWifiObserverNavigateToUrl        ( );
    void                                                disconnectWifiObserverNavigateToUrl     ( );
    void                                                loadURL                                 ( QString = "");
private:

    int                                                 dirty;
    int                                                 progress;

    bool                                                b_alreadyVisitedStore;
    bool                                                mSelectWebEdit;
    bool                                                b_blocked;
    bool                                                m_wifiConnectionCanceled;
    QPoint                                              m_selectedElementPos;
    QString                                             mCurrectURL;
    QString                                             s_forced;

    BrowserNetworkAccessManager*                        m_namgr;
    BrowserInputContext*                                mInputContext;
    Keyboard*                                           newKeyboard;
    PowerManagerLock*                                   m_powerLock;
    bqDeviceServices*                                   services;
    InfoDialog*                                         m_infoDialogAccessFailed;
    InfoDialog*                                         m_connectionDialog;
    ProgressDialog*                                     pDialog;
    QTimer*                                             progressTimer;

protected:
    virtual void                                        keyReleaseEvent                                 ( QKeyEvent* );
    virtual void                                        paintEvent                                      ( QPaintEvent* );
    bool                                                hasPurchaseHeaders                              ( QNetworkReply* );
    void                                                processPurchaseHeaders                          ( QNetworkReply* );
    bool                                                hasDoReloginHeader                              ( QNetworkReply* );
    void                                                processDoReloginHeader                          ( );
    void                                                createProgressDialog                            ( );
    void                                                destroyDialog                                   ( );
    void                                                goBack                                          ( );
    bool                                                hasOpenBookHeader                               ( QNetworkReply* reply );
    void                                                processOpenBookHeader                           ( QNetworkReply* reply ); 

signals:
    void                                                openBook                                        ( const BookInfo* );
    void                                                addedBook                                       ( const QString& );

    friend class QBookApp;
};

#endif // QBOOKWEBVIEW_H

