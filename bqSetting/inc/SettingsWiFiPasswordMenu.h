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

#ifndef ELFSECURITYDIALOG_H
#define ELFSECURITYDIALOG_H

#include "ui_SettingsWiFiPasswordMenu.h"

class QWidget;
class QKeyEvent;

class SettingsWiFiPasswordMenu : public QWidget, protected Ui::SettingsWiFiPasswordMenu{

    Q_OBJECT

public:
                            SettingsWiFiPasswordMenu            (QWidget *parent = 0, QString = "", QString = "");
    virtual                 ~SettingsWiFiPasswordMenu           ();

private:

    QString                 password;
    void                    hideMe                              ();

private slots:

   void                     setKeyboard                         ();
   void                     okClicked                           ();
   void                     cancelClicked                       ();
   void                     hiddenPasswordHandler               (bool);
signals:

   void                     ok                                  (QString);
   void                     canceled                            ();

protected:

   /* http://qt-project.org/forums/viewthread/7340 */
   void                     paintEvent                          (QPaintEvent *);

   virtual void             keyPressEvent                       ( QKeyEvent * /*event*/ );
   virtual void             keyReleaseEvent                     ( QKeyEvent * /*event*/ );

};
#endif // ELFSECURITYDIALOG_H
