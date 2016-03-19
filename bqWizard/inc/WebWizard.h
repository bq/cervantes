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

#ifndef WEBWIZARD_H
#define WEBWIZARD_H

#include <QWebPage>
#include <QPoint>
#include <QSslError>
#include <QNetworkReply>
#include <QTimer>

#include "QBookForm.h"
#include "PowerManagerLock.h"
#include "ui_Browser.h"

class QWebView;
class QWebPage;
class Keyboard;
class QBookApp;
class InfoDialog;
class BrowserWebPage;
class BrowserInputContext;
class BrowserNetworkAccessManager;

class WebWizard : public QBookForm, protected Ui::Browser
{
    Q_OBJECT

public:
    WebWizard(QWidget *parent = 0);
    virtual ~WebWizard();
    virtual void                            activateForm                        ( );
    virtual void                            deactivateForm                      ( );
    void                                    setWebPageEmail                     ( QString );
    void                                    forceUrl                            ( const QString& );

protected slots:
    void                                    setProgress                         ( int );
    void                                    errorListener                       ( QNetworkReply*, const QList<QSslError>&);
    void                                    handleURLChanged                    ( const QUrl& url );
    void                                    processHeaders                      ( QNetworkReply* );
    void                                    handleViewPressPos                  ( QPoint );
    void                                    handleLoadStart                     ( );
    void                                    handleViewReleasePos                ( );
    void                                    showKeyBoardDialog                  ( );
    void                                    sendKey                             ( const QString& );
    void                                    sendBackSpace                       ( );
    void                                    sendCursorLeft                      ( );
    void                                    sendCursorRight                     ( );
    void                                    sendReturn                          ( );
    void                                    closeKeyBoardDialog                 ( );
    void                                    checkConnectionToExit               ( );
    void                                    loadFinished                        ( bool );
    void                                    setTextCursor                       ( int position = 0 );

private:

    int                                     dirty;
    int                                     progress;
    bool                                    mSelectWebEdit;
    QString                                 mCurrectURL;
    QString                                 s_forced;
    QPoint                                  m_selectedElementPos;
    BrowserNetworkAccessManager*            m_namgr;
    BrowserWebPage*                         pPage;
    BrowserInputContext*                    mInputContext;
    PowerManagerLock*                       m_powerLock;
    InfoDialog*                             m_connectionDialog;
    Keyboard*                               newKeyboard;
    QTimer*                                 progressTimer;

protected:
    void                                    keyReleaseEvent                         ( QKeyEvent* );
    virtual void                            paintEvent                              ( QPaintEvent* );
    bool                                    hasLandingHeaders                       ( QNetworkReply* );
    void                                    processLandingHeaders                   ( QNetworkReply* );
    void                                    checkLandingErrors                      ( QNetworkReply* );
    void                                    loadURL                                 ( const QString& );
    void                                    hideKeyboard                            ( );
    void                                    activateFormWithConnection              ( );
    void                                    goBack                                  ( );

signals:
    void                                    registerOK                              ( );
    void                                    welcomePageLoaded                       ( );
    void                                    goToWifi                                ( );
    void                                    connectionProblem                       ( );
    void                                    processHeadersFailed                    ( );
    void                                    serialInvalid                           ( );

    friend class QBookApp;
};

#endif // QBOOKWEBVIEW_H

