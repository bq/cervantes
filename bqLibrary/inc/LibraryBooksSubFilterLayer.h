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

#ifndef LIBRARYBOOKSSUBFILTERLAYER_H
#define LIBRARYBOOKSSUBFILTERLAYER_H

#include <QWidget>
#include "ui_LibraryBooksSubFilterLayer.h"

// Predeclaration
class QButtonGroup;

class LibraryBooksSubFilterLayer : public QWidget, public Ui::LibraryBooksSubFilterLayer
{
    Q_OBJECT

public:
                                        LibraryBooksSubFilterLayer          (QWidget* parent);
    virtual                             ~LibraryBooksSubFilterLayer         ();

    void                                setAllBooksChecked                  ();

    void                                setActiveBooksEnabled               ( bool enabled );
    void                                setReadBooksEnabled                 ( bool enabled );
    void                                setNewBooksEnabled                  ( bool enabled );
    void                                setSampleBooksEnabled               ( bool enabled );
    void                                setPurchasedBooksEnabled            ( bool enabled );
    void                                setSubscriptionBooksEnabled         ( bool enabled );
    void                                setSubscriptionBooksShown           ( bool shown );
    void                                setStoreArchivedBooksEnabled        ( bool enabled );
    void                                setPremiumArchivedBooksEnabled      ( bool enabled );

    void                                setAllBooksNumber                   ( int size );
    void                                setNewBooksNumber                   ( int size );
    void                                setActiveBooksNumber                ( int size );
    void                                setReadBooksNumber                  ( int size );
    void                                setSampleBooksNumber                ( int size );
    void                                setPurchasedBooksNumber             ( int size );
    void                                setSubscriptionBooksNumber          ( int size );
    void                                setStoreArchivedBooksNumber         ( int size );
    void                                setPremiumArchivedBooksNumber       ( int size );

    QString                             getAllBooksName                     () const;
    QString                             getNewBooksName                     () const;
    QString                             getActiveBooksName                  () const;
    QString                             getReadBooksName                    () const;
    QString                             getSampleBooksName                  () const;
    QString                             getPurchasedBooksName               () const;
    QString                             getSubscriptionBooksName            () const;
    QString                             getStoreArchivedBooksName           () const;
    QString                             getPremiumArchivedBooksName         () const;

signals:
    void                                allBooksSelected                    ();
    void                                newBooksSelected                    ();
    void                                activeBooksSelected                 ();
    void                                readBooksSelected                   ();
    void                                sampleBooksSelected                 ();
    void                                purchasedBooksSelected              ();
    void                                subscriptionBooksSelected           ();
    void                                archivedStoreBooksSelected          ();
    void                                archivedPremiumBooksSelected        ();

private slots:
    void                                allBooksClicked                     ();
    void                                newBooksClicked                     ();
    void                                activeBooksClicked                  ();
    void                                readBooksClicked                    ();
    void                                sampleBooksClicked                  ();
    void                                subscriptionBooksClicked            ();
    void                                purchasedBooksClicked               ();
    void                                archivedStoreBooksClicked           ();
    void                                archivedPremiumBooksClicked         ();

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    virtual void                        paintEvent                          ( QPaintEvent* );

    QString                             m_allBooksName;
    QString                             m_newBooksName;
    QString                             m_activeBooksName;
    QString                             m_readBooksName;
    QString                             m_sampleBooksName;
    QString                             m_subscriptionBooksName;
    QString                             m_purchasedBooksName;
    QString                             m_archivedStoreBooksName;
    QString                             m_archivedPremiumBooksName;
};

#endif // LIBRARYBOOKSSUBFILTERLAYER_H
