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

#ifndef ConfirmDialog_H
#define ConfirmDialog_H

#include "ui_ConfirmDialog.h"
#include "Screen.h"


#include <QDialog>
#include <QBasicTimer>

class PowerManagerLock;

class ConfirmDialog : public QDialog, protected Ui::ConfirmDialog
{
    Q_OBJECT

public:
    ConfirmDialog(QWidget* parent, QString textStr = "", QString buttonTxt = tr("Aceptar"),
                  Screen::ScreenMode mode = Screen::MODE_SAFE);
    virtual ~ConfirmDialog();
    void setText(QString);
    void setButtonText(QString);
    SilentQPushButton* getButton() {return ok;}
    void setTimeLasts(long);
    void showForSpecifiedTime(long);


private slots:
    void doAccept();
    void hideAndShutDown();

private:
    Screen::ScreenMode m_refreshMode;

protected:
    virtual void timerEvent(QTimerEvent*);
    QBasicTimer m_timer;
    long l_timeMilis;
    PowerManagerLock* m_powerLock;

    /* http://qt-project.org/forums/viewthread/7340 */
    void                                paintEvent                          (QPaintEvent *);
};

#endif // ConfirmDialog_H
