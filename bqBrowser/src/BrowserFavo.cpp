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

#include "BrowserFavo.h"

#include "BrowserFavoModel.h"
#include "BrowserFavoItem.h"
#include "QBook.h"
#include "SelectionDialog.h"
#include "ConfirmDialog.h"
#include "Screen.h"

#include <QKeyEvent>
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQuery>

#define ITEMS_PER_PAGE 8
bool BrowserFavo::isSelectAll = true;

BrowserFavo::BrowserFavo(QWidget *parent)
        : FullScreenWidget(parent)
        , addToEmptyDate(false)
{
    BrowserFavo::isSelectAll = true;
    setupUi(this);

    items.append(Item1);
    items.append(Item2);
    items.append(Item3);
    items.append(Item4);
    items.append(Item5);
    items.append(Item6);
    items.append(Item7);
    items.append(Item8);

    connect(closeBtn, SIGNAL(pressed()), this, SIGNAL(hideMe()));
    connect(deleteBtn,SIGNAL(clicked()), this, SLOT(deleteBtnClicked()));
    connect(selectBtn,SIGNAL(clicked()), this, SLOT(selectBtnClicked()));
    connect(browserPagerCont, SIGNAL(nextPagePressed()),     this, SLOT(nextPage()));
    connect(browserPagerCont, SIGNAL(previousPagePressed()), this, SLOT(previousPage()));

    for(int i=0; i < items.size(); i++)
    {
        connect(items.at(i),SIGNAL(handleTap(QString)),this,SLOT(handlemyfavo(QString)));
        connect(items.at(i),SIGNAL(select()),this,SLOT(handlselectbtn()));
    }
    QFile fileSpecific(":/res/browser_styles.qss");
    QFile fileCommons(":/res/browser_styles_generic.qss");
    fileSpecific.open(QFile::ReadOnly);
    fileCommons.open(QFile::ReadOnly);

    QString styles = QLatin1String(fileSpecific.readAll() + fileCommons.readAll());
    setStyleSheet(styles);
    selectBtn->setChecked(true);
}

BrowserFavo::~BrowserFavo()
{
    qDebug() << Q_FUNC_INFO;
}

void BrowserFavo::handlemyfavo(QString url)
{
    qDebug() << Q_FUNC_INFO << url;
    if(url!="") {
        emit myfavoclick(url);
        emit hideMe();
    }
}

void BrowserFavo::handlselectbtn(){
	countSelect();
}

/*-------------------------------------------------------------------------*/

void BrowserFavo::deleteBtnClicked()
{
    qDebug() << Q_FUNC_INFO;
    if(countSelect()>0) {
        SelectionDialog* dialogSelect = new SelectionDialog(this,tr("Are you sure?"));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        dialogSelect->exec();
        if(dialogSelect->result())
            m_model->deleteSelect();
        delete dialogSelect;
        countSelect();
        emit hideMe();
    }
}

int BrowserFavo::countSelect() const
{
    qDebug() << Q_FUNC_INFO;
    QBookWebFavoInfo newImage;
    int count = 0;
    int pageOffset = m_page*ITEMS_PER_PAGE;
    for(int i = 0; i < (m_model->listCount() < items.size() + pageOffset ? m_model->listCount() - pageOffset: items.size()); i++)
    {
        int pos = pageOffset + i;
        newImage = m_model->getInfoData(pos);
        newImage.check = items.at(i)->isChecked();
        m_model->replace(pos,newImage);
        if(newImage.check)
            count++;
    }

    if(count == 0){
        selectBtn->setText(tr("Select All"));
        BrowserFavo::isSelectAll = true;
    }else{
        selectBtn->setText(tr("Deselect All"));
        BrowserFavo::isSelectAll = false;
    }

    selectBtn->setChecked(isSelectAll);
    return count;
}

void BrowserFavo::selectBtnClicked()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->queueUpdates();
    if (BrowserFavo::isSelectAll)
    {
        BrowserFavo::isSelectAll = false;
        selectBtn->setText(tr("Deselect All"));
        int pageOffset = m_page*ITEMS_PER_PAGE;
        for(int i = 0; i < (m_model->listCount() < items.size() + pageOffset ? m_model->listCount() - pageOffset: items.size()); i++)
        {
            items.at(i)->setChecked(true);
        }
    }
    else
    {
        BrowserFavo::isSelectAll = true;
        selectBtn->setText(tr("Select All"));
        int pageOffset = m_page*ITEMS_PER_PAGE;
        for(int i = 0; i < (m_model->listCount() < items.size() + pageOffset ? m_model->listCount() - pageOffset: items.size()); i++)
        {
            items.at(i)->setChecked(false);
        }
    }
    selectBtn->setChecked(isSelectAll);
    repaint();
    countSelect();
    Screen::getInstance()->setMode(Screen::MODE_QUICK, true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void BrowserFavo::setFavoItems()
{
    qDebug() << Q_FUNC_INFO << "size: " << m_model->listCount();
    QBookWebFavoInfo item;
    int pageOffset = m_page*ITEMS_PER_PAGE;
    for(int i = 0; i < (m_model->listCount() < items.size() + pageOffset ? m_model->listCount() - pageOffset: items.size()); i++)
    {
        int pos = pageOffset + i;
        item = m_model->getInfoData(pos);
        items.at(i)->setItem(item.eventLabel, item.eventUrl);
    }
    for(int j = m_model->listCount(); j < items.size() + pageOffset; j++)
    {
        items.at(j-pageOffset)->hideItem();
    }
    browserPagerCont->setPage(m_page, m_totalPages+1);
}

void BrowserFavo::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }

void BrowserFavo::nextPage()
{
    qDebug() << Q_FUNC_INFO;
    if(m_page < m_totalPages)
    {
        m_page++;
        setFavoItems();
        countSelect();
    }
    browserPagerCont->setPage(m_page, m_totalPages+1);
}

void BrowserFavo::previousPage()
{
    qDebug() << Q_FUNC_INFO;
    if(m_page > 0)
    {
        m_page--;
        setFavoItems();
        countSelect();
    }
    browserPagerCont->setPage(m_page, m_totalPages+1);
}

bool BrowserFavo::isAdded(QString& url)
{
    qDebug() << Q_FUNC_INFO;
    QBookWebFavoInfo newImage;
    bool isAdded = false;
    for(int i = 0; i < m_model->listCount(); i++)
    {
        newImage = m_model->getInfoData(i);
        if(newImage.getEventurl() == url)
        {
            isAdded = true;
            break;
        }
    }
    return isAdded;
}

void BrowserFavo::getModel()
{
    qDebug() << Q_FUNC_INFO;
    m_model = BrowserFavoModel::getModel();
}

void BrowserFavo::setup()
{
    qDebug() << Q_FUNC_INFO;
    getModel();
    m_page = 0;
    m_totalPages = (m_model->listCount() - 1)/ items.size();
    setFavoItems();
    countSelect();
}

void BrowserFavo::addFavo(QString& url, QString& s)
{
    qDebug() << Q_FUNC_INFO;
    getModel();
    if(isAdded(url))
    {
        ConfirmDialog* dialog = new ConfirmDialog(this,tr("The page was already present in your favorite list."));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        dialog->exec();
        delete dialog;
        return;
    }
    else
    {
        m_model->addFavo(url, s);
        ConfirmDialog* dialog2 = new ConfirmDialog(this,tr("Add Favorite Successfully"));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        dialog2->exec();
        delete dialog2;
    }
}
