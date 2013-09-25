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

#ifndef QDICTINTERFACE_H
#define QDICTINTERFACE_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <cstdlib>
#include <cstdio>
#include <clocale>

#include <string>
#include <vector>
#include <map>
#include <QList>

#include <QtGui/QWidget>
#include <QtGui/QPushButton>

#include <QtGui/QTextEdit>
#include <QtGui/QLineEdit>
#include <QtGui/QBoxLayout>

#include "semantix_dixio.h"

using std::string;
using std::vector;

class QTextEdit;
class QLineEdit;
class QPushButton;
class QString;
class QObject;

class DictionaryParams
{
    public:
        QString id;
        QString commercialName;
        QString shortName;
        QString downloadUrl;
        QString downloadUrlComp;
        QString downloadFileName;
        QString downloadFileNameComp;
        int     dixFileSize;
        int     compFileSize;
        bool    defaultDictio;
        bool    activationState;
};

class Dictionary : public QObject
{
    Q_OBJECT

public:
        static Dictionary*              instance                    ( ) { return m_spInstance; }
        static void                     staticInit                  ( );
        static void                     staticDone                  ( );

        QString                         translate                   ( const QString&, const QString&);
        QMap<QString, DictionaryParams> parseCurrentDictionaries    ( const QString& ) ;
        QMap<QString, DictionaryParams> parseAvailableDictionaries  ( const QString&, QString& ) ;
        QByteArray                      getJsonDictionaries         ( ) const;
        QByteArray                      getJsonAvailableDicts       ( ) const;
        bool                            getActivation               ( QString, QString&, QString&);
        String                          setActivation               ( QString, QString);

        QString                         getCurrentDictCode          () const;
        QString                         getDixioKey                 ();
        void                            renewDixioKey               ();

private:

                                        Dictionary                  ( );
        virtual                         ~Dictionary                 ( );
        void                            loadSemantixLib             ( const QString& );
        QString                         rename                      ( QString& );
        string                          getDictCode                 ( ) const;


        static Dictionary*   m_spInstance;

        QString     word;
        QTextEdit*  showOutput;
        QString     result;
        QString     m_dixioKey;

        bool initialized;
        void *handle;

        void (*initialize)(String, String);
        void (*finalize)();
        String (*getDictionaries) ();
        const std::string (*getWorkingDictionariesEx) ();
        String (*searchHtml) (String, String, String, int, String, int, int);
        String (*getVersion) ();

        bool (*getActivationData)(String, String , String& , String&);
        String (*setActivationData)(String, String);



#ifdef Q_WS_QWS
        static bool isLoaded;
#endif

};

struct TSearchResult{
  string bookname;
  string def;
  string exp;

  TSearchResult(const string& bookname_, const string& def_, const string& exp_)
    : bookname(bookname_), def(def_), exp(exp_) {}
};

typedef vector<TSearchResult> TSearchResultList;
typedef TSearchResultList::iterator PSearchResult;

#endif // QDICTINTERFACE_H
