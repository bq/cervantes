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

#include "ViewerAppearancePopup.h"
#include "Viewer.h"
#include "QBook.h"
#include "QBookApp.h"
#include "PowerManager.h"
#include "QDocView.h"
#include "BookInfo.h"
#include "Screen.h"

#include <QDebug>
#include <QPoint>
#include <cmath>
#include <QButtonGroup>
#include <QPainter>

#define DEFAULT_FONT_TYPE_ID        4
#define MAX_SIZE_LEVEL              7

#ifdef Q_WS_QWS
const QString NO_SPACING_FILE       ("file:///app/res/noSpacing.css");
const QString MEDIUM_SPACING_FILE   ("file:///app/res/mediumSpacing.css");
const QString HIGH_SPACING_FILE     ("file:///app/res/highSpacing.css");
#else
const QString NO_SPACING_FILE       ("QHome/res/noSpacing.css");
const QString MEDIUM_SPACING_FILE   ("QHome/res/mediumSpacing.css");
const QString HIGH_SPACING_FILE     ("QHome/res/highSpacing.css");
#endif

#define MAX_ZOOM_BUTTON_PLUS_STYLE      "background-repeat: no-repeat; background-image: url(:res/plus-ico.png); background-position: top right;"
#define MAX_ZOOM_BUTTON_NORMAL_STYLE    "background-image: none;"

// Fonts definitions
const int s_fontsSize = 7;
#define FONT_NAME 0
#define FONT_FILE 1
#define FONT_FAMILY 2
const QString s_fontsDefs[s_fontsSize][3] ={ {"Istok Web",    "file:///app/res/istokWebUserStyle.css",    "Istok Web"}
                                            ,{"Cabin",        "file:///app/res/cabinUserStyle.css",       "Cabin"}
                                            ,{"Vera",         "file:///app/res/veraUserStyle.css",        "Bitstream Vera Sans"}
                                            ,{"Vollkorn",     "file:///app/res/vollkornUserStyle.css",    "Vollkorn"}
                                            ,{"Noticia Text", "file:///app/res/noticiaTextUserStyle.css", "Noticia Text"}
                                            ,{"Droid Serif",  "file:///app/res/droidSerifUserStyle.css",  "Droid Serif"}
                                            ,{"Tiresias",     "file:///app/res/tiresiasUserStyle.css",    "Tiresias LPfont"}};


ViewerAppearancePopup::ViewerAppearancePopup(Viewer* viewer) :
    ViewerMenuPopUp(viewer)
  , m_fontSizeOutOfRange(false)
  , m_defaultSettings(true)
  , m_currentFontSizeId(0)
  , m_currentPdfZoomLevelId(0)
  , m_currentFontNameId(0)
  , m_currentPageMode(0)
  , m_currentMarginValue(MARGIN_MAX)
  , m_currentSpacingValue(SPACING_MIN)
  , m_currentJustifyValue(JUSTIFY_ALIGN)
  , g_SizeButtons(new QButtonGroup(this))
  , g_FontButtons(new QButtonGroup(this))
  , g_MarginButtons(new QButtonGroup(this))
  , g_SpacingButtons(new QButtonGroup(this))
  , g_JustifyButtons(new QButtonGroup(this))
{
    setupUi(this);

    // Set default values for margin and spacing
    m_marginMap.insert(MARGIN_MIN, MarginPercentage(3, 2, 3, 2));
    m_marginMap.insert(MARGIN_MEDIUM, MarginPercentage(6, 10, 6, 10));
    m_marginMap.insert(MARGIN_MAX, MarginPercentage(15, 15, 15, 15));

    m_spacingList.append(NO_SPACING_FILE);
    m_spacingList.append(MEDIUM_SPACING_FILE);
    m_spacingList.append(HIGH_SPACING_FILE);

    qDebug() << Q_FUNC_INFO;

    // Set Font name and Font style to font buttons
    QString fontFamily("font-family: '%1';");

    font1Btn->setText(s_fontsDefs[0][FONT_NAME]);
    font1Btn->setStyleSheet(fontFamily.arg(s_fontsDefs[0][FONT_FAMILY]));

    font2Btn->setText(s_fontsDefs[1][FONT_NAME]);
    font2Btn->setStyleSheet(fontFamily.arg(s_fontsDefs[1][FONT_FAMILY]));

    font3Btn->setText(s_fontsDefs[2][FONT_NAME]);
    font3Btn->setStyleSheet(fontFamily.arg(s_fontsDefs[2][FONT_FAMILY]));

    font4Btn->setText(s_fontsDefs[3][FONT_NAME]);
    font4Btn->setStyleSheet(fontFamily.arg(s_fontsDefs[3][FONT_FAMILY]));

    font5Btn->setText(s_fontsDefs[4][FONT_NAME]);
    font5Btn->setStyleSheet(fontFamily.arg(s_fontsDefs[4][FONT_FAMILY]));

    font6Btn->setText(s_fontsDefs[5][FONT_NAME]);
    font6Btn->setStyleSheet(fontFamily.arg(s_fontsDefs[5][FONT_FAMILY]));

    font7Btn->setText(s_fontsDefs[6][FONT_NAME]);
    font7Btn->setStyleSheet(fontFamily.arg(s_fontsDefs[6][FONT_FAMILY]));
    //

    g_SizeButtons->addButton(size1, 0);
    g_SizeButtons->addButton(size2, 1);
    g_SizeButtons->addButton(size3, 2);
    g_SizeButtons->addButton(size4, 3);
    g_SizeButtons->addButton(size5, 4);
    g_SizeButtons->addButton(size6, 5);
    g_SizeButtons->addButton(size7, 6);
    g_SizeButtons->addButton(size8, 7);

    g_FontButtons->addButton(font1Btn, 0);
    g_FontButtons->addButton(font2Btn, 1);
    g_FontButtons->addButton(font3Btn, 2);
    g_FontButtons->addButton(font4Btn, 3);
    g_FontButtons->addButton(font5Btn, 4);
    g_FontButtons->addButton(font6Btn, 5);
    g_FontButtons->addButton(font7Btn, 6);

    g_MarginButtons->addButton(margin1Btn, MARGIN_MAX);
    g_MarginButtons->addButton(margin2Btn, MARGIN_MEDIUM);
    g_MarginButtons->addButton(margin3Btn, MARGIN_MIN);

    g_SpacingButtons->addButton(spacing1Btn, SPACING_MIN);
    g_SpacingButtons->addButton(spacing2Btn, SPACING_MEDIUM);
    g_SpacingButtons->addButton(spacing3Btn, SPACING_MAX);

    g_JustifyButtons->addButton(justify1Btn, JUSTIFY_ALIGN);
    g_JustifyButtons->addButton(justify2Btn, LEFT_ALIGN);
    g_JustifyButtons->addButton(justify3Btn, DEFAULT_ALIGN);

    connect(g_SizeButtons,      SIGNAL(buttonPressed(int)), this, SLOT(selectFontSize(int)));
    connect(g_FontButtons,      SIGNAL(buttonPressed(int)), this, SLOT(selectFontType(int)));
    connect(g_MarginButtons,    SIGNAL(buttonPressed(int)), this, SLOT(selectMargin(int)));
    connect(g_SpacingButtons,   SIGNAL(buttonPressed(int)), this, SLOT(selectSpacing(int)));
    connect(g_JustifyButtons,   SIGNAL(buttonPressed(int)), this, SLOT(selectJustify(int)));

    connect(editorialBtn,       SIGNAL(clicked()),          this, SLOT(selectEditorDefaults()));
    connect(seeImagesBtn,       SIGNAL(clicked()),          this, SLOT(selectImagesMode()));
    connect(closeBtn,           SIGNAL(clicked()),          this, SIGNAL(hideMe()));
}

ViewerAppearancePopup::~ViewerAppearancePopup()
{
    qDebug() << Q_FUNC_INFO;

    delete g_SizeButtons;
    g_SizeButtons = NULL;

    delete g_FontButtons;
    g_FontButtons = NULL;

    delete g_MarginButtons;
    g_MarginButtons = NULL;

    delete g_SpacingButtons;
    g_SpacingButtons = NULL;

    delete g_JustifyButtons;
    g_JustifyButtons = NULL;
}

void ViewerAppearancePopup::setup()
{
    setupBook(m_parentViewer->getCurrentBookInfo());
}

void ViewerAppearancePopup::start()
{
    qDebug() << Q_FUNC_INFO;

    // save the current bookmark
    m_parentViewer->updateBookInfo(false);// NOTE: Es para actualizar el último punto de lectura en el que nos encontramos, para que la recarga del libro sea correcta
}

void ViewerAppearancePopup::stop()
{}

void ViewerAppearancePopup::setupBook( const BookInfo* info )
{
    qDebug() << Q_FUNC_INFO << info;

    // Retrieve the values depending on the extension of the book
    switch(Viewer::getFileExtension(info->path))
    {
        case Viewer::EXT_PDF:
        {
            // Editor fonts
            m_defaultSettings = false;
            // Font Size
            if(info->pageMode == QDocView::MODE_REFLOW_PAGES)
                m_currentPdfZoomLevelId = m_currentFontSizeId = QBook::settings().value("setting/reader/font_size_id", 2).toInt();
            else // PDF regular mode
                m_currentPdfZoomLevelId = m_currentFontSizeId = 0; // PDF always opened as full page

            // Page Mode
            m_currentPageMode = info->pageMode;

            break;
        }

        case Viewer::EXT_EPUB: // gestionar tambien para resto de formatos
        {
            // Editor fonts
            m_defaultSettings = QBook::settings().value("setting/reader/editorFonts", true).toBool();
            // Font Size
            m_currentFontSizeId = qBound(0, QBook::settings().value("setting/reader/font_size_id", 2).toInt(), NUM_AVAILABLE_SIZES-1);
            // Font Name
            QString fontFileName(QBook::settings().value("setting/reader/font/epub", s_fontsDefs[DEFAULT_FONT_TYPE_ID][FONT_FILE]).toString());
            int fontNameId = getFontTypeIdByFile(fontFileName);
            if(fontNameId == -1)
            {
                // TODO: Perform a change of font (Notify to the user? Remains unselected the default one)
                fontNameId = DEFAULT_FONT_TYPE_ID;
            }
            m_currentFontNameId = fontNameId;
            // Margins
            applyMargin();
            // Spacing
            m_currentSpacingValue = (eSpacing)m_spacingList.indexOf(QBook::settings().value("setting/reader/spacing/epub", NO_SPACING_FILE).toString());
            // Justify
            m_currentJustifyValue = (eJustify)QBook::settings().value("setting/reader/justify/epub", DEFAULT_ALIGN).toInt();
            // Page Mode
            m_currentPageMode = info->pageMode;

            break;
        }
        default:
        {
            // CRENGINE formats
            if(Viewer::isSupportedFile(info->path))
            {
                // Editor fonts
                m_defaultSettings = false;
                // Font Size
                m_currentFontSizeId = QBook::settings().value("setting/reader/font_size_id", 2).toInt();
                // Font Name
                QString fontFamilyName(QBook::settings().value("setting/reader/font/cr3", s_fontsDefs[DEFAULT_FONT_TYPE_ID][FONT_FAMILY]).toString());
                int fontNameId = getFontTypeIdByFamily(fontFamilyName);
                if(fontNameId == -1)
                {
                    // TODO: Perform a change of font (Notify to the user? Remains unselected the default one)
                    fontNameId = DEFAULT_FONT_TYPE_ID;
                }
                m_currentFontNameId = fontNameId;
                // Margins
                applyMargin();
                // Spacing
                m_currentSpacingValue = (eSpacing)m_spacingList.indexOf(QBook::settings().value("setting/reader/spacing/cr3", NO_SPACING_FILE).toString());
                // Page Mode
                m_currentPageMode = info->pageMode;

                break;
            }

            break;
        }
    }

    applySizeChange();

    // Setup Ui
    setupUiButtons();

    // Set the buttons values
    setDefaultEditorSettings();
    setFontSizeSelected();
    setFontTypeSelected();
    setMarginSelected();
    setSpacingSelected();
    setJustifySelected();
    setPageModeSelected();

    qDebug() << Q_FUNC_INFO << "FontsizeID = " << m_currentFontSizeId << ", FontnameID = " << m_currentFontNameId << ", PageMode = " << m_currentPageMode;
}

void ViewerAppearancePopup::applyMargin()
{
    qDebug() << Q_FUNC_INFO;

    if (m_parentViewer->getCurrentDocExt() == Viewer::EXT_EPUB)
        m_currentMarginValue = (eMargin)QBook::settings().value("setting/reader/margin/epub", MARGIN_MIN).toInt();
    else
        m_currentMarginValue = (eMargin)QBook::settings().value("setting/reader/margin/cr3", MARGIN_MIN).toInt();

    MarginPercentage margin(m_marginMap.value(m_currentMarginValue));
    m_parentViewer->setMargin(margin.topPercentage, margin.rightPercentage, margin.bottomPercentage, margin.leftPercentage);
}

void ViewerAppearancePopup::applySizeChange()
{
    qDebug()  << Q_FUNC_INFO;

    PowerManagerLock* powerLock = PowerManager::getNewLock(this);
    powerLock->activate();

    switch(m_parentViewer->getCurrentDocExt())
    {
    case Viewer::EXT_PDF:
    {
        // Editor fonts
        m_defaultSettings = true;
        // Font Size

        Screen::getInstance()->lockScreen();

        m_parentViewer->docView()->setPageMode(static_cast<QDocView::PageMode>(m_currentPageMode));
        m_parentViewer->getCurrentBookInfo()->pageMode = static_cast<int>(m_parentViewer->docView()->pageMode());
        m_parentViewer->docView()->setScaleFactor(getScaleFactor());

        Screen::getInstance()->releaseScreen();

        break;
    }
    default:
    {
        m_parentViewer->docView()->setPageMode(static_cast<QDocView::PageMode>(m_currentPageMode));// NOTE: EPUB doesn't care about pageMode, but the docView needs to reset this mode.
        m_parentViewer->getCurrentBookInfo()->pageMode = static_cast<int>(m_parentViewer->docView()->pageMode());
        m_parentViewer->docView()->setScaleFactor(getScaleFactor());
        break;
    }
    }

    delete powerLock;
}

int ViewerAppearancePopup::getFontSizeId() const
{
    if (isPdfHardMode()) return m_currentPdfZoomLevelId;

    return m_currentFontSizeId;
}

double ViewerAppearancePopup::getScaleFactor() const
{
	switch(m_parentViewer->getCurrentDocExt())
    {
    case Viewer::EXT_PDF:
    {
		double baseLine = m_parentViewer->docView()->minScaleFactor();
	    double step = m_parentViewer->docView()->scaleStep();
	    double factor   = baseLine + getFontSizeId() * step;

	    qDebug()  << Q_FUNC_INFO << "RETURNS" << factor;

	    return factor;
    }
    default: //EPUB and CREngine
    {
        double factor = m_parentViewer->docView()->getFontSizeListAt(getFontSizeId());

        qDebug()  << Q_FUNC_INFO << "RETURNS" << factor;

        return factor;
    }

	}
}

int ViewerAppearancePopup::getFontTypeIdByFile(const QString& fontFileName) const
{
    qDebug() << Q_FUNC_INFO << "Fontname: " << fontFileName;

    for (int i=0; i < s_fontsSize; ++i)
    {
        if (fontFileName == s_fontsDefs[i][FONT_FILE])
        {
            qDebug() << Q_FUNC_INFO << "Font Name found";
            return i;
        }
    }

    qDebug() << Q_FUNC_INFO <<  "Font name not found";
    return -1;
}

int ViewerAppearancePopup::getFontTypeIdByFamily(const QString& fontFamilyName) const
{
    qDebug() << Q_FUNC_INFO << "Fontname: " << fontFamilyName;

    for (int i=0; i < s_fontsSize; ++i)
    {
        if (fontFamilyName == s_fontsDefs[i][FONT_FAMILY])
        {
            qDebug() << Q_FUNC_INFO << "Font Name found";
            return i;
        }
    }

    qDebug() << Q_FUNC_INFO <<  "Font name not found";
    return -1;
}

void ViewerAppearancePopup::selectMargin(int newMargin)
{
    qDebug() << Q_FUNC_INFO << newMargin;

    PowerManagerLock* powerLock = PowerManager::getNewLock(this);
    powerLock->activate();

    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(true);
    Screen::getInstance()->flushUpdates();
    if (m_currentMarginValue != newMargin)
    {
        g_MarginButtons->button((int)m_currentMarginValue)->setChecked(false);
        g_MarginButtons->button(newMargin)->setChecked(true);

        m_currentMarginValue = (eMargin)newMargin;

        if (m_parentViewer->getCurrentDocExt() == Viewer::EXT_EPUB)
            QBook::settings().setValue("setting/reader/margin/epub", m_currentMarginValue);
        else
            QBook::settings().setValue("setting/reader/margin/cr3", m_currentMarginValue);

        m_parentViewer->reloadCurrentBook();
    }

    QBookApp::instance()->getStatusBar()->setSpinner(false);
    delete powerLock;
}

void ViewerAppearancePopup::selectSpacing(int newSpacing)
{
    qDebug() << Q_FUNC_INFO << newSpacing;

    PowerManagerLock* powerLock = PowerManager::getNewLock(this);
    powerLock->activate();

    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(true);
    Screen::getInstance()->flushUpdates();

    if (m_defaultSettings && m_parentViewer->getCurrentDocExt() == Viewer::EXT_EPUB)
    {
        m_defaultSettings = false;
        setDefaultEditorSettings();
        QBook::settings().setValue("setting/reader/editorFonts", false);

        m_currentSpacingValue = SPACING_NONE; // NOTE: Force going inside the next if
    }

    if (m_currentSpacingValue != newSpacing)
    {
        if(m_currentSpacingValue != SPACING_NONE)
            g_SpacingButtons->button((int)m_currentSpacingValue)->setChecked(false);

        g_SpacingButtons->button(newSpacing)->setChecked(true);
        m_currentSpacingValue = (eSpacing)newSpacing;

        QString spacingFile = m_spacingList.at(m_currentSpacingValue);
        qDebug() << Q_FUNC_INFO << "New spacing" << spacingFile;

        if (m_parentViewer->getCurrentDocExt() == Viewer::EXT_EPUB)
            QBook::settings().setValue("setting/reader/spacing/epub", spacingFile);
        else
            QBook::settings().setValue("setting/reader/spacing/cr3", spacingFile);

        m_parentViewer->reloadCurrentBook();
    }
    QBookApp::instance()->getStatusBar()->setSpinner(false);
    delete powerLock;
}

void ViewerAppearancePopup::selectJustify(int justifyValue)
{
    qDebug() << Q_FUNC_INFO << "justifyValue: " << justifyValue;

    PowerManagerLock* powerLock = PowerManager::getNewLock(this);
    powerLock->activate();

    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(true);
    Screen::getInstance()->flushUpdates();

    if (m_defaultSettings && m_parentViewer->getCurrentDocExt() == Viewer::EXT_EPUB) // Incluir soporte para resto de formatos y conf del editor
    {
        m_defaultSettings = false;
        setDefaultEditorSettings();
        QBook::settings().setValue("setting/reader/editorFonts", false);

        m_currentJustifyValue = JUSTIFY_NONE; // NOTE: Force going inside the next if
    }

    if(m_currentJustifyValue != justifyValue)
    {
        if(m_currentJustifyValue != JUSTIFY_NONE)
            g_JustifyButtons->button((int)m_currentJustifyValue)->setChecked(false);
        g_JustifyButtons->button(justifyValue)->setChecked(true);

        if (m_parentViewer->getCurrentDocExt() == Viewer::EXT_EPUB)
            QBook::settings().setValue("setting/reader/justify/epub", justifyValue);
        else
            QBook::settings().setValue("setting/reader/justify/cr3", justifyValue);

        m_currentJustifyValue = (eJustify)justifyValue;
        m_parentViewer->reloadCurrentBook();
    }

    QBookApp::instance()->getStatusBar()->setSpinner(false);
    delete powerLock;
}

void ViewerAppearancePopup::selectEditorDefaults()
{
    qDebug() << Q_FUNC_INFO << !m_defaultSettings;

    PowerManagerLock* powerLock = PowerManager::getNewLock(this);
    powerLock->activate();

    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(true);
    Screen::getInstance()->flushUpdates();

    if (!editorialCont->isEnabled())
        m_defaultSettings = true;
    else
        m_defaultSettings = !m_defaultSettings;

    Screen::getInstance()->resetQueue();
    // Setup Ui
    setupUiButtons();

    // Set the buttons values
    setDefaultEditorSettings();
    setFontSizeSelected();
    setFontTypeSelected();
    setMarginSelected();
    setSpacingSelected();
    setJustifySelected();
    setPageModeSelected();

    // Apply changes in document
    QBook::settings().setValue("setting/reader/editorFonts", m_defaultSettings);
    m_parentViewer->reloadCurrentBook();

    QBookApp::instance()->getStatusBar()->setSpinner(false);
    delete powerLock;
}

void ViewerAppearancePopup::selectImagesMode()
{
    qDebug() << Q_FUNC_INFO;

    PowerManagerLock* powerLock = PowerManager::getNewLock(this);
    powerLock->activate();

    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(true);
    Screen::getInstance()->flushUpdates();

    if(m_currentPageMode == QDocView::MODE_REFLOW_PAGES)
    {
        m_parentViewer->enablePdfMode(true);
        m_currentPageMode = QDocView::MODE_HARD_PAGES;
        seeImagesBtn->setChecked(true);
        seeImagesBtn->setStyleSheet("background-image: url(:/res/on_btn.png);");
    }
    else
    {
        m_parentViewer->enablePdfMode(false);
        m_currentPageMode = QDocView::MODE_REFLOW_PAGES;
        seeImagesBtn->setChecked(false);
        seeImagesBtn->setStyleSheet("background-image: url(:/res/off_btn.png);");
    }

    /////////////
    qDebug() << Q_FUNC_INFO << "FontsizeID = " << m_currentFontSizeId << ", FontnameID = " << m_currentFontNameId << ", PageMode = " << m_currentPageMode;
    int newFontSize = 0;// set to full page (for MODE_HARD_PAGES)
    if(m_currentPageMode == QDocView::MODE_REFLOW_PAGES)
        newFontSize = QBook::settings().value("setting/reader/font_size_id", 2).toInt(); // Reflow, use stored font

    setSizeButtonsAspect(newFontSize);
    applySizeChange();

    QBookApp::instance()->getStatusBar()->setSpinner(false);
    delete powerLock;
}

void ViewerAppearancePopup::selectFontSize(int newFontSize)
{
    qDebug() << Q_FUNC_INFO << newFontSize;

    PowerManagerLock* powerLock = PowerManager::getNewLock(this);
    powerLock->activate();

    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(true);
    Screen::getInstance()->flushUpdates();
    if (m_currentFontSizeId != newFontSize)
    {
        setSizeButtonsAspect(newFontSize);

        // Save font size
        qDebug() << Q_FUNC_INFO << "NEW SIZE" << m_currentFontSizeId;

        // Don't store font size when on pdfs with hard pages mode
        if (!isPdfHardMode())
        {
            m_parentViewer->getCurrentBookInfo()->fontSize = m_currentFontSizeId;
            QBook::settings().setValue("setting/reader/font_size_id", m_currentFontSizeId);
        }

        // Apply the size
        m_parentViewer->docView()->setScaleFactor(getScaleFactor());
    }
    QBookApp::instance()->getStatusBar()->setSpinner(false);
    delete powerLock;
}

void ViewerAppearancePopup::selectFontType(int newFontNameId)
{
    qDebug() << Q_FUNC_INFO << newFontNameId;

    PowerManagerLock* powerLock = PowerManager::getNewLock(this);
    powerLock->activate();

    Screen::getInstance()->queueUpdates();
    QBookApp::instance()->getStatusBar()->setSpinner(true);
    Screen::getInstance()->flushUpdates();

    if (m_defaultSettings && m_parentViewer->getCurrentDocExt() == Viewer::EXT_EPUB)
    {
        m_defaultSettings = false;
        setDefaultEditorSettings();
        QBook::settings().setValue("setting/reader/editorFonts", false);

        m_currentFontNameId = -1; // NOTE: Force going inside the next if
    }

    if (m_currentFontNameId != newFontNameId)
    {
        Screen::getInstance()->queueUpdates();
        if(m_currentFontNameId != -1)
            g_FontButtons->button((int)m_currentFontNameId)->setChecked(false);
        g_FontButtons->button(newFontNameId)->setChecked(true);
        Screen::getInstance()->flushUpdates();

        m_currentFontNameId = newFontNameId;

        // Font change, allowing same font in all formats.
        QBook::settings().setValue("setting/reader/font/epub", s_fontsDefs[m_currentFontNameId][FONT_FILE]);
        QBook::settings().setValue("setting/reader/font/cr3", s_fontsDefs[m_currentFontNameId][FONT_FAMILY]);
        m_parentViewer->reloadCurrentBook();
    }
    QBookApp::instance()->getStatusBar()->setSpinner(false);
    delete powerLock;
}

void ViewerAppearancePopup::setPageModeSelected()
{
    qDebug() << Q_FUNC_INFO << "id button" << m_currentPageMode;
    if (seeImagesCont->isEnabled())
    {
        if(m_currentPageMode == QDocView::MODE_REFLOW_PAGES)
        {
            seeImagesBtn->setChecked(false);// NOTE: We have to do this for the behaviour of this button
            seeImagesBtn->setStyleSheet("background-image: url(:/res/off_btn.png);");
        }
        else
        {
            seeImagesBtn->setChecked(true);// NOTE: We have to do this for the behaviour of this button
            seeImagesBtn->setStyleSheet("background-image: url(:/res/on_btn.png);");
        }
    }
    else
    {
        seeImagesBtn->setChecked(false);// NOTE: We have to do this for the behaviour of this button
        seeImagesBtn->setStyleSheet("background-image: url(:/res/off_btn.png);");
    }
}

void ViewerAppearancePopup::setMarginSelected()
{
    qDebug() << Q_FUNC_INFO << "id button" << m_currentMarginValue;
    if (marginCont->isEnabled())
        g_MarginButtons->button(m_currentMarginValue)->setChecked(true);
    else
        g_MarginButtons->button(m_currentMarginValue)->setChecked(false);
}

void ViewerAppearancePopup::setSpacingSelected()
{
    qDebug() << Q_FUNC_INFO << "id button" << m_currentSpacingValue;
    if (spacingCont->isEnabled() && !m_defaultSettings)
        g_SpacingButtons->button(m_currentSpacingValue)->setChecked(true);
    else
    {
        g_SpacingButtons->setExclusive(false);
        g_SpacingButtons->button(m_currentSpacingValue)->setChecked(false);
    }
}

void ViewerAppearancePopup::setDefaultEditorSettings()
{
    qDebug() << Q_FUNC_INFO << m_defaultSettings;

     Viewer::SupportedExt ext = (Viewer::SupportedExt)m_parentViewer->getCurrentDocExt();

    if(m_defaultSettings && !Viewer::isUsingCR3(ext) && ext != Viewer::EXT_PDF)
    {
        editorialBtn->setChecked(true);// NOTE: We have to do this for the behaviour of this button
        editorialBtn->setStyleSheet("background-image: url(:/res/on_btn.png);");
    }
    else
    {
        editorialBtn->setChecked(false);// NOTE: We have to do this for the behaviour of this button
        editorialBtn->setStyleSheet("background-image: url(:/res/off_btn.png);");
    }
}

// Puts font's selected marks
void ViewerAppearancePopup::setFontSizeSelected()
{
    qDebug() << Q_FUNC_INFO << "id button" << m_currentFontSizeId;

    g_SizeButtons->button(m_currentFontSizeId)->setChecked(sizeCont->isEnabled());

    if (m_currentFontSizeId == MAX_SIZE_LEVEL)
        g_SizeButtons->button(MAX_SIZE_LEVEL)->setStyleSheet(MAX_ZOOM_BUTTON_NORMAL_STYLE);
}

void ViewerAppearancePopup::setJustifySelected()
{
    qDebug() << Q_FUNC_INFO << m_currentJustifyValue;
    if (justifyCont->isEnabled() && !m_defaultSettings)
        g_JustifyButtons->button(m_currentJustifyValue)->setChecked(true);
    else
    {
        g_JustifyButtons->setExclusive(false);
        g_JustifyButtons->button(m_currentJustifyValue)->setChecked(false);
    }
}

void ViewerAppearancePopup::setFontTypeSelected()
{
    qDebug() << Q_FUNC_INFO << m_currentFontNameId;
    if (fontTypeCont->isEnabled() && !m_defaultSettings)
        g_FontButtons->button(m_currentFontNameId)->setChecked(true);
    else
    {
        g_FontButtons->setExclusive(false);
        g_FontButtons->button(m_currentFontNameId)->setChecked(false);
    }
}

void ViewerAppearancePopup::setupUiButtons()
{
    qDebug() << Q_FUNC_INFO;

    // Disable font type buttons depending on extension
    Viewer::SupportedExt ext = (Viewer::SupportedExt)m_parentViewer->getCurrentDocExt();

    if(Viewer::isUsingCR3(ext))
    {
        qDebug() << Q_FUNC_INFO << "setup fonts for CRENGINE";

        editorialCont->setEnabled(false);
        editorialBtn->setChecked(false);
        editorialBtn->setStyleSheet("background-image: url(:/res/off_btn.png);");

        sizeCont->setEnabled(true);
        fontTypeCont->setEnabled(true);
        marginCont->setEnabled(true);
        spacingCont->setEnabled(true);
        justifyCont->setEnabled(false);

        g_MarginButtons->setExclusive(true);
        g_SpacingButtons->setExclusive(true);
        g_FontButtons->setExclusive(true);
        g_JustifyButtons->setExclusive(false);

        seeImagesCont->setEnabled(false);

    }
    else if(ext == Viewer::EXT_EPUB)
    {
        qDebug() << Q_FUNC_INFO << "setup fonts for EPUB powered by Adobe";

        editorialCont->setEnabled(true);
        sizeCont->setEnabled(true);
        fontTypeCont->setEnabled(true);
        marginCont->setEnabled(true);
        spacingCont->setEnabled(true);
        justifyCont->setEnabled(true);

        g_MarginButtons->setExclusive(true);
        g_SpacingButtons->setExclusive(true);
        g_JustifyButtons->setExclusive(true);
        g_FontButtons->setExclusive(true);

        seeImagesCont->setEnabled(false);

        if(!m_parentViewer->docView()->getCanOverrideFonts())
        {
            // Disable choosing fonts
            fontTypeCont->setEnabled(false);
            editorialCont->setEnabled(false);
        }
        else
        {
            fontTypeCont->setEnabled(true);
            editorialCont->setEnabled(true);
        }
    }
    else if(ext == Viewer::EXT_PDF)
    {
        qDebug() << Q_FUNC_INFO << "setup fonts for PDF";

        seeImagesCont->setEnabled(true);
        sizeCont->setEnabled(true);

        editorialCont->setEnabled(false);
        fontTypeCont->setEnabled(false);
        marginCont->setEnabled(false);
        spacingCont->setEnabled(false);
        justifyCont->setEnabled(false);

        g_MarginButtons->setExclusive(false);
        g_SpacingButtons->setExclusive(false);
        g_JustifyButtons->setExclusive(false);
     }
}

void ViewerAppearancePopup::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ViewerAppearancePopup::pdfZoomLevelChange(int newZoomLevel)
{
    qDebug() << Q_FUNC_INFO << newZoomLevel;

    setSizeButtonsAspect(newZoomLevel);
}

void ViewerAppearancePopup::setSizeButtonsAspect(const uint newFontSize)
{
    if (m_currentFontSizeId == (int)newFontSize) return;

    Screen::getInstance()->queueUpdates();

    if (newFontSize > MAX_SIZE_LEVEL && withinSizeButtonsRange()) // Leave range.
    {
        g_SizeButtons->button(MAX_SIZE_LEVEL)->setStyleSheet(MAX_ZOOM_BUTTON_PLUS_STYLE);

        if (m_currentFontSizeId != MAX_SIZE_LEVEL)
        {
            g_SizeButtons->button(m_currentFontSizeId)->setChecked(false);
            g_SizeButtons->button(MAX_SIZE_LEVEL)->setChecked(true);
        }

        m_currentPdfZoomLevelId = newFontSize;
        m_currentFontSizeId = MAX_SIZE_LEVEL;
        m_fontSizeOutOfRange = true;
    }
    else if (newFontSize <= MAX_SIZE_LEVEL)
    {
        if (!withinSizeButtonsRange())// Enter in range.
        {
            g_SizeButtons->button(MAX_SIZE_LEVEL)->setStyleSheet(MAX_ZOOM_BUTTON_NORMAL_STYLE);

            if (newFontSize < MAX_SIZE_LEVEL)
            {
                g_SizeButtons->button(m_currentFontSizeId)->setChecked(false);
                g_SizeButtons->button(newFontSize)->setChecked(true);
            }

            m_currentPdfZoomLevelId = m_currentFontSizeId = newFontSize;
            m_fontSizeOutOfRange = false;
        }
        else // Stay in range.
        {
            g_SizeButtons->button(m_currentFontSizeId)->setChecked(false);
            g_SizeButtons->button(newFontSize)->setChecked(true);

            m_currentPdfZoomLevelId = m_currentFontSizeId = newFontSize;
            m_fontSizeOutOfRange = false;
        }
    }

    Screen::getInstance()->flushUpdates();
}


bool ViewerAppearancePopup::withinSizeButtonsRange()
{
    return (m_currentPdfZoomLevelId <= MAX_SIZE_LEVEL && !m_fontSizeOutOfRange);
}

bool ViewerAppearancePopup::isPdfHardMode() const
{
    return m_parentViewer->getCurrentDocExt() == Viewer::EXT_PDF && m_currentPageMode == QDocView::MODE_HARD_PAGES;
}
