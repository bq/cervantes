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

#ifndef BQSEARCHLIBRARY_H
#define BQSEARCHLIBRARY_H

#include "QBookForm.h"
#include "ui_Search.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QString>

#include <QMutex>
#include <QFileInfo>

// Predeclaration
class QShowEvent;
class QKeyEvent;
class Keyboard;
class ElfLibrary;
class Model;
class BookInfo;
class SearchItem;
class PowerManagerLock;
class bqDeviceServices;
class bqDeviceServicesClientBooksList;
class QBookApp;
class SearchItemShop;
class QFileInfo;

class Search : public QBookForm, protected Ui::Search
{
    Q_OBJECT

public:
                                        Search                          (QWidget*);
    virtual                             ~Search                         ();

#ifndef HACKERS_EDITION
    bqDeviceServicesClientBooksList*    searchInService                 ( const QString& );
#endif
    void                                searchInLibrary                 ( const QString& );
    void                                init                            ();
    void                                deactivateForm                  ();
    void                                activateForm                    ();

public slots:
    void                                handleKeyboard                  ();

protected slots:
    void                                clearSearchLine                 ();
    void                                handleClose                     ();
    void                                handleSearch                    ();
    void                                openPath                        ( const QString& );
#ifndef HACKERS_EDITION
    void                                openStore                       ( const BookInfo* );
    void                                openServiceSearch               ();
#endif
    void                                keyboardPressedChar             (const QString &);
    void                                handleClicked                   ();
    void                                handleSearchFinished            ();
    void                                removeResults                   ();

protected:
    virtual void                        keyReleaseEvent                 (QKeyEvent*);
    PowerManagerLock*                   m_powerLock;

    /* http://qt-project.org/forums/viewthread/7340 */
    void                                paintEvent                      (QPaintEvent *);
private:
    Keyboard*                           keyboard;

    QList<const QFileInfo*>             searchResult;
    bqDeviceServicesClientBooksList*    remoteSearchResult;
    void                                show();
    QList<SearchItem*>                  books;
    QList<SearchItemShop*>              shopBooks;
    int                                 page;
    int                                 i_searchSize;
    int                                 getTotalPages                   ();
    Model*                              model;
    bool                                m_userTyped;
    void                                searchResults                   ();
    QFuture<void>                       future;

    QMutex                              m_searchMutex;
    QFutureWatcher<void>                m_futureWatcher;
    QString                             qs_lastSearch;
    bool                                m_abortSearch;
    QList<const QFileInfo*>             m_files;
    QList<const QFileInfo*>             m_dirs;
    QList<const BookInfo*>              m_books;
    QList<QString>                      m_images;

private slots:
    void                                forward                         ();
    void                                back                            ();
    void                                first                           ();
    void                                last                            ();
    void                                deleteData                      ();
    void                                searchInPath                    ( const QString&, const QString& );
};

#endif // BQSEARCHLIBRARY_H
