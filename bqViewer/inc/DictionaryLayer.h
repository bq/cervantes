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

#ifndef DICTIONARYLAYER_H
#define DICTIONARYLAYER_H

#include "PopUp.h"
#include "ui_DictionaryLayer.h"
#include "Dictionary.h"

#include <QList>

class DictionaryLayer : public PopUp, protected Ui::DictionaryLayer
{
    Q_OBJECT

public:
    DictionaryLayer(QWidget *);
    virtual ~DictionaryLayer();

    void                                init                                ();

    QString                             objNameToObjText                    ( const QString& );

public slots:
    void                                paint                               ();
    void                                handleChangeDictionary              ( int );

protected:
    virtual void                        paintEvent                          ( QPaintEvent* event );

    void                                changeDictionary                    ( const QString& );

    QButtonGroup*                       m_buttonGroup;
    QList<DictionaryParams>             m_dictionaryList;

    int                                 m_dictionarySelected;

protected slots:
    void                                previousPage                        ();
    void                                nextPage                            ();
    void                                firstPage                           ();
    void                                lastPage                            ();

signals:
    void                                dictionaryChanged                   ( const QString& );

private:
    int                                 m_page;
    int                                 m_totalPages;
};

#endif // DICTIONARYLAYER_H
