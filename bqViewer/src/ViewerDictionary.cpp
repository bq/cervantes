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
#include "bqPublicServices.h"
#include "InfoDialog.h"
#include "SelectionDialog.h"
#include "MetaDataExtractor.h"

#include <QKeyEvent>
#include <QScrollBar>
#include <QAbstractSlider>
#include <QDebug>
#include <QPoint>
#include <QStringList>
#include <QPainter>

#define PERCENT_STEP_VALUE              0.75

#define EDIT_NODE               QString("<span class=\"mw-editsection\">")
#define EDIT_NODE_OPEN_TAG      QString("<span ")
#define EDIT_NODE_CLOSE_TAG     QString("</span>")
#define IMAG_NODE               QString("<div class=\"thumb tright\">")
#define IMAG_NODE_OPEN_TAG      QString("<div ")
#define IMAG_NODE_CLOSE_TAG     QString("</div>")

ViewerDictionary::ViewerDictionary(Viewer *parentViewer) :
    QWidget(parentViewer)
  , m_changeDictioWidget(NULL)
  , b_askingForWikipediaInfo(false)
  , m_sectionsDialog(NULL)
  , state(DICTIO_NONE)
  , prevState(DICTIO_NONE)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);
    m_parentViewer = parentViewer;
    dictionaryDefinitionTextBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    dictionaryDefinitionTextBrowser->setTappable(true);
    vbar = dictionaryDefinitionTextBrowser->verticalScrollBar();

    connect(QBookApp::instance(), SIGNAL(goingHome()),       this, SLOT(resetDictionary()));
    connect(QBookApp::instance(), SIGNAL(startSleep()),      this, SLOT(resetDictionary()));
    connect(VerticalPagerPopup,   SIGNAL(previousPageReq()), this, SLOT(previousPage()));
    connect(VerticalPagerPopup,   SIGNAL(nextPageReq()),     this, SLOT(nextPage()));
    connect(closeBtn,             SIGNAL(clicked()),         this, SLOT(handleCloseBtn()));
    connect(dictionaryBtn,        SIGNAL(clicked()),         this, SLOT(handleDictionaryBtn()));
    connect(wikipediaBtn,         SIGNAL(clicked()),         this, SLOT(handleWikipediaBtn()));
    connect(backSearchBtn,        SIGNAL(clicked()),         this, SLOT(handleBackSearchBtn()));
    connect(changeDictionaryBtn,  SIGNAL(clicked()),         this, SLOT(showChangeDictionaryWidget()));
    connect(wikipediaWebBtn,      SIGNAL(clicked()),         this, SLOT(handleWikipediaWebBtn()));

    connect(dictionaryDefinitionTextBrowser, SIGNAL(linkPressed(const QString&)),
            this,                            SLOT(handleLink(const QString&)), Qt::UniqueConnection);

    connect(dictionaryDefinitionTextBrowser, SIGNAL(wordClicked(const QString&, const QString&)),
            this,                            SLOT(wordToSearch(const QString&, const QString&)), Qt::UniqueConnection);


    topArrowLbl->hide();
    showBackBtn(false);
    searchTermLbl->hide();

    m_changeDictioWidget = new DictionaryLayer(this);
    m_changeDictioWidget->hide();
    connect(m_changeDictioWidget, SIGNAL(dictionaryChanged(const QString& )), this, SLOT(dictionaryChanged(const QString&)));

    VerticalPagerPopup->hideLabel();

    if(QBook::getResolution() == QBook::RES758x1024) m_searchTitleMaxWidth = 416;
    else                                             m_searchTitleMaxWidth = 360;

    hide();
}

ViewerDictionary::~ViewerDictionary()
{
    qDebug() << Q_FUNC_INFO;

    if (m_changeDictioWidget) delete m_changeDictioWidget;
    m_changeDictioWidget = NULL;
    if (m_sectionsDialog) delete m_sectionsDialog;
    m_sectionsDialog = NULL;
}

void ViewerDictionary::dictioSearch( const QStringList& searchList )
{
    qDebug() << Q_FUNC_INFO;

    setState(DICTIO_CLASSIC);

    m_lastSearch = searchList;
    m_searchTerm = QString(searchList.at(0));

    if (searchList.size() != 1 || !containsSeveralWords(m_searchTerm))
    {
        if (searchList.size() == 2 && searchList.at(1).contains(QString("<w>")))
            m_context = QString(searchList.at(1));                                              // Search from wikiTerm
        else
            m_context = QString(searchList.at(1)) + QString("<w>") + QString(searchList.at(2)); // Classic search

        m_searchResult = Dictionary::instance()->translate(m_searchTerm, m_context);
    }
    else // Search from wikiSentence
    {
        dictioSearchFromWikiSentence(m_searchTerm);
    }

    if ((m_dictioTermSearchHistory.isEmpty() && m_dictioContextSearchHistory.isEmpty()) ||
        (m_searchTerm != m_dictioTermSearchHistory.last() || m_context != m_dictioContextSearchHistory.last()))
    {
        m_dictioTermSearchHistory.append(m_searchTerm);
        m_dictioContextSearchHistory.append(m_context);
        considerDictioBackButton();
    }

    showDictioSearch(m_searchTerm);
}

void ViewerDictionary::dictioSearchFromWikiSentence(const QString& string)
{
    m_context.clear();
    m_searchResult = "<html><body><p>" + tr("Pulsa sobre la palabra que deseas buscar en el diccionario:") + "</p><ul><b>" + m_searchTerm + "</b></ul><div style=\"clear:both;\"></div><p><br></p><table style=\"\" class=\"noprint plainlinks ambox ambox-notice\"><tr><td class=\"ambox-image\"><div style=\"width:50px;\"><span class=\"mw-mf-image-replacement\"></span></td></tr></table></body></html>";
}

bool ViewerDictionary::containsSeveralWords(const QString& string)
{
    QStringList stringList = string.split(" ", QString::SkipEmptyParts);

    return stringList.size() > 1;
}

void ViewerDictionary::showDictioSearch( const QString& searchedWord )
{
    qDebug() << Q_FUNC_INFO << ", searchedWord" << searchedWord;

    QString dictionary = QBook::settings().value("setting/dictionary", "").toString();
    changeDictionaryBtn->setText(getDictionaryName(dictionary));
    dictionaryDefinitionTextBrowser->clear();
    dictionaryDefinitionTextBrowser->applyDocument(m_searchResult);

    Screen::getInstance()->queueUpdates();


    // TODO: improve and taking into account the state machine
    if (m_changeDictioWidget) m_changeDictioWidget->hide();
    m_parentViewer->hideAllElements();
    changeDictionaryBtn->show();
    m_parentViewer->showElement(this);
    this->setFocus();
    setupPagination();
    move((m_parentViewer->width() - this->width()) >> 1, (m_parentViewer->height() - this->height()) >> 1);// NOTE Move the widget in the middle of the screen

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void ViewerDictionary::dictioSearchAgain()
{
    if (m_lastSearch.size() > 0) dictioSearch(m_lastSearch);
}

void ViewerDictionary::handleLink(const QString& linkUrl)
{
    qDebug() << Q_FUNC_INFO << linkUrl;

    // External link
    if(linkUrl.contains("http") && !linkUrl.contains("wikipedia.org"))
    {
        qDebug() << Q_FUNC_INFO << "No wikipedia URL";

        SelectionDialog* openBrowserDialog = new SelectionDialog(this, tr("El enlace pulsado no pertenece a la wikipedia¿Quieres abrirlo en el navegador?"));
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        Screen::getInstance()->resetQueue();
        openBrowserDialog->exec();
        if(openBrowserDialog->result()) {
            qDebug() << Q_FUNC_INFO << "Opening browser";
            QBookApp::instance()->openBrowser(linkUrl);
        }
        delete openBrowserDialog;
    }
    // Wikipedia link
    else if(linkUrl.contains("/wiki/"))
    {
        qDebug() << Q_FUNC_INFO << "Wikipedia URL";
        QString tokenToSearch = linkUrl.split("/wiki/")[1];
        newWikiSearch(tokenToSearch);
    }
    else
    {
        qDebug() << Q_FUNC_INFO << "Unkonown URL type";
    }
}

void ViewerDictionary::newWikiSearch(const QString& newSearchTerm)
{
    qDebug() << Q_FUNC_INFO << newSearchTerm;

    if (newSearchTerm.isEmpty()) return;

    disconnectWifiObserverWikipedia();
    disconnect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disconnectWifiObserverWikipedia()));
    disconnect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disableSearchigWikipediaFlag()));
    disconnect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(goToViewer()));

    enableSearchigWikipediaFlag();

    if (!goOnline())
    {
        requestConnection();
        return;
    }

    m_currentWikiSearchTerm = newSearchTerm;

    if(b_askingForWikipediaInfo) return;
    b_askingForWikipediaInfo = true;

    QString searchResult;
    dictionaryDefinitionTextBrowser->clear();
    setSearchTitle(m_currentWikiSearchTerm);

    disconnect(QBookApp::instance(), SIGNAL(periodicSyncFinished()), QBookApp::instance()->getViewer(), SLOT(disconnectWifi()));
    disconnect(QBookApp::instance(), SIGNAL(resumingWifiFailed()),   QBookApp::instance()->getViewer(), SLOT(disconnectWifi()));
    QBookApp::instance()->disconnectConnectionManagerSignals();

    getWikipediaSearch(m_currentWikiSearchTerm, &searchResult);

    Screen::getInstance()->queueUpdates();

    // SHOW
    formatWikipediaHtmlInfo(searchResult);
    m_currentWikiText = searchResult;

    dictionaryDefinitionTextBrowser->applyWikipediaInfoStyle(m_currentWikiText);
    setupPagination();

    m_wikiTermSearchHistory.append(m_currentWikiSearchTerm);
    m_wikiTextHistory.append(m_currentWikiText);
    considerWikiBackButton();

    Screen::getInstance()->refreshScreen(Screen::MODE_QUICK);
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

    //Define a single step as the 75% of the normal height in the widget.
    vbar->setSingleStep(dictionaryDefinitionTextBrowser->height()*PERCENT_STEP_VALUE);

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
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();

    m_changeDictioWidget->hide();

    dictioSearchAgain();
    changeDictionaryBtn->setText(getDictionaryName(newdictionary));
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_PARTIALSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void ViewerDictionary::handleCloseBtn()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();

    setState(DICTIO_NONE);
    goOffline();
    emit hideMe();    

    Screen::getInstance()->flushUpdates();
}

void ViewerDictionary::wordToSearch(const QString& word, const QString& context)
{
    qDebug() << Q_FUNC_INFO << "word: " << word << ", m_searchTerm: " << m_searchTerm;

    if (m_changeDictioWidget) m_changeDictioWidget->hide();
    if(!QBookApp::instance()->isActivated() && !QBookApp::instance()->isLinked()) return;

    if(word == m_searchTerm) return;

    Screen::getInstance()->queueUpdates();    

    m_lastSearch.clear();
    m_lastSearch << word << "" << "";
    m_searchResult.clear();
    m_searchTerm = word;
    m_context = context;
    m_searchResult = Dictionary::instance()->translate(m_searchTerm, m_context);

    m_dictioTermSearchHistory.append(m_searchTerm);
    m_dictioContextSearchHistory.append(m_context);
    considerDictioBackButton();

    dictionaryDefinitionTextBrowser->clear();
    dictionaryDefinitionTextBrowser->applyDocument(m_searchResult);
    setupPagination();
    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void ViewerDictionary::considerDictioBackButton()
{
    if (m_dictioTermSearchHistory.size() > 1) showBackBtn();
    else                                      showBackBtn(false);
}

void ViewerDictionary::handleBackSearchBtn()
{
    switch (state)
    {
        case DICTIO_CLASSIC:
            if (m_dictioTermSearchHistory.size() > 1 && m_dictioContextSearchHistory.size() > 1)
            {
                Screen::getInstance()->queueUpdates();

                // Clear the last search
                m_searchTerm.clear();

                // Extract the previous search
                m_dictioTermSearchHistory.removeLast();
                QString m_lastSearch = m_dictioTermSearchHistory.takeLast();
                m_dictioContextSearchHistory.removeLast();
                QString m_lastContext = m_dictioContextSearchHistory.takeLast();

                if (containsSeveralWords(m_lastSearch))
                {
                    m_searchTerm = m_lastSearch;
                    dictioSearchFromWikiSentence(m_searchTerm);
                    showDictioSearch(m_searchResult);
                    considerDictioBackButton();
                    m_dictioTermSearchHistory.append(m_searchTerm);
                    m_dictioContextSearchHistory.append(m_context);
                }
                else
                {
                    wordToSearch(m_lastSearch, m_lastContext);
                }


                Screen::getInstance()->flushUpdates();
            }
        break;

        case DICTIO_WIKIPEDIA:
            if (m_wikiTermSearchHistory.size() > 1 && m_wikiTextHistory.size() > 1)
            {
                Screen::getInstance()->queueUpdates();

                m_wikiTermSearchHistory.removeLast();
                m_currentWikiSearchTerm = m_wikiTermSearchHistory.last();
                setSearchTitle(m_currentWikiSearchTerm);

                m_wikiTextHistory.removeLast();
                m_currentWikiText = m_wikiTextHistory.last();
                dictionaryDefinitionTextBrowser->applyWikipediaInfoStyle(m_currentWikiText);
                setupPagination();
                considerWikiBackButton();

                Screen::getInstance()->flushUpdates();
            }
        break;
    }
}

void ViewerDictionary::handleDictionaryBtn()
{
    qDebug() << Q_FUNC_INFO;

    // Dictionary showed from wikipedia button in textActionsMenu.
    if (state == DICTIO_WIKIPEDIA && prevState == DICTIO_NONE && m_wikiTermSearchHistory.size())
    {
        Screen::getInstance()->queueUpdates();
        setState(DICTIO_CLASSIC);
        QStringList searchFromWiki;
        searchFromWiki << m_wikiTermSearchHistory.first();
        dictioSearch(searchFromWiki);
        Screen::getInstance()->flushUpdates();
        return;
    }

    Screen::getInstance()->queueUpdates();

    if (!setState(DICTIO_CLASSIC)) {Screen::getInstance()->flushUpdates(); return;}

    // Dictionary showed from wikipedia button in textActionsPopup.
    if (m_searchResult.isEmpty() && m_dictioTermSearchHistory.size() && m_dictioContextSearchHistory.size())
    {
        m_searchResult = Dictionary::instance()->translate(m_dictioTermSearchHistory.first(), m_dictioContextSearchHistory.first());
    }

    dictionaryDefinitionTextBrowser->applyDocument(m_searchResult);
    setupPagination();
    considerDictioBackButton();

    Screen::getInstance()->flushUpdates();
}

void ViewerDictionary::goOffline()
{
    if(!QBookApp::instance()->isSynchronizing() && !QBookApp::instance()->isBuying() && !QBookApp::instance()->isResumingWifi())
    {
        QBookApp::instance()->getViewer()->disconnectWifi();
    }
    else
    {
        connect(QBookApp::instance(), SIGNAL(periodicSyncFinished()), QBookApp::instance()->getViewer(), SLOT(disconnectWifi()), Qt::UniqueConnection);
        connect(QBookApp::instance(), SIGNAL(resumingWifiFailed()),   QBookApp::instance()->getViewer(), SLOT(disconnectWifi()), Qt::UniqueConnection);
    }
}

bool ViewerDictionary::goOnline()
{
    if(ConnectionManager::getInstance()->isConnected()) return true;

    PowerManagerLock *mountingLock = PowerManager::getNewLock();
    mountingLock->activate();

    InfoDialog* readingFilesdialog = new InfoDialog(this, tr("Conectando..."));

    connect(QBookApp::instance(), SIGNAL(startSleep()), readingFilesdialog,                SLOT(rejectDialog()), Qt::UniqueConnection);
    connect(readingFilesdialog,   SIGNAL(rejected()),   QBookApp::instance()->getViewer(), SLOT(disconnectWifi()), Qt::UniqueConnection);

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, FLAG_FULLSCREEN_UPDATE, Q_FUNC_INFO);
    readingFilesdialog->show();

    QCoreApplication::flush();
    QCoreApplication::processEvents();

    QBookApp::instance()->resumeWifi();
    bool res =  QBookApp::instance()->waitForConnection();

    if(readingFilesdialog)
    {
        delete readingFilesdialog;
        readingFilesdialog = NULL;
    }
    mountingLock->release();
    delete mountingLock;

    return res;
}

void ViewerDictionary::connectWifiObserverWikipedia()
{
    connect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(goToWikipedia()),Qt::UniqueConnection);
}

void ViewerDictionary::disconnectWifiObserverWikipedia()
{
    disconnect(ConnectionManager::getInstance(), SIGNAL(connected()), this, SLOT(goToWikipedia()));
}

void ViewerDictionary::enableSearchigWikipediaFlag()
{
    m_parentViewer->setSearchigWikipedia(true);
}

void ViewerDictionary::disableSearchigWikipediaFlag()
{
    m_parentViewer->setSearchigWikipedia(false);
}

void ViewerDictionary::goToViewer()
{
    handleCloseBtn();
}

void ViewerDictionary::handleWikipediaWebBtn()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();

    resetDictionary();

    QString lang = QBookApp::instance()->getViewer()->getCurrentBookInfo()->language;
    if(!eBookLangIsOk(lang)) lang = QBook::settings().value("setting/language",QVariant("es")).toString();
    QString wikipediaUrl = QString("http://%1.wikipedia.org/wiki/").arg(lang);
    wikipediaUrl += m_currentWikiSearchTerm;
    QBookApp::instance()->openBrowser(wikipediaUrl);

    Screen::getInstance()->flushUpdates();
}

void ViewerDictionary::wikiSearch(const QString& searchTerm)
{
    qDebug() << Q_FUNC_INFO;

    m_searchTerm = searchTerm;

    m_parentViewer->hideAllElements();
    m_parentViewer->showElement(this);
    this->setFocus();

    handleWikipediaBtn();
}

void ViewerDictionary::requestConnection()
{
    connectWifiObserverWikipedia();
    connect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disconnectWifiObserverWikipedia()),  Qt::UniqueConnection);
    connect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disableSearchigWikipediaFlag()),     Qt::UniqueConnection);
    connect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(goToViewer()),                       Qt::UniqueConnection);

    QBookApp::instance()->requestConnection(true);
}

void ViewerDictionary::handleWikipediaBtn()
{
    if (!setState(DICTIO_WIKIPEDIA)) return;

    // Show old result.
    if (m_currentWikiSearchTerm == m_searchTerm && m_wikiTermSearchHistory.size() && m_wikiTextHistory.size())
    {
        showLastWikiResult();
        return;
    }

    // Show new search.
    if (QBook::settings().value("wifi/disabled",false).toBool()) requestConnection();
    else                                                         goToWikipedia();
}

void ViewerDictionary::goToWikipedia()
{
    qDebug() << Q_FUNC_INFO;    

    disconnectWifiObserverWikipedia();
    disconnect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disconnectWifiObserverWikipedia()));
    disconnect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disableSearchigWikipediaFlag()));
    disconnect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(goToViewer()));

    // To ensure that the dictionary is visible.
    ensureVisibility();

    enableSearchigWikipediaFlag();

    if (!goOnline())
    {
        if (isVisible()) requestConnection(); // If the process not is canceled while runnig goOnline().
        return;
    }

    if (isSameThatLastSearch(m_searchTerm))
    {
        disableSearchigWikipediaFlag();
        return;
    }

    m_currentWikiSearchTerm = m_searchTerm;

    if(b_askingForWikipediaInfo) return;
    b_askingForWikipediaInfo = true;

    QString searchResult;
    changeDictionaryBtn->hide();
    dictionaryDefinitionTextBrowser->clear();
    setSearchTitle(m_currentWikiSearchTerm);
    searchTermLbl->show();

    disconnect(QBookApp::instance(), SIGNAL(periodicSyncFinished()), QBookApp::instance()->getViewer(), SLOT(disconnectWifi()));
    disconnect(QBookApp::instance(), SIGNAL(resumingWifiFailed()),   QBookApp::instance()->getViewer(), SLOT(disconnectWifi()));
    QBookApp::instance()->disconnectConnectionManagerSignals();

    getWikipediaSearch(m_currentWikiSearchTerm, &searchResult);

    Screen::getInstance()->queueUpdates();

    // SHOW
    formatWikipediaHtmlInfo(searchResult);
    m_currentWikiText = searchResult;

    dictionaryDefinitionTextBrowser->applyWikipediaInfoStyle(m_currentWikiText);
    setupPagination();

    m_wikiTermSearchHistory.append(m_currentWikiSearchTerm);
    m_wikiTextHistory.append(m_currentWikiText);
    considerWikiBackButton();

    Screen::getInstance()->refreshScreen(Screen::MODE_QUICK);
    Screen::getInstance()->flushUpdates();
}

void ViewerDictionary::ensureVisibility()
{
    Screen::getInstance()->queueUpdates();

    if (!m_parentViewer->isVisible()) QBookApp::instance()->pushTopForm(m_parentViewer);

    if (!isVisible()) {show(); raise();}

    Screen::getInstance()->flushUpdates();
}

void ViewerDictionary::considerWikiBackButton()
{
    if (m_wikiTermSearchHistory.size() > 1) showBackBtn();
    else                                    showBackBtn(false);
}

void ViewerDictionary::getWikipediaSearch(const QString& searchTerm, QString *searchResult)
{
    qDebug() << Q_FUNC_INFO;

    QBookApp::instance()->connectConnectionManagerSignals();

    prepareProgressDialog();

    QString lang = QBookApp::instance()->getViewer()->getCurrentBookInfo()->language;
    if(!eBookLangIsOk(lang)) lang = QBook::settings().value("setting/language",QVariant("es")).toString();

    *searchResult = QBookApp::instance()->getPublicServices()->getWikipediaWord(lang, searchTerm);

    qDebug() << Q_FUNC_INFO << " searchResult: " << *searchResult;

    disableSearchigWikipediaFlag();
    b_askingForWikipediaInfo = false;
    if (m_sectionsDialog) m_sectionsDialog->hide();
}

void ViewerDictionary::showLastWikiResult()
{
    considerWikiBackButton();
    setSearchTitle(m_currentWikiSearchTerm);
    dictionaryDefinitionTextBrowser->applyWikipediaInfoStyle(m_currentWikiText);
    setupPagination();
}

void ViewerDictionary::formatWikipediaHtmlInfo(QString& html)
{
    qDebug() << Q_FUNC_INFO << html;;

    eraseWikipediaHTMLinfoNode(html, EDIT_NODE, EDIT_NODE_OPEN_TAG, EDIT_NODE_CLOSE_TAG);
    eraseWikipediaHTMLinfoNode(html, IMAG_NODE, IMAG_NODE_OPEN_TAG, IMAG_NODE_CLOSE_TAG);

    html = html.replace("<table", "<!--table");
    html = html.replace("</table", "</table --");

    qDebug() << Q_FUNC_INFO << html;
}

void ViewerDictionary::eraseWikipediaHTMLinfoNode(QString& html, QString node, QString openTag, QString closeTag)
{
    qDebug() << Q_FUNC_INFO;

    while (html.contains(node)) if (!eraseFirstNodeOcurrence(html, node, openTag, closeTag)) break;
}

bool ViewerDictionary::eraseFirstNodeOcurrence(QString& html, QString node, QString openTag, QString closeTag)
{
    int openTagCounter = 0, closeTagCounter = 0;

    int startIDX = html.indexOf(node), endIDX = 0, openIDX = 0, closeIDX = 0;
    if (startIDX == -1) return false;

    closeIDX = startIDX;

    do
    {
        openIDX = closeIDX;
        closeIDX = html.indexOf(closeTag, closeIDX + 1);
        if (closeIDX == -1) return false; // ERROR
        ++closeTagCounter;

        while (openIDX < closeIDX)
        {
            openIDX = html.indexOf(openTag, openIDX + 1);
            if (openIDX == -1) break;
            if (openIDX < closeIDX) ++openTagCounter;
        }
    }
    while (openTagCounter >= closeTagCounter);

    if (openTagCounter < closeTagCounter)
    {
        endIDX = closeIDX + QString(closeTag).size() - 1;
        html.remove(startIDX, endIDX - startIDX + 1);
        return true;
    }
    else
    {
        // ERROR
        return false;
    }
}

void ViewerDictionary::prepareProgressDialog()
{
    qDebug() << Q_FUNC_INFO;

    if (!m_sectionsDialog)
    {
        m_sectionsDialog = new ProgressDialog(this,tr("Obteniendo informacion..."));
        m_sectionsDialog->setTextValue(false);
        m_sectionsDialog->setHideBtn(false);
        connect(QBookApp::instance()->getPublicServices(), SIGNAL(downloadProgress(int)), m_sectionsDialog, SLOT(setProgressBar(int)),      Qt::UniqueConnection);
        connect(m_sectionsDialog,                          SIGNAL(cancel()),              this,             SLOT(cancelWikipediaRequest()), Qt::UniqueConnection);
    }
    m_sectionsDialog->setProgressBar(0);
    m_sectionsDialog->show();
}

void ViewerDictionary::cancelWikipediaRequest()
{
    qDebug() << Q_FUNC_INFO;

    Screen::getInstance()->queueUpdates();

    if (m_sectionsDialog) m_sectionsDialog->hide();
    QBookApp::instance()->getPublicServices()->cancelDownload();
    Screen::getInstance()->refreshScreen(Screen::MODE_QUICK);

    Screen::getInstance()->flushUpdates();
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

    setState(DICTIO_CLASSIC);

    clearSearch();

    m_lastSearch << word << context;
    m_searchResult.clear();
    m_searchTerm = word;
    m_context = context;

    m_dictioTermSearchHistory.append(m_searchTerm);
    m_dictioContextSearchHistory.append(m_context);
}

void ViewerDictionary::resetInitialSearch()
{
    clearSearch();
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

    m_dictioTermSearchHistory.clear();
    m_dictioContextSearchHistory.clear();
    m_wikiTermSearchHistory.clear();
    m_wikiTextHistory.clear();

    showBackBtn(false);

    m_lastSearch.clear();
    m_context.clear();
}

void ViewerDictionary::showBackBtn(bool show)
{
    if (show)
    {
        backSearchBtn->show();
        if (state == DICTIO_WIKIPEDIA) backSearchSpaceLbl->show();
        else backSearchSpaceLbl->hide();
    }
    else
    {
        backSearchBtn->hide();
        backSearchSpaceLbl->hide();
    }
}


void ViewerDictionary::mousePressEvent(QMouseEvent* event)
{
    qDebug() << Q_FUNC_INFO << event->pos();

    event->accept();
}

void ViewerDictionary::showEvent(QShowEvent* event)
{
    qDebug() << Q_FUNC_INFO;

    QWidget::showEvent(event);
}

void ViewerDictionary::hideEvent(QHideEvent* event)
{
    qDebug() << Q_FUNC_INFO;

    QWidget::hideEvent(event);
}

bool ViewerDictionary::setState(DictioState newState)
{
    qDebug() << Q_FUNC_INFO;

    if (newState == state) return false;

    prevState = state;
    state = newState;
    handleState();
    return true;
}

void ViewerDictionary::handleState()
{
    qDebug() << Q_FUNC_INFO;

    switch(state)
    {
        case DICTIO_CLASSIC:
            searchTermLbl->hide();            
            dictionaryDefinitionTextBrowser->clear();
            dictionaryDefinitionTextBrowser->setLinksAccepted(false);
            changeDictionaryBtn->setText(getDictionaryName(QBook::settings().value("setting/dictionary", "").toString()));
            changeDictionaryBtn->show();
            wikipediaWebBtn->hide();
            showBackBtn(false);
            powerOnDictioButton();
        break;

        case DICTIO_WIKIPEDIA:
            searchTermLbl->clear();
            searchTermLbl->show();
            dictionaryDefinitionTextBrowser->clear();
            dictionaryDefinitionTextBrowser->setLinksAccepted(true);
            wikipediaWebBtn->show();
            m_changeDictioWidget->hide();
            changeDictionaryBtn->hide();
            showBackBtn(false);
            powerOnWikiButton();
        break;

        //case DICTIO_NONE:
        default:
            m_changeDictioWidget->hide();
            searchTermLbl->hide();
            wikipediaWebBtn->hide();
            showBackBtn(false);
            clearSearch();
            m_searchResult.clear();

            prevState = DICTIO_NONE;

            // Disable flags.
            disableSearchigWikipediaFlag();
            b_askingForWikipediaInfo = false;

            // Disconnect signals.
            disconnectWifiObserverWikipedia();
            disconnect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disconnectWifiObserverWikipedia()));
            disconnect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disableSearchigWikipediaFlag()));
            disconnect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(goToViewer()));
        break;
    }
}

void ViewerDictionary::powerOnDictioButton()
{
    Screen::getInstance()->queueUpdates();
    dictionaryBtn->setStyleSheet("background-color:#DADADA;");
    contentsTabArrowLbl->show();
    wikipediaBtn->setStyleSheet("background-color:#FFFFFF;");
    notesTabArrowLbl->hide();
    Screen::getInstance()->flushUpdates();
}

void ViewerDictionary::powerOnWikiButton()
{
    Screen::getInstance()->queueUpdates();
    dictionaryBtn->setStyleSheet("background-color:#FFFFFF;");
    contentsTabArrowLbl->hide();
    wikipediaBtn->setStyleSheet("background-color:#DADADA;");
    notesTabArrowLbl->show();
    Screen::getInstance()->flushUpdates();
}

void ViewerDictionary::resetDictionary()
{    
    state = prevState = DICTIO_NONE;
    clearSearch();
    m_searchResult.clear();

    // Disable flags.
    disableSearchigWikipediaFlag();
    b_askingForWikipediaInfo = false;

    // Disconnect signals.
    disconnectWifiObserverWikipedia();
    disconnect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disconnectWifiObserverWikipedia()));
    disconnect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(disableSearchigWikipediaFlag()));
    disconnect(QBookApp::instance(), SIGNAL(wifiConnectionCanceled()), this, SLOT(goToViewer()));
}

bool ViewerDictionary::isSameThatLastSearch(const QString& searchTerm) const
{
    return m_wikiTermSearchHistory.size() && searchTerm == m_wikiTermSearchHistory.last();
}

void ViewerDictionary::setSearchTitle(QString title)
{
    qDebug() << Q_FUNC_INFO << title;

    QString titleToShow = m_currentWikiSearchTerm.replace("_"," ");
    if (titleToShow.contains("#")) titleToShow = titleToShow.split("#", QString::SkipEmptyParts).at(0);
    titleToShow = bqUtils::truncateStringToWidth(titleToShow, m_searchTitleMaxWidth, searchTermLbl->font());
    qDebug() << Q_FUNC_INFO << titleToShow;
    searchTermLbl->setText(titleToShow);

    qDebug() << Q_FUNC_INFO << searchTermLbl->width();
}

bool ViewerDictionary::eBookLangIsOk(QString& lang)
{
    qDebug() << Q_FUNC_INFO << lang;

    // See http://en.wikipedia.org/wiki/ISO_639-2 for info:
    //     und undetermined (language)
    //     mis uncoded language
    //     mul multiple languages
    //     zxx no linguistic content
    if(lang.toLower() == UNDEFINED_LANGUAGE   ||
       lang.toLower() == "mis"                ||
       lang.toLower() == "mul"                ||
       lang.toLower() == "zxx"                )
    {
        return false;
    }

    // Handle en-US style strings
    if (lang.split("-").size() == 2) lang = lang.split("-")[0];

    // Special case to allow bad spanish codification
    if (lang.toLower() == "sp") lang = "es";

    if (!supportedISO639Language(lang)) return false;

    qDebug() << Q_FUNC_INFO << lang << "OK";
    return true;
}

bool ViewerDictionary::supportedISO639Language(QString& lang)
{
    /// From http://en.wikipedia.org/wiki/List_of_ISO_639-2_codes
    QString langStream = "ast aar	aar	aa abk	abk	ab afr	afr	af aka	aka	ak alb sqi	sqi	sq amh	amh	am ara	ara	ar arg	arg	an arm hye	hye	hy asm	asm	as ava	ava	av ave	ave	ae aym	aym	ay aze	aze	az bak	bak	ba bam	bam	bm baq eus	eus	eu bel	bel	be ben	ben	bn bih		bh bis	bis	bi bod tib	bod	bo bos	bos	bs bre	bre	br bul	bul	bg bur mya	mya	my cat	cat	ca ces cze ces	cs cha	cha	ch che	che	ce chi zho	zho	zh chu	chu	cu chv	chv	cv cor	cor	kw cos	cos	co cre	cre	cr cym wel cym	cy cze ces	ces	cs dan	dan	da deu ger	deu	de div	div	dv dut nld	nld	nl dzo	dzo	dz ell  gre ell	el eng	eng	en epo	epo	eo est	est	et eus baq	eus	eu ewe	ewe	ee fao	fao	fo fas per	fas	fa fij	fij	fj fin	fin	fi fra fre	fra	fr fre fra	fra	fr fry	fry	fy ful	ful	ff geo kat	kat	ka ger deu	deu	de gla	gla	gd gle	gle	ga glg	glg	gl glv	glv	gv gre ell	ell	el grn	grn	gn guj	guj	gu hat	hat	ht hau	hau	ha heb	heb	he her	her	hz hin	hin	hi hmo	hmo	ho hrv	hrv	hr hun	hun	hu hye arm	hye	hy ibo	ibo	ig ice isl	isl	is ido	ido	io iii	iii	ii iku	iku	iu ile	ile	ina	ina	ia ind	ind	id ipk	ipk	ik isl ice isl	is ita	ita	it jav	jav	jv jpn	jpn	ja kal	kal	kl kan	kan	kn kas	kas	ks kat geo	kat	ka kau	kau	kr kaz	kaz	kk khm	khm	km kik	kik	ki kin	kin	rw kir	kir	ky kom	kom	kv kon	kon	kg kor	kor	ko kua	kua	kj kur	kur	ku lao	lao	lo lat	lat	la lav	lav	lv lim	lim	li lin	lin	ln lit	lit	lt ltz	ltz	lb lub	lub	lu lug	lug	lg mac mkd	mkd	mk mah	mah	mh mal	mal	ml mao mri	mri	mi mar	mar	mr may msa	msa	ms mkd mac	mkd	mk mlg	mlg	mg mlt	mlt	mt mon	mon	mn mri mao	mri	mi msa may	msa	ms mya bur	mya	my nau	nau	na nav	nav	nv nbl	nbl	nr nde	nde	nd ndo	ndo	ng nep	nep	ne nld dut	nld	nl nno	nno	nn nob	nob	nb nor	nor	no nya	nya	ny oci	oci	oc oji	oji	oj ori	ori	or orm	orm	om oss	oss	os pan	pan	pa per fas	fas	fa pli	pli	pi pol	pol	pl por	por	pt pus	pus	ps que	que	qu roh	roh	rm ron rum ron	ro rum ron	ron	ro run	run	rn rus	rus	ru sag	sag	sg san	san	sa sin	sin	si slo slk	slk	sk slk slo	slk	sk slv	slv	sl sme	sme	se smo	smo	sm sna	sna	sn snd	snd	sd som	som	so sot	sot	st spa	spa	es sqi alb	sqi	sq srd	srd	sc srp	srp	sr ssw	ssw	ss sun	sun	su swa	swa	sw swe	swe	sv tah	tah	ty tam	tam	ta tat	tat	tt tel	tel	te tgk	tgk	tg tgl	tgl	tl tha	tha	th tib bod	bod	bo tir	tir	ti ton	ton	to tsn	tsn	tn tso	tso	ts tuk	tuk	tk tur	tur	tr twi	twi	tw uig	uig	ug ukr	ukr	uk urd	urd	ur uzb	uzb	uz ven	ven	ve vie	vie	vi vol	vol	vo wel cym	cym	cy wln	wln	wa wol	wol	wo xho	xho	xh yid	yid	yi yor	yor	yo zha	zha	za zho chi	zho	zh zul	zul	zu";
    QStringList langList = langStream.split(QRegExp("\\s+"));
    qDebug() << Q_FUNC_INFO << langList;
    return langList.contains(lang, Qt::CaseInsensitive);
}
