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

#ifndef SelectionDialog_H
#define SelectionDialog_H

#include "ui_SelectionDialog.h"

#include <QDialog>

class SelectionDialog : public QDialog, protected Ui::SelectionDialog
{
    Q_OBJECT

public:
    SelectionDialog(QWidget * parent,
                    QString textStr = "",
                    QString okBtnTxt = tr("Aceptar"),
                    QString cancelBtnText = tr("Cancelar"));

    void setText(QString);
    void setOkBtnText(QString);
    void setCancelBtnText(QString);
    void addWidget(QWidget*);
    QWidget *getWidget();


private slots:

    void doReject();
    void doAccept();
    void hideAndShutDown();

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    void                                paintEvent                          (QPaintEvent *);

signals:
    void accepted();
    void rejected();

private:
    QWidget *widget;

};

#endif // SelectionDialog_H
