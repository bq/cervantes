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

#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include "ui_ProgressDialog.h"

#include <QDialog>

class ProgressDialog : public QDialog, protected Ui::ProgressDialog
{
    Q_OBJECT

public:
    ProgressDialog(QWidget * parent, QString textStr = "");
    virtual ~ProgressDialog();
    void setText(QString);
    void setTextValue(bool);
    void hideCancelButton();
    void setHideBtn(bool);

public slots:
    void setProgressBar(int);
    void setTotalProgressBar(int);
    void hideProgressBar();
    void showProgressBar();

signals:
    void cancel();
    void hideDialog();

protected:
    /* http://qt-project.org/forums/viewthread/7340 */
    void                                paintEvent                          ( QPaintEvent* );
    virtual void                        keyPressEvent                       ( QKeyEvent* );
    virtual void                        keyReleaseEvent                     ( QKeyEvent* );
    virtual void                        mousePressEvent                     ( QMouseEvent* );
    virtual void                        mouseReleaseEvent                   ( QMouseEvent* );
};
#endif // PROGRESSDIALOG_H
