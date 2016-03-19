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

#include "WizardDateTime.h"
#include "QBook.h"
#include "QBookApp.h"
#include "TimeZones.h"
#include <QPainter>
#include "Screen.h"
#include <QPoint>
#include <QDebug>
#include <QKeyEvent>
#include <QStringList>
#include "InfoDialog.h"


#define UPDATE_TIME_INTERVAL 10000

WizardDateTime::WizardDateTime(QWidget *parent) : FullScreenWidget(parent)
{
        setupUi(this);

        zones.append(new TimeZone(tr("Spain/Madrid"), "GMT + 1:00", "Europe__Madrid"));
        zones.append(new TimeZone(tr("Spain/Canary Islands"), "GMT + 0:00", "Atlantic__Canary"));
        zones.append(new TimeZone(tr("Portugal/Lisbon"), "GMT + 0:00", "Europe__Lisbon"));
        zones.append(new TimeZone(tr("Chile/Santiago"), "GMT - 4:00", "Chile__Continental"));
        zones.append(new TimeZone(tr("Chile/Easter Island"), "GMT - 6:00", "Chile__EasterIsland"));
        zones.append(new TimeZone(tr("Mexico/Mexico DF"), "GMT -6:00", "America__Mexico_City"));
        zones.append(new TimeZone(tr("Mexico/Chihuahua"), "GMT -7:00", "America__Chihuahua"));
        zones.append(new TimeZone(tr("Mexico/Tijuana"), "GMT -8:00", "America__Tijuana"));

        zones.append(new TimeZone(tr("France/Paris"), "GMT + 1:00", "Europe__Paris"));
        zones.append(new TimeZone(tr("Germany/Berlin"), "GMT + 1:00", "Europe__Berlin"));
        zones.append(new TimeZone(tr("UK/London"), "GMT + 0:00", "Europe__London"));
        zones.append(new TimeZone(tr("Argentina/Buenos Aires"), "GMT - 3:00", "America__Argentina/Buenos_Aires"));
        zones.append(new TimeZone(tr("Brazil/Noronha"), "GMT -2:00", "America__Noronha"));
        zones.append(new TimeZone(tr("Brazil/Sao Paulo"), "GMT -3:00", "America__Sao_Paulo"));
        zones.append(new TimeZone(tr("Brazil/Porto Velho"), "GMT -4:00", "America__Porto_Velho"));

        items.append(Item1);
        items.append(Item2);
        items.append(Item3);
        items.append(Item4);
        items.append(Item5);

        connect(Item1, SIGNAL(setTimeZone(TimeZone*)), this, SLOT(changeTimeZone(TimeZone*)));
        connect(Item2, SIGNAL(setTimeZone(TimeZone*)), this, SLOT(changeTimeZone(TimeZone*)));
        connect(Item3, SIGNAL(setTimeZone(TimeZone*)), this, SLOT(changeTimeZone(TimeZone*)));
        connect(Item4, SIGNAL(setTimeZone(TimeZone*)), this, SLOT(changeTimeZone(TimeZone*)));
        connect(Item5, SIGNAL(setTimeZone(TimeZone*)), this, SLOT(changeTimeZone(TimeZone*)));


        connect(wizardPagerCont,SIGNAL(previousPageReq()),this,SLOT(back()));
        connect(wizardPagerCont,SIGNAL(nextPageReq()),this,SLOT(forward()));


        setupTimeZone();

        m_timer.start(UPDATE_TIME_INTERVAL, this);
}

/*virtual*/ WizardDateTime::~WizardDateTime()
{

}

int WizardDateTime::getTotalPages(){
        if(zones.size() == 0)
            return 1;
        if(zones.size() % items.size() == 0)
            return zones.size() / items.size();
        else
            return (zones.size() / items.size()) + 1;
}

void WizardDateTime::setupTimeZone(){

        page = 0;
        wizardPagerCont->setup(getTotalPages());

        paintTimeZones();
}

void WizardDateTime::forward (){
        qDebug() << "--->" << Q_FUNC_INFO;
        page++;
        paintTimeZones();

}

void WizardDateTime::back(){
        qDebug() << "--->" << Q_FUNC_INFO;
        page--;

        if(page < 0)
            page = 0;

        paintTimeZones();
}

void WizardDateTime::paintTimeZones(){
        qDebug() << "--->" << Q_FUNC_INFO;

        const int librarySize = zones.size();
        const int pageOffset = page*items.size();
        const int size = items.size()-1;
        for(int i=0; i <= size; ++i){
            WizardDateTimeItem* item = items.at(i);
            int pos = pageOffset + i;
            if(pos < librarySize){
                item->paint(zones.at(pos));
                item->show();
            }else{
                item->hideLbl();
            }
        }
}

/* static */ void WizardDateTime::setTimeZoneEnvironment()
{
    QString timezone = getTimeZoneStr();
    timezone.replace("__", "/");
    setenv("TZ", timezone.toAscii().constData() , 1);
    qDebug() << Q_FUNC_INFO << ": setting timezone " << timezone;

}
/** The timezone selected send that format (zone__city) or (zone__subzone__city).
  if subzone is sended, zone is not printed
*/
void WizardDateTime::changeTimeZone(TimeZone* timeZone)
{
    qDebug() << Q_FUNC_INFO << ": selected " << timeZone->code;

    QStringList list = timeZone->code.split("__");
    QString regionStr;
    QString cityStr;
    int i = 0;
    while (list.size() > i)
    {
        if (i == 0)
            regionStr = list[i];
        else if (i == 1)
            cityStr = list[i];
        else
            cityStr += "/" + list[i];
        ++i;
    }

    int regionInt = timeZoneRegionStrToInt(regionStr);
    int cityInt = -1;
    switch(regionInt)
    {
    case AFRICA:
            cityInt = timeZoneAfricaCityStrToInt(cityStr);
            break;
    case AMERICA:
            cityInt = timeZoneAmericaCityStrToInt(cityStr);
            break;
    case ANTARCTICA:
            cityInt = timeZoneAntarcticaCityStrToInt(cityStr);
            break;
    case ARCTIC:
            cityInt = timeZoneArcticCityStrToInt(cityStr);
            break;
    case ASIA:
            cityInt = timeZoneAsiaCityStrToInt(cityStr);
            break;
    case ATLANTIC:
            cityInt = timeZoneAtlanticCityStrToInt(cityStr);
            break;
    case AUSTRALIA:
            cityInt = timeZoneAustraliaCityStrToInt(cityStr);
            break;
    case BRAZIL:
            cityInt = timeZoneBrazilCityStrToInt(cityStr);
            break;
    case CANADA:
            cityInt = timeZoneCanadaCityStrToInt(cityStr);
            break;
    case CHILE:
            cityInt = timeZoneChileCityStrToInt(cityStr);
            break;
    case EUROPE:
            cityInt = timeZoneEuropeCityStrToInt(cityStr);
            break;
    case INDIAN:
            cityInt = timeZoneIndianCityStrToInt(cityStr);
            break;
    case PACIFIC:
            cityInt = timeZonePacificCityStrToInt(cityStr);
            break;
    }

    setNewTimeZone(timeZone->code, regionInt, cityInt);

    setupTimeZone();
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();

    emit hideMe();
}

/**
  The timezone selected has to be the format (zone__city) or (zone__subzone__city).
  */
void WizardDateTime::getCurrentTimeZone()
{
    m_currentTimeZone = getTimeZoneStr();
    qDebug() << Q_FUNC_INFO << m_currentTimeZone;

}

void WizardDateTime::setNewTimeZone(QString timezone, int region, int city)
{
    qDebug() << Q_FUNC_INFO << timezone << region << city;
    timezone.replace("__", "/");
    setenv("TZ", timezone.toAscii().constData() , 1);
    if (region >= 0)
        QBook::settings().setValue("setting/zoneregion", region);
    if (city >= 0)
        QBook::settings().setValue("setting/zonecity", city);
    QBook::settings().sync();

    QBookApp::instance()->getStatusBar()->updateTime();

}



QString WizardDateTime::getTimeZoneStr(int region, int city)
{
    qDebug() << Q_FUNC_INFO;
    QString str = NULL;
    if (region < 0)
        region = QBook::settings().value("setting/zoneregion", EUROPE).toInt();
    if (city < 0)
        city = QBook::settings().value("setting/zonecity", 25).toInt(); // Madrid

    qDebug() << Q_FUNC_INFO << region << city;

    str = tr(REGION[region].toUtf8().constData()) + "__" + getTimeCityStr();

    qDebug() << str;
    return str;
}

QString WizardDateTime::getTimeCityStr()
{
    QString str = NULL;
    int timezone = QBook::settings().value("setting/zoneregion", ASIA).toInt();
    int city = QBook::settings().value("setting/zonecity", 0).toInt();

    const QString* cityStr = NULL;
    switch (timezone) {
    case AFRICA:
            cityStr = AFRICA_CITY;
            break;
    case AMERICA:
            cityStr = AMERICA_CITY;
            break;
    case ANTARCTICA:
            cityStr = ANTARCTICA_CITY;
            break;
    case ARCTIC:
            cityStr = ARCTIC_CITY;
            break;
    case ASIA:
            cityStr = ASIA_CITY;
            break;
    case ATLANTIC:
            cityStr = ATLANTIC_CITY;
            break;
    case AUSTRALIA:
            cityStr = AUSTRALIA_CITY;
            break;
    case BRAZIL:
            cityStr = BRAZIL_CITY;
            break;
    case CANADA:
            cityStr = CANADA_CITY;
            break;
    case CHILE:
            cityStr = CHILE_CITY;
            break;
    case EUROPE:
            cityStr = EUROPE_CITY;
            break;
    case INDIAN:
            cityStr = INDIAN_CITY;
            break;
    /*case MEXICO:
            city = MEXICO_CITY;
            break;*/
    case PACIFIC:
            cityStr = PACIFIC_CITY;
            break;
    }
    str = tr(cityStr[city].toUtf8().constData());
    return str;
}


void WizardDateTime::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }
