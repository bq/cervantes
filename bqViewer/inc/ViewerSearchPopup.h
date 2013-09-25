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

#ifndef VIEWERSEARCHPOPUP_H
#define VIEWERSEARCHPOPUP_H

#include "ViewerMenuPopUp.h"
#include "ui_ViewerSearchPopup.h"

#include "QDocView.h"
#include <QTimer>

#include <QFutureWatcher>
#include <QMutex>

// Predeclaration
class QWidget;
class Viewer;
class Keyboard;
class ViewerSearchResultItem;
class ViewerSearchContextMenu;

class ViewerSearchPopup : public ViewerMenuPopUp, protected Ui::ViewerSearchPopup
{
    Q_OBJECT

public:
    ViewerSearchPopup( Viewer* viewer );
    virtual ~ViewerSearchPopup();

    virtual void                        setup                               ();
    virtual void                        start                               ();
    virtual void                        stop                                ();

    void                                searchRequest                       ( const QString& text );
    void                                stopSearch                          ();

signals:
    void                                newPageReady                        ();
    void                                searchFinished                      ();

protected slots:
    void                                handleClearEditLineClicked          ();
    void                                handleCloseSearchClicked            ();
    void                                handleSearchLineEditClicked         ();

    void                                keyboardPressedChar                 ( const QString& input );
    void                                handleKeyboardAction                ();

    void                                updateCounters                      ();

    void                                handleResultPressed                 ( const QString& url );

    void                                previousPage                        ();
    void                                nextPage                            ();
    void                                firstPage                           ();
    void                                lastPage                            ();
    void                                navigateToNext                      ();
    void                                navigateToPrevious                  ();
    void                                navigateToCurrent                   ();
    void                                navigateToPending                   ();
    void                                clearSearch                         ();
    void                                handleBackToList                    ();

protected:
    virtual void                        paintEvent                          ( QPaintEvent* );

    void                                performSearch                       ( const QString& searchTerm );

    void                                performSearchConcurrent             ();

    void                                showKeyboardSearch                  ();
    void                                hideKeyboardSearch                  ();

    Keyboard*                           m_keyboard;

    QString                             m_initialSearchText;
    bool                                m_userTyped;
    bool                                b_goToSearch;

    ViewerSearchContextMenu *           m_navigationMenu;
    int                                 i_currentShownResultIndex;
    int                                 i_firstShownIndex;

    bool                                m_isSearchActive;
    bool                                m_alreadyPainted;
    bool                                b_pendingGoto;
    bool                                m_finishSearch;

    QMutex                              m_searchMutex;
    QFutureWatcher<void>                m_futureWatcher;

    QDocView::SearchResult*             m_currentSearch;
    QList<QDocView::Location*>          m_searchResults;
    QString                             m_searchTerm;
    void                                clearSearchResults                  ();

    void                                trimPreview                         ( QString& totalStr );
    void                                setupPagination                     ();

    static const int                    RESULTS_PER_PAGE = 5;
    int                                 m_page;
    int                                 m_totalPages;
    ViewerSearchResultItem*             m_uiSearchItems[RESULTS_PER_PAGE];
    void                                paintUISearchItems                    ();
    QTimer*                             m_timer_gotoSearch;
};

#endif // VIEWERSEARCHPOPUP_H
