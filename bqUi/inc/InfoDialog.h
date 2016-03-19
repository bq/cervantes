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

#ifndef InfoDialog_H
#define InfoDialog_H

#include "ui_InfoDialog.h"

#include <QDialog>
#include <QBasicTimer>

class QString;
class QTimerEvent;
class PowerManagerLock;
class QMouseEvent;
class QKeyEvent;


class InfoDialog : public QDialog, protected Ui::InfoDialog
{
    Q_OBJECT

public:
    InfoDialog(QWidget * parent, QString textStr = "", long lastMilis = InfoDialog::_INFO_DIALOG_DEFAULT_WAIT);
    virtual ~InfoDialog();
    void                                setText                             ( const QString& );
    void                                hideSpinner                         ();
    void                                setTimeLasts                        (long);
    void                                showForSpecifiedTime                ();
    void                                setHorizontal                       () { m_horizontal = true; }

protected:
    virtual void                        timerEvent                          (QTimerEvent*);
    QBasicTimer                         m_timer;
    long                                l_timeMilis;
    static const long _INFO_DIALOG_DEFAULT_WAIT = 5000;
    PowerManagerLock*                   m_powerLock;
    bool                                m_horizontal;

    /* http://qt-project.org/forums/viewthread/7340 */
    void                                paintEvent                          (QPaintEvent *);
    virtual void                        mousePressEvent                     (QMouseEvent* );
    virtual void                        mouseReleaseEvent                   (QMouseEvent* );
    virtual void                        keyPressEvent                       (QKeyEvent* );
    virtual void                        keyReleaseEvent                     (QKeyEvent* );

protected slots:
    void                                        hideDialog                  ();
    void                                        rejectDialog                ();

};

#endif // InfoDialog_H
