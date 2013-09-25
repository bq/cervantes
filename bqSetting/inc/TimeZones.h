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

#ifndef TIMEZONES_H
#define TIMEZONES_H

#include <QDate>
#include <QTime>

enum TimeZoneRegion
    {
        AFRICA = 0,
        AMERICA = 1,
        ANTARCTICA = 2,
        ARCTIC = 3,
        ASIA = 4,
        ATLANTIC = 5,
        AUSTRALIA = 6,
        BRAZIL = 7,
        CANADA = 8,
        CHILE = 9,
        EUROPE = 10,
        INDIAN = 11,
        PACIFIC = 12,
        TIMEZONE_END = 13
    };
        static const int REGION_CNT = 13;
        static const QString REGION[] = {
         "Africa",
         "America",
         "Antarctica",
         "Arctic",
         "Asia",
         "Atlantic",
         "Australia",
         "Brazil",
         "Canada",
         "Chile",
         "Europe",
         "Indian",
         "Pacific",
        };

        static int timeZoneRegionStrToInt(QString region)
        {
            for (int i = 0; i < REGION_CNT; ++i)
            {
                if (REGION[i] == region)
                    return i;
            }
            return -1;
        };

    static const int  AFRICA_CITY_CNT = 52;
        static const QString AFRICA_CITY[] = {
         "Abidjan",
         "Accra",
         "Addis Ababa",
         "Algiers",
         "Asmara",
         "Bamako",
         "Bangui",
         "Banjul",
         "Bissau",
         "Blantyre",
         "Brazzaville",
         "Bujumbura",
         "Cairo",
         "Casablanca",
         "Ceuta",
         "Conakry",
         "Dakar",
         "Dar_es_Salaam",
         "Djibouti",
         "Douala",
         "El_Aaiun",
         "Freetown",
         "Gaborone",
         "Harare",
         "Johannesburg",
         "Kampala",
         "Khartoum",
         "Kigali",
         "Kinshasa",
         "Lagos",
         "Libreville",
         "Lome",
         "Luanda",
         "Lubumbashi",
         "Lusaka",
         "Malabo",
         "Maputo",
         "Maseru",
         "Mbabane",
         "Mogadishu",
         "Monrovia",
         "Nairobi",
         "Ndjamena",
         "Niamey",
         "Nouakchott",
         "Ouagadougou",
         "Porto-Novo",
         "Sao_Tome",
         "Timbuktu",
         "Tripoli",
         "Tunis",
         "Windhoek",
        };

        static int timeZoneAfricaCityStrToInt(QString city)
        {
            for (int i = 0; i < AFRICA_CITY_CNT; ++i)
            {
                if (AFRICA_CITY[i] == city)
                    return i;
            }
            return -1;
        };

    static const QString AFRICA_TIMEZONE[] = {
        "GMT0",
        "GMT0",
        "EAT-3",
        "CET-1",
        "EAT-3",
        "GMT0",
        "WAT-1",
        "GMT0",
        "GMT0",
        "CAT-2",
        "WAT-1",
        "CAT-2",
        "EEST",
        "WET0",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "GMT0",
        "GMT0",
        "EAT-3",
        "EAT-3",
        "WAT-1",
        "WET0",
        "GMT0",
        "CAT-2",
        "CAT-2",
        "SAST-2",
        "EAT-3",
        "EAT-3",
        "CAT-2",
        "WAT-1",
        "WAT-1",
        "WAT-1",
        "GMT0",
        "WAT-1",
        "CAT-2",
        "CAT-2",
        "WAT-1",
        "CAT-2",
        "SAST-2",
        "SAST-2",
        "EAT-3",
        "GMT0",
        "EAT-3",
        "WAT-1",
        "WAT-1",
        "GMT0",
        "GMT0",
        "WAT-1",
        "GMT0",
        "GMT0",
        "EET-2",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "WAT-1WAST,M9.1.0,M4.1.0",
        };

        static const int  AMERICA_CITY_CNT = 152;
        static const QString AMERICA_CITY[] = {
         "Adak",
         "Anchorage",
         "Anguilla",
         "Antigua",
         "Araguaina",
         "Argentina/Buenos_Aires",
         "Argentina/Catamarca",
         "Argentina/ComodRivadavia",
         "Argentina/Cordoba",
         "Argentina/Jujuy",
         "Argentina/La_Rioja",
         "Argentina/Mendoza",
         "Argentina/Rio_Gallegos",
         "Argentina/San_Juan",
         "Argentina/San_Luis",
         "Argentina/Tucuman",
         "Argentina/Ushuaia",
         "Aruba",
         "Asuncion",
         "Atikokan",
         "Atka",
         "Bahia",
         "Barbados",
         "Belem",
         "Belize",
         "Blanc-Sablon",
         "Boa_Vista",
         "Bogota",
         "Boise",
         "Buenos_Aires",
         "Cambridge_Bay",
         "Campo_Grande",
         "Cancun",
         "Caracas",
         "Catamarca",
         "Cayenne",
         "Cayman",
         "Chicago",
         "Chihuahua",
         "Coral_Harbour",
         "Cordoba",
         "Costa_Rica",
         "Cuiaba",
         "Curacao",
         "Danmarkshavn",
         "Dawson",
         "Dawson_Creek",
         "Denver",
         "Detroit",
         "Dominica",
         "Edmonton",
         "Eirunepe",
         "El_Salvador",
         "Ensenada",
         "Fortaleza",
         "Fort_Wayne",
         "Glace_Bay",
         "Godthab",
         "Goose_Bay",
         "Grand_Turk",
         "Grenada",
         "Guadeloupe",
         "Guatemala",
         "Guayaquil",
         "Guyana",
         "Halifax",
         "Havana",
         "Hermosillo",
         "Indiana/Indianapolis",
         "Indiana/Knox",
         "Indiana/Marengo",
         "Indiana/Petersburg",
         "Indianapolis",
         "Indiana/Tell_City",
         "Indiana/Vevay",
         "Indiana/Vincennes",
         "Indiana/Winamac",
         "Inuvik",
         "Iqaluit",
         "Jamaica",
         "Jujuy",
         "Juneau",
         "Kentucky/Louisville",
         "Kentucky/Monticello",
         "Knox_IN",
         "La_Paz",
         "Lima",
         "Los_Angeles",
         "Louisville",
         "Maceio",
         "Managua",
         "Manaus",
         "Marigot",
         "Martinique",
         "Mazatlan",
         "Mendoza",
         "Menominee",
         "Merida",
         "Mexico_City",
         "Miquelon",
         "Moncton",
         "Monterrey",
         "Montevideo",
         "Montreal",
         "Montserrat",
         "Nassau",
         "New_York",
         "Nipigon",
         "Nome",
         "Noronha",
         "North_Dakota/Center",
         "North_Dakota/New_Salem",
         "Panama",
         "Pangnirtung",
         "Paramaribo",
         "Phoenix",
         "Port-au-Prince",
         "Porto_Acre",
         "Port_of_Spain",
         "Porto_Velho",
         "Puerto_Rico",
         "Rainy_River",
         "Rankin_Inlet",
         "Recife",
         "Regina",
         "Resolute",
         "Rio_Branco",
         "Rosario",
         "Santiago",
         "Santo_Domingo",
         "Sao_Paulo",
         "Scoresbysund",
         "Shiprock",
         "St_Barthelemy",
         "St_Johns",
         "St_Kitts",
         "St_Lucia",
         "St_Thomas",
         "St_Vincent",
         "Swift_Current",
         "Tegucigalpa",
         "Thule",
         "Thunder_Bay",
         "Tijuana",
         "Toronto",
         "Tortola",
         "Vancouver",
         "Virgin",
         "Whitehorse",
         "Winnipeg",
         "Yakutat",
         "Yellowknife",
        };

        static int timeZoneAmericaCityStrToInt(QString city)
        {
            for (int i = 0; i < AMERICA_CITY_CNT; ++i)
            {
                if (AMERICA_CITY[i] == city)
                    return i;
            }
            return -1;
        };

    static const QString AMERICA_TIMEZONE[] = {
        "HAST10HADT,M3.2.0,M11.1.0",
        "AKST9AKDT,M3.2.0,M11.1.0",
        "AST4",
        "AST4",
        "BRT3",
        "ART3ARST,M10.1.0/0,M3.3.0/0",
        "ART3ARST,M10.1.0/0,M3.3.0/0",
        "ART3ARST,M10.1.0/0,M3.3.0/0",
        "ART3ARST,M10.1.0/0,M3.3.0/0",
        "ART3ARST,M10.1.0/0,M3.3.0/0",
        "ART3ARST,M10.1.0/0,M3.3.0/0",
        "ART3ARST,M10.1.0/0,M3.3.0/0",
        "ART3ARST,M10.1.0/0,M3.3.0/0",
        "ART3ARST,M10.1.0/0,M3.3.0/0",
        "ART3",
        "ART3ARST,M10.1.0/0,M3.3.0/0",
        "ART3ARST,M10.1.0/0,M3.3.0/0",
        "AST4",
        "PYT4PYST,M10.3.0/0,M3.2.0/0",
        "EST5",
        "HAST10HADT,M3.2.0,M11.1.0",
        "BRT3",
        "AST4",
        "BRT3",
        "CST6",
        "AST4",
        "AMT4",
        "COT5",
        "MST7MDT,M3.2.0,M11.1.0",
        "ART3ARST,M10.1.0/0,M3.3.0/0",
        "MST7MDT,M3.2.0,M11.1.0",
        "AMT4AMST,M10.2.0/0,M2.3.0/0",
        "CST6CDT,M4.1.0,M10.5.0",
        "VET4:30",
        "ART3ARST,M10.1.0/0,M3.3.0/0",
        "GFT3",
        "EST5",
        "CST6CDT,M3.2.0,M11.1.0",
        "MST7MDT,M4.1.0,M10.5.0",
        "EST5",
        "ART3ARST,M10.1.0/0,M3.3.0/0",
        "CST6",
        "AMT4AMST,M10.2.0/0,M2.3.0/0",
        "AST4",
        "GMT0",
        "PST8PDT,M3.2.0,M11.1.0",
        "MST7",
        "MST7MDT,M3.2.0,M11.1.0",
        "EST5EDT,M3.2.0,M11.1.0",
        "AST4",
        "MST7MDT,M3.2.0,M11.1.0",
        "ACT5",
        "CST6",
        "PST8PDT,M4.1.0,M10.5.0",
        "BRT3",
        "EST5EDT,M3.2.0,M11.1.0",
        "AST4ADT,M3.2.0,M11.1.0",
        "WGST",
        "AST4ADT,M3.2.0/0:01,M11.1.0/0:01",
        "EST5EDT,M3.2.0,M11.1.0",
        "AST4",
        "AST4",
        "CST6",
        "ECT5",
        "GYT4",
        "AST4ADT,M3.2.0,M11.1.0",
        "CST5CDT,M3.3.0/0,M10.5.0/1",
        "MST7",
        "EST5EDT,M3.2.0,M11.1.0",
        "CST6CDT,M3.2.0,M11.1.0",
        "EST5EDT,M3.2.0,M11.1.0",
        "EST5EDT,M3.2.0,M11.1.0",
        "EST5EDT,M3.2.0,M11.1.0",
        "CST6CDT,M3.2.0,M11.1.0",
        "EST5EDT,M3.2.0,M11.1.0",
        "EST5EDT,M3.2.0,M11.1.0",
        "EST5EDT,M3.2.0,M11.1.0",
        "MST7MDT,M3.2.0,M11.1.0",
        "EST5EDT,M3.2.0,M11.1.0",
        "EST5",
        "ART3ARST,M10.1.0/0,M3.3.0/0",
        "AKST9AKDT,M3.2.0,M11.1.0",
        "EST5EDT,M3.2.0,M11.1.0",
        "EST5EDT,M3.2.0,M11.1.0",
        "CST6CDT,M3.2.0,M11.1.0",
        "BOT4",
        "PET5",
        "PST8PDT,M3.2.0,M11.1.0",
        "EST5EDT,M3.2.0,M11.1.0",
        "BRT3",
        "CST6",
        "AMT4",
        "AST4",
        "AST4",
        "MST7MDT,M4.1.0,M10.5.0",
        "ART3ARST,M10.1.0/0,M3.3.0/0",
        "CST6CDT,M3.2.0,M11.1.0",
        "CST6CDT,M4.1.0,M10.5.0",
        "CST6CDT,M4.1.0,M10.5.0",
        "PMST3PMDT,M3.2.0,M11.1.0",
        "AST4ADT,M3.2.0,M11.1.0",
        "CST6CDT,M4.1.0,M10.5.0",
        "UYT3UYST,M10.1.0,M3.2.0",
        "EST5EDT,M3.2.0,M11.1.0",
        "AST4",
        "EST5EDT,M3.2.0,M11.1.0",
        "EST5EDT,M3.2.0,M11.1.0",
        "EST5EDT,M3.2.0,M11.1.0",
        "AKST9AKDT,M3.2.0,M11.1.0",
        "FNT2",
        "CST6CDT,M3.2.0,M11.1.0",
        "CST6CDT,M3.2.0,M11.1.0",
        "EST5",
        "EST5EDT,M3.2.0,M11.1.0",
        "SRT3",
        "MST7",
        "EST5",
        "ACT5",
        "AST4",
        "AMT4",
        "AST4",
        "CST6CDT,M3.2.0,M11.1.0",
        "CST6CDT,M3.2.0,M11.1.0",
        "BRT3",
        "CST6",
        "EST5",
        "ACT5",
        "ART3ARST,M10.1.0/0,M3.3.0/0",
        "CLST",
        "AST4",
        "BRT3BRST,M10.2.0/0,M2.3.0/0",
        "EGT1EGST,M3.5.0/0,M10.5.0/1",
        "MST7MDT,M3.2.0,M11.1.0",
        "AST4",
        "NST3:30NDT,M3.2.0/0:01,M11.1.0/0:01",
        "AST4",
        "AST4",
        "AST4",
        "AST4",
        "CST6",
        "CST6",
        "AST4ADT,M3.2.0,M11.1.0",
        "EST5EDT,M3.2.0,M11.1.0",
        "PST8PDT,M4.1.0,M10.5.0",
        "EST5EDT,M3.2.0,M11.1.0",
        "AST4",
        "PST8PDT,M3.2.0,M11.1.0",
        "AST4",
        "PST8PDT,M3.2.0,M11.1.0",
        "CST6CDT,M3.2.0,M11.1.0",
        "AKST9AKDT,M3.2.0,M11.1.0",
        "MST7MDT,M3.2.0,M11.1.0",
        };

        static const int  ANTARCTICA_CITY_CNT = 9;
        static const QString ANTARCTICA_CITY[] = {
         "Casey",
         "Davis",
         "Mawson",
         "McMurdo",
         "Palmer",
         "Rothera",
         "South_Pole",
         "Syowa",
         "Vostok",
        };

        static int timeZoneAntarcticaCityStrToInt(QString city)
        {
            for (int i = 0; i < ANTARCTICA_CITY_CNT; ++i)
            {
                if (ANTARCTICA_CITY[i] == city)
                    return i;
            }
            return -1;
        };

    static const QString ANTARCTICA_TIMEZONE[] = {
        "WST-8",
        "DAVT-7",
        "MAWT-6",
        "NZST-12NZDT,M9.5.0,M4.1.0/3",
        "CLST",
        "ROTT3",
        "NZST-12NZDT,M9.5.0,M4.1.0/3",
        "SYOT-3",
        "VOST-6",
        };

        static const int  ARCTIC_CITY_CNT = 1;
        static const QString ARCTIC_CITY[] = {
         "Longyearbyen",
        };
        static int timeZoneArcticCityStrToInt(QString city)
        {
            for (int i = 0; i < ARCTIC_CITY_CNT; ++i)
            {
                if (ARCTIC_CITY[i] == city)
                    return i;
            }
            return -1;
        };
    static const QString ARCTIC_TIMEZONE[] = {
        "CET-1CEST,M3.5.0,M10.5.0/3",
        };

        static const int  ASIA_CITY_CNT = 81;
        static const QString ASIA_CITY[] = {
         "Aden",
         "Almaty",
         "Amman",
         "Anadyr",
         "Aqtau",
         "Aqtobe",
         "Ashgabat",
         "Ashkhabad",
         "Baghdad",
         "Bahrain",
         "Baku",
         "Bangkok",
         "Beirut",
         "Bishkek",
         "Brunei",
         "Calcutta",
         "Choibalsan",
         "Chungking",
         "Colombo",
         "Dacca",
         "Damascus",
         "Dhaka",
         "Dili",
         "Dubai",
         "Dushanbe",
         "Gaza",
         "Harbin",
         "Ho_Chi_Minh",
         "Hong_Kong",
         "Hovd",
         "Irkutsk",
         "Istanbul",
         "Jakarta",
         "Jayapura",
         "Jerusalem",
         "Kabul",
         "Kamchatka",
         "Karachi",
         "Kashgar",
         "Katmandu",
         "Kolkata",
         "Krasnoyarsk",
         "Kuala_Lumpur",
         "Kuching",
         "Kuwait",
         "Macau",
         "Magadan",
         "Makassar",
         "Manila",
         "Muscat",
         "Nicosia",
         "Novosibirsk",
         "Omsk",
         "Oral",
         "Phnom_Penh",
         "Pontianak",
         "Pyongyang",
         "Qatar",
         "Rangoon",
         "Riyadh",
         "Saigon",
         "Sakhalin",
         "Samarkand",
         "Seoul",
         "Shanghai",
         "Singapore",
         "Taipei",
         "Tashkent",
         "Tbilisi",
         "Tehran",
         "Tel_Aviv",
         "Thimbu",
         "Tokyo",
         "Ujung_Pandang",
         "Ulan_Bator",
         "Urumqi",
         "Vientiane",
         "Vladivostok",
         "Yakutsk",
         "Yekaterinburg",
         "Yerevan",
        };

        static int timeZoneAsiaCityStrToInt(QString city)
        {
            for (int i = 0; i < ASIA_CITY_CNT; ++i)
            {
                if (ASIA_CITY[i] == city)
                    return i;
            }
            return -1;
        };
    static const QString ASIA_TIMEZONE[] = {
        "AST-3",
        "ALMT-6",
        "EET-2EEST,M3.5.4/0,M10.5.5/1",
        "ANAT-12ANAST,M3.5.0,M10.5.0/3",
        "AQTT-5",
        "AQTT-5",
        "TMT-5",
        "TMT-5",
        "AST-3",
        "AST-3",
        "AZT-4AZST,M3.5.0/4,M10.5.0/5",
        "ICT-7",
        "EET-2EEST,M3.5.0/0,M10.5.0/0",
        "KGT-6",
        "BNT-8",
        "IST-5:30",
        "CHOT-9",
        "CST-8",
        "IST-5:30",
        "BDT-6",
        "EET-2EEST,M4.1.5/0,J274/0",
        "BDT-6",
        "TLT-9",
        "GST-4",
        "TJT-5",
        "EET-2EEST,J91/0,M9.2.4",
        "CST-8",
        "ICT-7",
        "HKT-8",
        "HOVT-7",
        "IRKT-8IRKST,M3.5.0,M10.5.0/3",
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "WIT-7",
        "EIT-9",
        "IDDT",
        "AFT-4:30",
        "PETT-12PETST,M3.5.0,M10.5.0/3",
        "PKT-5",
        "CST-8",
        "NPT-5:45",
        "IST-5:30",
        "KRAT-7KRAST,M3.5.0,M10.5.0/3",
        "MYT-8",
        "MYT-8",
        "AST-3",
        "CST-8",
        "MAGT-11MAGST,M3.5.0,M10.5.0/3",
        "CIT-8",
        "PHT-8",
        "GST-4",
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "NOVT-6NOVST,M3.5.0,M10.5.0/3",
        "OMST-6OMSST,M3.5.0,M10.5.0/3",
        "ORAT-5",
        "ICT-7",
        "WIT-7",
        "KST-9",
        "AST-3",
        "MMT-6:30",
        "AST-3",
        "ICT-7",
        "SAKT-10SAKST,M3.5.0,M10.5.0/3",
        "UZT-5",
        "KST-9",
        "CST-8",
        "SGT-8",
        "CST-8",
        "UZT-5",
        "GET-4",
        "IRDT",
        "IDDT",
        "BTT-6",
        "JST-9",
        "CIT-8",
        "ULAT-8",
        "CST-8",
        "ICT-7",
        "VLAT-10VLAST,M3.5.0,M10.5.0/3",
        "YAKT-9YAKST,M3.5.0,M10.5.0/3",
        "YEKT-5YEKST,M3.5.0,M10.5.0/3",
        "AMT-4AMST,M3.5.0,M10.5.0/3",
        };

        static const int  ATLANTIC_CITY_CNT = 11;
        static const QString ATLANTIC_CITY[] = {
         "Azores",
         "Bermuda",
         "Canary",
         "Cape_Verde",
         "Faeroe",
         "Jan_Mayen",
         "Madeira",
         "Reykjavik",
         "South_Georgia",
         "Stanley",
         "St_Helena",
        };

        static int timeZoneAtlanticCityStrToInt(QString city)
        {
            for (int i = 0; i < ATLANTIC_CITY_CNT; ++i)
            {
                if (ATLANTIC_CITY[i] == city)
                    return i;
            }
            return -1;
        };

    static const QString ATLANTIC_TIMEZONE[] = {
        "AZOT1AZOST,M3.5.0/0,M10.5.0/1",
        "AST4ADT,M3.2.0,M11.1.0",
        "WET0WEST,M3.5.0/1,M10.5.0",
        "CVT1",
        "WET0WEST,M3.5.0/1,M10.5.0",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "WET0WEST,M3.5.0/1,M10.5.0",
        "GMT0",
        "GST2",
        "FKT4FKST,M9.1.0,M4.3.0",
        "GMT0",
        };

        static const int  AUSTRALIA_CITY_CNT = 20;
        static const QString AUSTRALIA_CITY[] = {
         "Adelaide",
         "Brisbane",
         "Broken_Hill",
         "Canberra",
         "Currie",
         "Darwin",
         "Eucla",
         "Hobart",
         "Lindeman",
         "Lord_Howe",
         "Melbourne",
         "North",
         "NSW",
         "Perth",
         "Queensland",
         "South",
         "Sydney",
         "Tasmania",
         "Victoria",
         "West",
        };

        static int timeZoneAustraliaCityStrToInt(QString city)
        {
            for (int i = 0; i < AUSTRALIA_CITY_CNT; ++i)
            {
                if (AUSTRALIA_CITY[i] == city)
                    return i;
            }
            return -1;
        };

    static const QString AUSTRALIA_TIMEZONE[] = {
        "CST-9:30CST,M10.1.0,M4.1.0/3",
        "EST-10",
        "CST-9:30CST,M10.1.0,M4.1.0/3",
        "EST-10EST,M10.1.0,M4.1.0/3",
        "EST-10EST,M10.1.0,M4.1.0/3",
        "CST-9:30",
        "CWST-8:45",
        "EST-10EST,M10.1.0,M4.1.0/3",
        "EST-10",
        "LHST-10:30LHST-11,M10.1.0,M4.1.0",
        "EST-10EST,M10.1.0,M4.1.0/3",
        "CST-9:30",
        "EST-10EST,M10.1.0,M4.1.0/3",
        "WST-8",
        "EST-10",
        "CST-9:30CST,M10.1.0,M4.1.0/3",
        "EST-10EST,M10.1.0,M4.1.0/3",
        "EST-10EST,M10.1.0,M4.1.0/3",
        "EST-10EST,M10.1.0,M4.1.0/3",
        "WST-8",
        };

        static const int  BRAZIL_CITY_CNT = 4;
        static const QString BRAZIL_CITY[] = {
         "Acre",
         "DeNoronha",
         "East",
         "West",
        };
        static int timeZoneBrazilCityStrToInt(QString city)
        {
            for (int i = 0; i < BRAZIL_CITY_CNT; ++i)
            {
                if (BRAZIL_CITY[i] == city)
                    return i;
            }
            return -1;
        };
    static const QString BRAZIL_TIMEZONE[] = {
        "ACT5",
        "FNT2",
        "BRT3BRST,M10.2.0/0,M2.3.0/0",
        "AMT4",
        };

        static const int  CANADA_CITY_CNT = 9;
        static const QString CANADA_CITY[] = {
         "Atlantic",
         "Central",
         "Eastern",
         "East-Saskatchewan",
         "Mountain",
         "Newfoundland",
         "Pacific",
         "Saskatchewan",
         "Yukon",
        };
        static int timeZoneCanadaCityStrToInt(QString city)
        {
            for (int i = 0; i < CANADA_CITY_CNT; ++i)
            {
                if (CANADA_CITY[i] == city)
                    return i;
            }
            return -1;
        };
    static const QString CANADA_TIMEZONE[] = {
        "AST4ADT,M3.2.0,M11.1.0",
        "CST6CDT,M3.2.0,M11.1.0",
        "EST5EDT,M3.2.0,M11.1.0",
        "CST6",
        "MST7MDT,M3.2.0,M11.1.0",
        "NST3:30NDT,M3.2.0/0:01,M11.1.0/0:01",
        "PST8PDT,M3.2.0,M11.1.0",
        "CST6",
        "PST8PDT,M3.2.0,M11.1.0",
        };

        static const int  CHILE_CITY_CNT = 2;
        static const QString CHILE_CITY[] = {
         "Continental",
         "EasterIsland",
        };

        static int timeZoneChileCityStrToInt(QString city)
        {
            for (int i = 0; i < CHILE_CITY_CNT; ++i)
            {
                if (CHILE_CITY[i] == city)
                    return i;
            }
            return -1;
        };

    static const QString CHILE_TIMEZONE[] = {
        "CLST",
        "EASST",
        };

        static const int  EUROPE_CITY_CNT = 58;
        static const QString EUROPE_CITY[] = {
         "Amsterdam",
         "Andorra",
         "Athens",
         "Belfast",
         "Belgrade",
         "Berlin",
         "Bratislava",
         "Brussels",
         "Bucharest",
         "Budapest",
         "Chisinau",
         "Copenhagen",
         "Dublin",
         "Gibraltar",
         "Guernsey",
         "Helsinki",
         "Isle_of_Man",
         "Istanbul",
         "Jersey",
         "Kaliningrad",
         "Kiev",
         "Lisbon",
         "Ljubljana",
         "London",
         "Luxembourg",
         "Madrid",
         "Malta",
         "Mariehamn",
         "Minsk",
         "Monaco",
         "Moscow",
         "Nicosia",
         "Oslo",
         "Paris",
         "Podgorica",
         "Prague",
         "Riga",
         "Rome",
         "Samara",
         "San Marino",
         "Sarajevo",
         "Simferopol",
         "Skopje",
         "Sofia",
         "Stockholm",
         "Tallinn",
         "Tirane",
         "Tiraspol",
         "Uzhgorod",
         "Vaduz",
         "Vatican",
         "Vienna",
         "Vilnius",
         "Volgograd",
         "Warsaw",
         "Zagreb",
         "Zaporozhye",
         "Zurich",
        };
        static int timeZoneEuropeCityStrToInt(QString city)
        {
            for (int i = 0; i < EUROPE_CITY_CNT; ++i)
            {
                if (EUROPE_CITY[i] == city)
                    return i;
            }
            return -1;
        };
    static const QString EUROPE_TIMEZONE[] = {
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "GMT0BST,M3.5.0/1,M10.5.0",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "GMT0IST,M3.5.0/1,M10.5.0",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "GMT0BST,M3.5.0/1,M10.5.0",
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "GMT0BST,M3.5.0/1,M10.5.0",
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "GMT0BST,M3.5.0/1,M10.5.0",
        "EET-2EEST,M3.5.0,M10.5.0/3",
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "WET0WEST,M3.5.0/1,M10.5.0",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "GMT0BST,M3.5.0/1,M10.5.0",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "EET-2EEST,M3.5.0,M10.5.0/3",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "MSK-3MSD,M3.5.0,M10.5.0/3",
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "SAMT-4SAMST,M3.5.0,M10.5.0/3",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "VOLT-3VOLST,M3.5.0,M10.5.0/3",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "CET-1CEST,M3.5.0,M10.5.0/3",
        };

        static const int  INDIAN_CITY_CNT = 11;
        static const QString INDIAN_CITY[] = {
         "Antananarivo",
         "Chagos",
         "Christmas",
         "Cocos",
         "Comoro",
         "Kerguelen",
         "Mahe",
         "Maldives",
         "Mauritius",
         "Mayotte",
         "Reunion",
        };
        static int timeZoneIndianCityStrToInt(QString city)
        {
            for (int i = 0; i < INDIAN_CITY_CNT; ++i)
            {
                if (INDIAN_CITY[i] == city)
                    return i;
            }
            return -1;
        };
    static const QString INDIAN_TIMEZONE[] = {
        "EAT-3",
        "IOT-6",
        "CXT-7",
        "CCT-6:30",
        "EAT-3",
        "TFT-5",
        "SCT-4",
        "MVT-5",
        "MUT-4",
        "EAT-3",
        "RET-4",
        };

        static const int  PACIFIC_CITY_CNT = 40;
        static const QString PACIFIC_CITY[] = {
         "Apia",
         "Auckland",
         "Chatham",
         "Easter",
         "Efate",
         "Enderbury",
         "Fakaofo",
         "Fiji",
         "Funafuti",
         "Galapagos",
         "Gambier",
         "Guadalcanal",
         "Guam",
         "Honolulu",
         "Johnston",
         "Kiritimati",
         "Kosrae",
         "Kwajalein",
         "Majuro",
         "Marquesas",
         "Midway",
         "Nauru",
         "Niue",
         "Norfolk",
         "Noumea",
         "Pago_Pago",
         "Palau",
         "Pitcairn",
         "Ponape",
         "Port_Moresby",
         "Rarotonga",
         "Saipan",
         "Samoa",
         "Tahiti",
         "Tarawa",
         "Tongatapu",
         "Truk",
         "Wake",
         "Wallis",
         "Yap",
        };
        static int timeZonePacificCityStrToInt(QString city)
        {
            for (int i = 0; i < PACIFIC_CITY_CNT; ++i)
            {
                if (PACIFIC_CITY[i] == city)
                    return i;
            }
            return -1;
        };
    static const QString PACIFIC_TIMEZONE[] = {
        "WST11",
        "NZST-12NZDT,M9.5.0,M4.1.0/3",
        "CHAST-12:45CHADT,M9.5.0/2:45,M4.1.0/3:45",
        "EASST",
        "VUT-11",
        "PHOT-13",
        "TKT10",
        "FJT-12",
        "TVT-12",
        "GALT6",
        "GAMT9",
        "SBT-11",
        "ChST-10",
        "HST10",
        "HST10",
        "LINT-14",
        "KOST-11",
        "MHT-12",
        "MHT-12",
        "MART9:30",
        "SST11",
        "NRT-12",
        "NUT11",
        "NFT-11:30",
        "NCT-11",
        "SST11",
        "PWT-9",
        "PST8",
        "PONT-11",
        "PGT-10",
        "CKT10",
        "ChST-10",
        "SST11",
        "TAHT10",
        "GILT-12",
        "TOT-13",
        "TRUT-10",
        "WAKT-12",
        "WFT-12",
        "TRUT-10",
        };

#endif // TIMEZONES_H
