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

#ifndef VIEWERDICTIONARY_H
#define VIEWERDICTIONARY_H

#include "ui_ViewerDictionaryDefinitionPopup.h"
#include <QWidget>

class Viewer;
class DictionaryLayer;

class ViewerDictionary : public QWidget,
                         protected Ui::ViewerDictionaryDefinitionPopup
{
    Q_OBJECT

public:
    ViewerDictionary(Viewer*);
    virtual ~ViewerDictionary();

signals:
    void                            hideMe                                  ();

public slots:
   void                             dictioSearch                            ( const QStringList& );
   void                             searchAgain                             ();
   void                             previousPage                            ();
   void                             nextPage                                ();
   void                             setInitialSearch                        (const QString&, const QString&);
   void                             resetInitialSearch                      ();
   void                             clearSearch                             ();

protected:
   QString                          m_searchTerm;
   QString                          m_context;
   QString                          m_searchResult;
   QStringList                      m_lastSearch;
   Viewer*                          m_parentViewer;
   QScrollBar*                      vbar;

   void                             showSearch                              ( const QString& );
   QString                          getDictionaryName                       ( const QString& ) const;
   int                              getDictionaryPosition                   ( const QString& ) const;
   virtual void                     paintEvent                              ( QPaintEvent* );

private slots:
   void                             showChangeDictionaryWidget              ();
   void                             dictionaryChanged                       ( const QString& );
   void                             handleCloseBtn                          ();
   void                             wordToSearch                            ( const QString&, const QString &context );
   void                             handleBackSearchBtn                     ();

private:
   void                             setupPagination                         ();
   DictionaryLayer*                 m_changeDictioWidget;
   QStringList                      mIterativeSearchHistory;
   QStringList                      mIterativeContextSearchHistory;
};
#endif // VIEWERDICTIONARY_H

