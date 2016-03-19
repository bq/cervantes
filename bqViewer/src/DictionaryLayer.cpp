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


#include "DictionaryLayer.h"
#include "Dictionary.h"

#include "Screen.h"

#include "QBook.h"
#include <QPainter>
#include <QDebug>

const int DICTIOS_PER_PAGE = 5;

DictionaryLayer::DictionaryLayer(QWidget *parent)
    : PopUp(parent)
    , m_dictionarySelected(-1)
    , m_page(0)
{

    setupUi(this);
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(false);
    m_buttonGroup->addButton(dictBtn0,0);
    m_buttonGroup->addButton(dictBtn1,1);
    m_buttonGroup->addButton(dictBtn2,2);
    m_buttonGroup->addButton(dictBtn3,3);
    m_buttonGroup->addButton(dictBtn4,4);

    connect(m_buttonGroup, SIGNAL(buttonClicked(int)),this, SLOT(handleChangeDictionary(int)));

    connect(VerticalPagerPopup,   SIGNAL(previousPageReq()),   this, SLOT(previousPage()));
    connect(VerticalPagerPopup,   SIGNAL(nextPageReq()),       this, SLOT(nextPage()));
    connect(VerticalPagerPopup,   SIGNAL(firstPage()),         this, SLOT(firstPage()));
    connect(VerticalPagerPopup,   SIGNAL(lastPage()),          this, SLOT(lastPage()));
}

DictionaryLayer::~DictionaryLayer()
{
    qDebug() << Q_FUNC_INFO;
}

void DictionaryLayer::init()
{
    QString dictioCode = Dictionary::instance()->getCurrentDictCode();

    qDebug() << Q_FUNC_INFO << "DictioCode: " << dictioCode;

    m_dictionaryList = Dictionary::instance()->parseCurrentDictionaries(QString::fromUtf8(Dictionary::instance()->getJsonDictionaries())).values();
    qDebug() << Q_FUNC_INFO << "m_dictionaryList: " << m_dictionaryList.size();

    int size = m_dictionaryList.size();
    m_page = 0;

    m_totalPages = ((size-1) / DICTIOS_PER_PAGE) + 1;
    VerticalPagerPopup->setup(m_totalPages, m_page+1, true);

    QList<DictionaryParams>::const_iterator it = m_dictionaryList.constBegin();
    QList<DictionaryParams>::const_iterator itEnd = m_dictionaryList.constEnd();
    for(int i = 0; it != itEnd; ++it, ++i)
    {
        qDebug() << Q_FUNC_INFO << "Id: " << (*it).id;

        if((*it).id == dictioCode)
        {
            m_dictionarySelected = i;
            return;
        }
    }
    m_dictionarySelected = 0;
}

QString DictionaryLayer::objNameToObjText( const QString& objName )
{
    qDebug() << Q_FUNC_INFO << " objName" << objName;
    QList<DictionaryParams>::iterator it = m_dictionaryList.begin();
    QList<DictionaryParams>::iterator itEnd = m_dictionaryList.end();
    QString name;

    while(it != itEnd)
    {
        if ((*it).id.compare(objName) == 0)
        {
            name = (*it).shortName;
            break;
        }
        it++;
    }

    if (name.size())
        return name;
    else
        return QString(tr("Change Dictionary"));
}

void DictionaryLayer::paint()
{
    qDebug() << Q_FUNC_INFO;

    QString dictioCode = Dictionary::instance()->getCurrentDictCode();
    qDebug() << Q_FUNC_INFO << "dictioCode: " << dictioCode;

    int size = m_dictionaryList.size();
    int offset = m_page*DICTIOS_PER_PAGE;

    for(int i = 0; i < DICTIOS_PER_PAGE; ++i)
    {
        int pos = offset + i;
        if(pos < size)
        {
            qDebug() << Q_FUNC_INFO << "id: " << m_dictionaryList.at(pos).id;

            if(m_dictionarySelected == pos)
                m_buttonGroup->button(i)->setChecked(true);
            else
                m_buttonGroup->button(i)->setChecked(false);
            m_buttonGroup->button(i)->setText(m_dictionaryList.at(pos).commercialName);
            m_buttonGroup->button(i)->show();
        }else
            m_buttonGroup->button(i)->hide();
    }
}

void DictionaryLayer::handleChangeDictionary( int buttonId )
{
    qDebug() << Q_FUNC_INFO << "dictio selected: " << m_dictionarySelected << ", buttonId: " << buttonId;

    int newDictionarySelected = (m_page*DICTIOS_PER_PAGE) + buttonId;

    if(m_dictionarySelected != newDictionarySelected)
    {
        m_buttonGroup->button(m_dictionarySelected%DICTIOS_PER_PAGE)->setChecked(false);
        m_buttonGroup->button(buttonId)->setChecked(true);
        m_dictionarySelected = newDictionarySelected;
        changeDictionary(m_dictionaryList.at(m_dictionarySelected).id);
    }
}

void DictionaryLayer::changeDictionary(const QString& dictionary)
{
    qDebug() << Q_FUNC_INFO << "Changing Dictionary to " << dictionary;
    QBook::settings().setValue("setting/dictionary", dictionary);
    QBook::settings().sync();
    emit dictionaryChanged(dictionary);
}

void DictionaryLayer::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void DictionaryLayer::firstPage()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();

    m_page = 0;
    VerticalPagerPopup->setPage();
    paint();

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void DictionaryLayer::lastPage()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();

    m_page = m_totalPages - 1;
    VerticalPagerPopup->setPage();
    paint();

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void DictionaryLayer::nextPage()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();

    m_page++;
    VerticalPagerPopup->setPage();
    paint();

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void DictionaryLayer::previousPage()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();

    m_page--;
    VerticalPagerPopup->setPage();
    paint();

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}
