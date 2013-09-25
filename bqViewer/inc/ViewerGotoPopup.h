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

#ifndef VIEWERGOTOPOPUP_H
#define VIEWERGOTOPOPUP_H

#include "ViewerMenuPopUp.h"
#include "ui_ViewerGotoPopup.h"
#include <QString>

class Viewer;
class QDocView;

class ViewerGotoPopup : public ViewerMenuPopUp,
                        protected Ui::ViewerGotoPopup
{
    Q_OBJECT

public:
    ViewerGotoPopup( Viewer* viewer );
    virtual ~ViewerGotoPopup();

    virtual void                                setup                                                   ();
    virtual void                                start                                                   ();
    virtual void                                stop                                                    ();

    void                                        setCurrentDoc                                           ( QDocView*, const QString& );

protected:
//    virtual void                                showEvent                                               (QShowEvent*);
//    virtual void                                paintEvent                                              ( QPaintEvent* );

public slots:
    void                                        clearLineEdit                                           ();

signals:
    void                                        goPage                                                (int);

private slots:
    void                                        goToBeggining                                           ();
    void                                        goToEnd                                                 ();
    void                                        goToPage                                                (int);
    void                                        handleNumber                                            ();
    void                                        handlePage                                              ();
    void                                        handleDeleteOne                                         ();
};

#endif // VIEWERGOTOPOPUP_H
