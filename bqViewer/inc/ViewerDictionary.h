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

#include <QWidget>
#include <QDomDocument>

#include "ui_ViewerDictionaryDefinitionPopup.h"
#include "ProgressDialog.h"


class Viewer;
class DictionaryLayer;

class ViewerDictionary : public QWidget,
                         protected Ui::ViewerDictionaryDefinitionPopup
{
    Q_OBJECT

    enum DictioState
    {
        DICTIO_NONE = 0,
        DICTIO_CLASSIC,
        DICTIO_WIKIPEDIA
    };

public:
    ViewerDictionary(Viewer*);
    virtual ~ViewerDictionary();

protected:
    virtual void                    paintEvent                              (QPaintEvent*);
    virtual void                    mousePressEvent                         (QMouseEvent*);
    virtual void                    showEvent                               (QShowEvent*);
    virtual void                    hideEvent                               (QHideEvent*);

signals:
    void                            hideMe                                  ();

public slots:
   void                             dictioSearch                            (const QStringList&);
   void                             wikiSearch                              (const QString& searchTerm);
   void                             dictioSearchAgain                       ();
   void                             previousPage                            ();
   void                             nextPage                                ();
   void                             setInitialSearch                        (const QString&, const QString&);
   void                             resetInitialSearch                      ();
   void                             clearSearch                             ();

private slots:
   void                             showChangeDictionaryWidget              ();
   void                             dictionaryChanged                       (const QString&);
   void                             handleCloseBtn                          ();
   void                             wordToSearch                            (const QString&, const QString &context);
   void                             handleBackSearchBtn                     ();
   void                             handleDictionaryBtn                     ();
   void                             handleWikipediaWebBtn                   ();
   void                             handleWikipediaBtn                      ();
   void                             getWikipediaSearch                      (const QString& word, QString *searchResult);
   void                             prepareProgressDialog                   ();
   void                             cancelWikipediaRequest                  ();
   bool                             goOnline                                ();
   void                             goOffline                               ();
   void                             connectWifiObserverWikipedia            ();
   void                             disconnectWifiObserverWikipedia         ();
   void                             enableSearchigWikipediaFlag             ();
   void                             disableSearchigWikipediaFlag            ();
   void                             goToWikipedia                           ();
   void                             goToViewer                              ();
   void                             formatWikipediaHtmlInfo                 (QString& html);
   void                             eraseWikipediaHTMLinfoNode              (QString& html, QString node, QString openTag, QString closeTag);
   bool                             eraseFirstNodeOcurrence                 (QString& html, QString node, QString openTag, QString closeTag);
   void                             resetDictionary                         ();
   void                             handleLink                              (const QString&);
   void                             newWikiSearch                           (const QString&);

protected:
   QString                          m_searchTerm;
   QString                          m_context;
   QString                          m_searchResult;
   QStringList                      m_lastSearch;
   Viewer*                          m_parentViewer;
   QScrollBar*                      vbar;

protected:
   void                             showDictioSearch                        (const QString&);
   QString                          getDictionaryName                       (const QString&) const;
   int                              getDictionaryPosition                   (const QString&) const;
   void                             showLastWikiResult                      ();
   void                             considerWikiBackButton                  ();
   void                             considerDictioBackButton                ();
   void                             showBackBtn                             (bool show = true);
   void                             powerOnDictioButton                     ();
   void                             powerOnWikiButton                       ();
   void                             ensureVisibility                        ();
   bool                             containsSeveralWords                    (const QString&);
   void                             dictioSearchFromWikiSentence            (const QString&);
   bool                             eBookLangIsOk                           (QString&);
   bool                             supportedISO639Language                 (QString& lang);

private:
   DictionaryLayer*                 m_changeDictioWidget;
   QStringList                      m_dictioTermSearchHistory;
   QStringList                      m_dictioContextSearchHistory;
   ProgressDialog*                  m_sectionsDialog;
   bool                             b_askingForWikipediaInfo;   
   DictioState                      state, prevState;
   QStringList                      m_wikiTermSearchHistory;
   QStringList                      m_wikiTextHistory;
   QString                          m_currentWikiSearchTerm;
   QString                          m_currentWikiText;
   uint                             m_searchTitleMaxWidth;

private:
   void                             setupPagination                         ();
   void                             requestConnection                       ();
   bool                             setState                                (DictioState newState);
   void                             handleState                             ();
   bool                             isSameThatLastSearch                    (const QString&) const;
   void                             setSearchTitle                          (QString);
};
#endif // VIEWERDICTIONARY_H

