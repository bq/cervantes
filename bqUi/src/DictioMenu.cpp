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

#include "DictioMenu.h"

#include "bqViewer.h"
#include "bqUtils.h"

#include <QDebug>

DictioMenu::DictioMenu (QWidget *parent): ContextMenu(parent) {
    qDebug() << "--->" << Q_FUNC_INFO;

    Ui::bqSelectedWordMenu::setupUi(this);
}

DictioMenu::~DictioMenu() {
        /// Do nothing
        qDebug() << "--->" << Q_FUNC_INFO;
}

void DictioMenu::setup(const QString& result, const QPoint& hiliPos, const QRect& bbox){
    qDebug() << Q_FUNC_INFO << "Result: " << result << ", hiliPos: " << hiliPos << ", bbox: " << bbox;

    Definition->clear();
    Definition->applyDocument(result);

    connect(Definition, SIGNAL(wordClicked(const QString&)), SIGNAL(wordToSearch(const QString&)), Qt::UniqueConnection);
    connect(viewFullDef,SIGNAL(clicked()),&m_btnSignalsMapper,SLOT(map()), Qt::UniqueConnection);
    m_btnSignalsMapper.setMapping(viewFullDef,ContextMenu::DICTIOSEARCH);
    if(((bqViewer*)parent())->getCurrentDocExt() == bqViewer::EXT_TXT)
        m_btnSignalsMapper.removeMappings(viewFullDef);

    connect(searchTextBtn,SIGNAL(clicked()),&m_btnSignalsMapper,SLOT(map()), Qt::UniqueConnection);
    m_btnSignalsMapper.setMapping(searchTextBtn,ContextMenu::TEXTSEARCH);

    connect(createNoteBtn,SIGNAL(clicked()),&m_btnSignalsMapper,SLOT(map()), Qt::UniqueConnection);
    m_btnSignalsMapper.setMapping(createNoteBtn,ContextMenu::CREATE_NOTE);

    connect(highlightedBtn,SIGNAL(clicked()),&m_btnSignalsMapper,SLOT(map()), Qt::UniqueConnection);
    m_btnSignalsMapper.setMapping(highlightedBtn,ContextMenu::CREATE_HIGHLIGHT);

    connect(&m_btnSignalsMapper,SIGNAL(mapped(int)),SIGNAL(pressedButton(int)), Qt::UniqueConnection);

    // Place the menu
    int mpx = (600 - width())>>1; // NOTE: (600 - width())/2
    int mpy;
    if(hiliPos.y() > 400){ // Lower half
        arrow_top->hide();
        arrow_bottom->show();
        int arrowX = bbox.left() - mpx - (arrow_bottom->width()>>1);
        arrow_bottom->move(arrowX, arrow_bottom->y());
        mpy = bbox.top() - height() + arrow_bottom->height() - 4;
    }
    else{ // Upper half
        arrow_bottom->hide();
        arrow_top->show();
        int arrowX = bbox.right() - mpx - (arrow_bottom->width()>>1);
        arrow_top->move(arrowX, arrow_top->y());
        mpy = bbox.bottom();
    }
    move(mpx,mpy);
}
