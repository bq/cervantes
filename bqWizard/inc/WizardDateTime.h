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

#ifndef WIZARDDATETIME_H
#define WIZARDDATETIME_H

#include "ui_WizardDateTime.h"
#include "WizardPagerCont.h"
#include "WizardDateTimeItem.h"
#include <QBasicTimer>
#include "FullScreenWidget.h"

/**
  * Se ha implementado un sistema semi-automatico para poder añadir nuevas zonas horarias.
  * En la UI del desplegable se definen los botones de manera que el nombre de ellos sea Region__ciudad (p.e.: Africa__Ceuta)
  * de manera que coincida con el formato de la zona horaria soportado por el SO (/usr/share/zoneinfo o tzselect).
  * La variable de entorno TZ (zona horaria) debe establecerse al inicio de la aplicación (main.cpp) para que la hora se muestre siempre correcta.
  * Si se cambia la zona horaria, con reestablecer esta variable es suficiente.
  */

/**
  * It was implemented a semi-automatic system to allow new time zones.
  * At the
  */
class WizardDateTime : public FullScreenWidget , protected Ui::WizardDateTime
{
    Q_OBJECT

public:

                                        WizardDateTime(QWidget*);
    virtual                             ~WizardDateTime();
    static void                         setTimeZoneEnvironment();

private:

    QList<TimeZone*>                    zones;
    QList<WizardDateTimeItem*>          items;
    int                                 page;
    void                                setupTimeZone                       ();
    QBasicTimer                         m_timer;

protected slots:

    void                                changeTimeZone                      (TimeZone*);
    void                                paintTimeZones                               ();
    int                                 getTotalPages                       ();
    void                                back                                ();
    void                                forward                             ();


protected:

    QString                             m_currentTimeZone;
    void                                getCurrentTimeZone                  ();
    void                                setNewTimeZone                      (QString, int, int);
    static QString                      getTimeZoneStr                      (int timeZone = -1, int timeCity = -1);
    static QString                      getTimeCityStr                      ();

    /* http://qt-project.org/forums/viewthread/7340 */
    void                                paintEvent                          (QPaintEvent *);

};

#endif // WIZARDDATETIME_H
