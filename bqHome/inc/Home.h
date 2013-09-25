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

#ifndef HOME_H
#define HOME_H

#include "ui_Home.h"
#include "QBookForm.h"
#include "QSize"

#define USERGUIDEPATH "/app/share/userGuides/"


class BookInfo;
class HomeBooksListOptions;
class QBookApp;
class QWidget;
class bqDeviceServicesClientBooksList;
class bqDeviceServicesCategoryObject;

class Home : public QBookForm, protected Ui::Home
{
    Q_OBJECT


public:

    enum VIEW   {
        RECENT,
        CATEGORY1,
        CATEGORY2,
        CATEGORY3,
        NONE
    };

                                        Home                                (QWidget* parent = 0);
    virtual                             ~Home                               ();
    void                                activateForm                        ();
    void                                deactivateForm                      ();

    static QString                      resizeTextForRecent                 (QString, bool);
    static QString                      getDateStyled                       ( const QDateTime&, bool);
    void                                setupHome                           ( );
    void                                initHomeList                        ();
    QSize                               getHomeThumbnailSize                ();

private:

    BookInfo*                           defaultHelpBook;
    const BookInfo*                     currentBook;
    VIEW                                currentView;

    void                                setupBooksList                      ();
    void                                setMostRecentBook                   (const BookInfo*);
    void                                setupRecentBooks                    ( );
    QString                             resizeTextForLastBook               ( const QString& );
    void                                setNewHomeBooks                     ( );
    int                                 getCategoryId                       ( );

    QList<const BookInfo*>              recentBooks;
    bqDeviceServicesCategoryObject*     m_category1;
    bqDeviceServicesCategoryObject*     m_category2;
    bqDeviceServicesCategoryObject*     m_category3;
    bqDeviceServicesCategoryObject*     memoryCategory1List;
    bqDeviceServicesCategoryObject*     memoryCategory2List;
    bqDeviceServicesCategoryObject*     memoryCategory3List;
    QHash<QString,int>*                 homeBookList;
    HomeBooksListOptions*               homeBooksListOptions;
    QList<VIEW>                         optionsAvailables;
    bool                                newHomeBooksReceived();

signals:

    void                                openStoreReq                        (QString);
    void                                activateWifiButton                  ();
    void                                openContentSignal                   (const BookInfo*);
    void                                viewRecentBooks                     ();

private slots:

    void                                goToLibrary                         ();
    void                                viewAll                             ();
    void                                goToHelp                            ();
    void                                goToSearch                          ();
    void                                goToSettings                        ();
    void                                openBook                            (const BookInfo*);
    void                                openMostRecentBook                  ();
    void                                viewSelected                        (Home::VIEW);

public slots:

    void                                getHomeBooksFinished                ();
    void                                doAddHomeBook                       (const QVariantMap &data, int type, int id, const QString& name);
    void                                modelChanged                        (QString, int);
    void                                showOptions                         ();

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    void                                paintEvent                          ( QPaintEvent *);
    void                                mouseReleaseEvent                   ( QMouseEvent * );
    void                                showEvent                           ( QShowEvent * ) ;
    bool                                b_isMostRecentFromSD;
};

#endif // ELFHOME_H
