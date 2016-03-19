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

#ifndef SETTINGSREADERPROGRESSBAR_H
#define SETTINGSREADERPROGRESSBAR_H

#include "FullScreenWidget.h"
#include "ui_SettingsReaderProgressBar.h"

class SettingsReaderProgressBar : public FullScreenWidget , protected Ui::SettingsReaderProgressBar
{
    Q_OBJECT

public:

                    SettingsReaderProgressBar                          (QWidget *);
    virtual         ~SettingsReaderProgressBar                         ();
    void            setWithChapterLinesBtn                             ();


protected:

    void            setWithProgressBtn                          ();
    void            setWithPageBtn                              ();
    void            setWithChapterBtn                           ();
    void            setWithProgressBarBtn                       ();
    void            setWithBookTitleBtn                         ();
    void            setWithDateTimeBtn                          ();
    void            showChapterLines                            ();
    void            hideChapterLines                            ();
    int             m_chapterLineWidth;

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void    paintEvent                                  (QPaintEvent *);

private slots:

    void            withPageBtnHandler                          ();
    void            withProgressBtnHandler                      ();
    void            withChapterBtnHandler                       ();
    void            withProgressBarHandler                      ();
    void            withBookTitleHandler                        ();
    void            withDateTimHandler                          ();
    void            withChapterLinesHandler                     ();

};

#endif // SETTINGSREADERPROGRESSBAR_H
