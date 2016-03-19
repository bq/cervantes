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

#ifndef TIMEZONEITEM_H
#define TIMEZONEITEM_H

#include "ui_SettingsDateTimeMenuItem.h"
#include "GestureWidget.h"

class TimeZone {

public:
        QString description;
        QString timeZone;
        QString code;
        bool defaultTimeZone;

        TimeZone(QString _description, QString _timeZone, QString _code){
            description = _description;
            timeZone = _timeZone;
            code = _code;
            defaultTimeZone = false;
        }
};

class GestureWidget;

class SettingsDateTimeMenuItem : public GestureWidget, protected Ui::SettingsDateTimeMenuItem
{
    Q_OBJECT

public:

                        SettingsDateTimeMenuItem                    (QWidget*);
                        ~SettingsDateTimeMenuItem                   ();
        void            paint                                       (TimeZone*);

private:
        TimeZone*       timeZone;

public slots:
        void            handleTap                                   ();

protected:

        /* http://qt-project.org/forums/viewthread/7340 */
        void            paintEvent                              (QPaintEvent *);

signals:

        void            setTimeZone                             (TimeZone*);
};

#endif // TIMEZONEITEM_H
