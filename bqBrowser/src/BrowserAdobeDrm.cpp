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

#include "BrowserAdobeDrm.h"

#include "InfoDialog.h"
#include "QBookApp.h"
#include "Model.h"
#include "AdobeDRM.h"
#include "dp_doc.h"
#include <QFile>

BrowserAdobeDrm::BrowserAdobeDrm(QWidget *) : bReturnable(false)
{}

BrowserAdobeDrm::~BrowserAdobeDrm(){}

void BrowserAdobeDrm::processFulFillment(QFile& file)
{
    qDebug() << Q_FUNC_INFO << file.fileName();
    if(!file.open(QFile::ReadOnly))
        return;

    m_processDialog = new InfoDialog(this, tr("Your book is being downloaded, when finished it will be available on the book list!"));
    m_processDialog->show();
    m_AdeptErrorStr.clear();


    AdobeDRM* drm_hdlr = AdobeDRM::getInstance();
    connect(drm_hdlr, SIGNAL(fulfillmentOK(QString,bool,QString)),  this, SLOT(handleFulfillmentDone(QString,bool,QString)));
    connect(drm_hdlr, SIGNAL(fulfillmentKO(QString)),               this, SLOT(handleFulfillmentError(QString)));

    drm_hdlr->fulfillment(file);

    file.close();
    file.remove();
}

void BrowserAdobeDrm::handleFulfillmentDone(QString fulfillmentId, bool bReturnable, QString fulfillmentDocPath)
{
    qDebug() << Q_FUNC_INFO;

    AdobeDRM* drm_hdlr = AdobeDRM::getInstance();
    disconnect(drm_hdlr, SIGNAL(fulfillmentOK(QString,bool,QString)),   this, SLOT(handleFulfillmentDone(QString,bool,QString)));
    disconnect(drm_hdlr, SIGNAL(fulfillmentKO(QString)),                this, SLOT(handleFulfillmentError(QString)));

    if(m_processDialog)
    {
        delete m_processDialog;
        m_processDialog = NULL;
    }

    if(m_AdeptErrorStr.isEmpty())
    {
        QBookApp::instance()->getModel()->addDir(qgetenv("ADOBE_DE_DOC_FOLDER"));
        InfoDialog *dialog = new InfoDialog(this,tr("The book is available on your library!"));
        dialog->showForSpecifiedTime();
        delete dialog;
    }
    else
    {
        QFile file(fulfillmentDocPath);
        qDebug() << Q_FUNC_INFO << "delete downloaded adept book" << file.remove();
    }
}

void BrowserAdobeDrm::handleFulfillmentError( QString errorMsg )
{
    qDebug() << Q_FUNC_INFO << errorMsg;

    AdobeDRM* drm_hdlr = AdobeDRM::getInstance();
    disconnect(drm_hdlr, SIGNAL(fulfillmentOK(QString,bool,QString)),   this, SLOT(handleFulfillmentDone(QString,bool,QString)));
    disconnect(drm_hdlr, SIGNAL(fulfillmentKO(QString)),                this, SLOT(handleFulfillmentError(QString)));

    if(m_processDialog)
    {
        delete m_processDialog;
        m_processDialog = NULL;
    }

    if(!m_AdeptErrorStr.isEmpty())
        return;

    m_AdeptErrorStr = errorMsg;

    InfoDialog* dialog = new InfoDialog(this, tr("Error processing credentials"));
    dialog->showForSpecifiedTime();
    delete dialog;
}

