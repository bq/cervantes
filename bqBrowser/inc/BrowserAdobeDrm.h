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

#ifndef BROWSERADOBEDRM_H
#define BROWSERADOBEDRM_H

#include "PowerManagerLock.h"

#include <QWidget>
#include <QDebug>
#include <QFile>
class QBookApp;
class InfoDialog;

class BrowserAdobeDrm : public QWidget
{
    Q_OBJECT

public:
    BrowserAdobeDrm                                 (QWidget *parent = 0);
    virtual ~BrowserAdobeDrm();

    void                                            processFulFillment                      (QFile& file);

protected slots:
    void                                            handleFulfillmentDone                   ( QString fulfillmentId, bool bReturnable, QString fulfillmentDocPath );
    void                                            handleFulfillmentError                  ( QString errorMsg );

private:

    bool                                            bReturnable;
    QString                                         fulfillmentId;
    QString                                         fulfillmentDocPath;
    QString                                         m_AdeptErrorStr;

    PowerManagerLock*                               m_powerLock;
    InfoDialog*                                     m_processDialog;
};

#endif // BROWSERADOBEDRM_H


