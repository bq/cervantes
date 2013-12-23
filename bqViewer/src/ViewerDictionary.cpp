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

#include "ViewerDictionary.h"

#include "Viewer.h"
#include "Dictionary.h"
#include "Screen.h"
#include "QBook.h"
#include "QBookApp.h"
#include "bqUtils.h"
#include "DictionaryLayer.h"

#include <QKeyEvent>
#include <QScrollBar>
#include <QAbstractSlider>
#include <QDebug>
#include <QPoint>
#include <QStringList>
#include <QPainter>

#define PERCENT_STEP_VALUE 0.75

ViewerDictionary::ViewerDictionary(Viewer *parentViewer) :
    QWidget(parentViewer)
  , m_changeDictioWidget(NULL)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);
    m_parentViewer = parentViewer;
    dictionaryDefinitionTextBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vbar = dictionaryDefinitionTextBrowser->verticalScrollBar();
    dictionaryDefinitionTextBrowser->setTappable(true);
    connect(VerticalPagerPopup, SIGNAL(previousPageReq()),  this , SLOT(previousPage()));
    connect(VerticalPagerPopup, SIGNAL(nextPageReq()), this, SLOT(nextPage()));
    connect(closeBtn, SIGNAL(clicked()),this,SLOT(handleCloseBtn()));
    connect(backSearchBtn, SIGNAL(clicked()),this,SLOT(handleBackSearchBtn()));
    connect(changeDictionaryBtn, SIGNAL(clicked()), this, SLOT(showChangeDictionaryWidget()));
    connect(dictionaryDefinitionTextBrowser, SIGNAL(wordClicked(const QString&, const QString&)), this, SLOT(wordToSearch(const QString&, const QString&)), Qt::UniqueConnection);

    topArrowLbl->hide();
    backSearchBtn->hide();

    m_changeDictioWidget = new DictionaryLayer(this);
    m_changeDictioWidget->hide();
    connect(m_changeDictioWidget, SIGNAL(dictionaryChanged(const QString& )), this, SLOT(dictionaryChanged(const QString&)));

    hide();
}

ViewerDictionary::~ViewerDictionary()
{
    qDebug() << Q_FUNC_INFO;

    delete m_changeDictioWidget;
    m_changeDictioWidget = NULL;
}

void ViewerDictionary::dictioSearch( const QStringList& searchList )
{
    qDebug() << Q_FUNC_INFO;

    m_lastSearch = searchList;
    m_searchResult.clear();
    m_searchTerm.clear();
    m_searchTerm = QString(searchList.at(0));
    m_context = QString(searchList.at(1)) + QString("<w>") + QString(searchList.at(2));
    m_searchResult = Dictionary::instance()->translate(m_searchTerm, m_context);
    showSearch(m_searchTerm);
}

void ViewerDictionary::searchAgain()
{
    if (m_lastSearch.size() > 0)
        dictioSearch(m_lastSearch);
}

void ViewerDictionary::showSearch( const QString& searchedWord )
{
    qDebug() << Q_FUNC_INFO << ", searchedWord" << searchedWord;
    QString dictionary = QBook::settings().value("setting/dictionary", "").toString();
    changeDictionaryBtn->setText(getDictionaryName(dictionary));
    dictionaryDefinitionTextBrowser->clear();
    dictionaryDefinitionTextBrowser->applyDocument(m_searchResult);

    Screen::getInstance()->queueUpdates();
    if (m_changeDictioWidget)
        m_changeDictioWidget->hide();

    m_parentViewer->hideAllElements();
    m_parentViewer->showElement(this);
    this->setFocus();
    //Define a single step as the 75% of the normal height in the widget.
    vbar->setSingleStep(dictionaryDefinitionTextBrowser->height()*PERCENT_STEP_VALUE);
    setupPagination();
    move((m_parentViewer->width() - this->width()) >> 1, (m_parentViewer->height() - this->height()) >> 1);// NOTE Move the widget in the middle of the screen
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void ViewerDictionary::previousPage()
{
    Screen::getInstance()->queueUpdates();
    if (vbar)
    {
        vbar->triggerAction(QAbstractSlider::SliderSingleStepSub);
    }
    Screen::getInstance()->setMode(Screen::MODE_QUICK, true, FLAG_WAITFORCOMPLETION, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void ViewerDictionary::nextPage()
{
    Screen::getInstance()->queueUpdates();
    if (vbar)
    {
        vbar->triggerAction(QAbstractSlider::SliderSingleStepAdd);
    }
    Screen::getInstance()->setMode(Screen::MODE_QUICK, true, FLAG_WAITFORCOMPLETION, Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE, true);
    Screen::getInstance()->flushUpdates();
}

void ViewerDictionary::setupPagination()
{
    qDebug() << Q_FUNC_INFO;

    int pageNumber;
    if(vbar->maximum() % vbar->singleStep() == 0)
        pageNumber = vbar->maximum() / vbar->singleStep() + 1;
    else
        pageNumber = vbar->maximum() / vbar->singleStep() + 2;
    VerticalPagerPopup->setup(pageNumber, 1, false);
    VerticalPagerPopup->hideLabel();// NOTE: We do not want the labels to be shown.
}

void ViewerDictionary::showChangeDictionaryWidget()
{
    qDebug() << Q_FUNC_INFO;

    if (m_changeDictioWidget->isVisible())
    {
        m_changeDictioWidget->hide();
    }
    else
    {
        m_changeDictioWidget->init();
        m_changeDictioWidget->paint();

        QPoint pos(changeDictionaryBtn->mapToGlobal(QPoint(0,0)));
        pos.setY(pos.y() + changeDictionaryBtn->height());
        m_changeDictioWidget->move(mapFromGlobal(pos));
        //m_changeDictioWidget->resize(changeDictionaryBtn->width(), m_changeDictioWidget->height());

        m_changeDictioWidget->show();
    }
}

void ViewerDictionary::dictionaryChanged( const QString& newdictionary)
{
    Screen::getInstance()->queueUpdates();

    m_changeDictioWidget->hide();

    searchAgain();
    changeDictionaryBtn->setText(getDictionaryName(newdictionary));
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void ViewerDictionary::handleCloseBtn()
{
    qDebug() << Q_FUNC_INFO;
    if (m_changeDictioWidget && m_changeDictioWidget->isVisible())
        m_changeDictioWidget->hide();

    clearSearch();

    emit hideMe();
}

void ViewerDictionary::wordToSearch(const QString& word, const QString& context)
{
    qDebug() << Q_FUNC_INFO << "word: " << word << ", m_searchTerm: " << m_searchTerm;

    if (m_changeDictioWidget)
        m_changeDictioWidget->hide();
    if(!QBookApp::instance()->isActivated() && !QBookApp::instance()->isLinked()) return;

    if(word == m_searchTerm) return;

    // We do not call to dictioSearch to avoid hiding the dialog and the repositioning.
    if(!m_searchTerm.isEmpty())
    {
        mIterativeSearchHistory.append(m_searchTerm);
        mIterativeContextSearchHistory.append(m_context);
    }

    Screen::getInstance()->queueUpdates();

    if(mIterativeSearchHistory.size())
        backSearchBtn->show();
    else
        backSearchBtn->hide();

    m_lastSearch.clear();
    m_lastSearch << word << "" << "";
    m_searchResult.clear();
    m_searchTerm = word;
    m_context = context;
    m_searchResult = Dictionary::instance()->translate(m_searchTerm, m_context);

    dictionaryDefinitionTextBrowser->clear();
    //sectionTitle->setText(m_searchTerm);
    dictionaryDefinitionTextBrowser->applyDocument(m_searchResult);
    setupPagination();
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void ViewerDictionary::handleBackSearchBtn()
{
    if(!mIterativeSearchHistory.isEmpty())
    {
        // Clear the last search
        m_searchTerm.clear();
        // Extract the previous search
        QString m_lastSearch = mIterativeSearchHistory.takeLast();
        QString m_lastContext = mIterativeContextSearchHistory.takeLast();
        qDebug() << Q_FUNC_INFO << "m_lastSearch: " << m_lastSearch << m_lastContext;
        Screen::getInstance()->queueUpdates();
        wordToSearch(m_lastSearch, m_lastContext);

        if(mIterativeSearchHistory.isEmpty())
        {
            backSearchBtn->hide();
        }
        Screen::getInstance()->flushUpdates();
    }
}

int ViewerDictionary::getDictionaryPosition(const QString& codigo) const
{
    qDebug() << Q_FUNC_INFO << "Dictionary code: " << codigo;

    if(codigo.compare("000200") == 0)
        return 1;
    else if(codigo.compare("000300") == 0)
        return 4;
    else if(codigo.compare("000400") == 0)
        return 2;
    else if(codigo.compare("001000") == 0)
        return 0;
    else if(codigo.compare("001300") == 0)
        return 3;
    return 0;
}

QString ViewerDictionary::getDictionaryName(const QString& codigo) const
{
    qDebug() << Q_FUNC_INFO << "Dictionary code: " << codigo;

    if(codigo == "000200" || codigo == "138")
        return QString("English definitions");
    else if(codigo == "001000" || codigo == "354")
        return QString("Definiciones Espa%1ol").arg(QChar(241));
    else if(codigo == "000300" || codigo == "111")
        return QString("Semantix Ingl%1s - Espa%2ol").arg(QChar(233)).arg(QChar(241));
    else if(codigo == "000400" || codigo == "112")
        return QString("Spanish - English by Semantix");
    else if(codigo == "224")
        return QString("Definicions Catal%1").arg(QChar(225));
    else if(codigo.compare("0") == 0 || !codigo.size())
        return tr("Automatico");
    else
        return tr("Change Dictionary");
}

void ViewerDictionary::setInitialSearch(const QString & word , const QString & context)
{
    qDebug() << Q_FUNC_INFO << word << context;

    clearSearch();

    m_lastSearch << word << context;
    m_searchResult.clear();
    m_searchTerm = word;
    m_context = context;

    mIterativeSearchHistory.append(m_searchTerm);
    mIterativeContextSearchHistory.append(m_context);
    if(mIterativeSearchHistory.size())
    {
        backSearchBtn->show();
    }
}

void ViewerDictionary::resetInitialSearch()
{
    handleBackSearchBtn();
}

void ViewerDictionary::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}


void ViewerDictionary::clearSearch()
{
    qDebug() << Q_FUNC_INFO;

    mIterativeSearchHistory.clear();
    mIterativeContextSearchHistory.clear();
    backSearchBtn->hide();

    m_lastSearch.clear();
}


void ViewerDictionary::mousePressEvent( QMouseEvent* event )
{
    qDebug() << Q_FUNC_INFO;

    event->accept();

}
