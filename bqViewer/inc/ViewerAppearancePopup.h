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

#ifndef VIEWERAPPEARANCEPOPUP_H
#define VIEWERAPPEARANCEPOPUP_H

#include "ViewerMenuPopUp.h"
#include "ui_ViewerAppearancePopup.h"

#include <QString>
#include <QStringList>
#include <QMap>

// Predeclarations
class QWidget;
class Viewer;
class BookInfo;
class PowerManagerLock;
class QButtonGroup;
class QFrame;
class Viewer;

class MarginPercentage
{
public:
    int topPercentage;
    int rightPercentage;
    int bottomPercentage;
    int leftPercentage;
    MarginPercentage() {}
    MarginPercentage( const MarginPercentage& other ) :
        topPercentage(other.topPercentage),
        rightPercentage(other.rightPercentage),
        bottomPercentage(other.bottomPercentage),
        leftPercentage(other.leftPercentage)
    {}

    MarginPercentage( int tP, int rP , int bP, int lP) :
        topPercentage(tP),
        rightPercentage(rP),
        bottomPercentage(bP),
        leftPercentage(lP)
    {}
};

class ViewerAppearancePopup : public ViewerMenuPopUp, protected Ui::ViewerAppearancePopup

{
    Q_OBJECT

public:

    enum eMargin
    {
        MARGIN_NONE = -1,
        MARGIN_MIN = 0,
        MARGIN_MEDIUM,
        MARGIN_MAX
    };

    enum eSpacing
    {
        SPACING_NONE = -1,
        SPACING_MIN,
        SPACING_MEDIUM,
        SPACING_MAX
    };

    enum eJustify
    {
        JUSTIFY_NONE = -1,
        JUSTIFY_ALIGN,
        LEFT_ALIGN,
        DEFAULT_ALIGN
    };

    ViewerAppearancePopup( Viewer* viewer );
    virtual ~ViewerAppearancePopup();

    virtual void                        setup                               ();
    virtual void                        start                               ();
    virtual void                        stop                                ();

    inline int                          getFontSizeId                       () const { return m_currentFontSizeId; }

    void                                applyMargin                         ();

public slots:
    void                                pdfZoomLevelChange                  (int newZoomLevel);

protected slots:
    void                                setFontTypeSelected                 ();
    void                                setFontSizeSelected                 ();
    void                                setJustifySelected                  ();
    void                                setPageModeSelected                 ();
    void                                setMarginSelected                   ();
    void                                setSpacingSelected                  ();
    void                                setDefaultEditorSettings            ();

    void                                selectFontType                      (int);
    void                                selectFontSize                      (int);
    void                                selectMargin                        (int);
    void                                selectSpacing                       (int);
    void                                selectEditorDefaults                ();
    void                                selectImagesMode                    ();
    void                                selectJustify                       ( int );

protected:
    virtual void                        paintEvent                          ( QPaintEvent* );

    int                                 getFontTypeIdByFile                 ( const QString& ) const;
    int                                 getFontTypeIdByFamily               ( const QString& fontFamilyName) const;
    double                              getScaleFactor                      () const;

    void                                setupBook                           ( const BookInfo* );
    void                                setupUiButtons                      ();

    void                                applySizeChange                     ();

    void                                savePageMode                        ();

    QMap<eMargin, MarginPercentage>     m_marginMap;
    QStringList                         m_spacingList;

    PowerManagerLock*                   m_powerLock;

    bool                                m_defaultSettings;
    int                                 m_currentFontSizeId;
    int                                 m_currentFontNameId;
    int                                 m_currentPageMode;
    eMargin                             m_currentMarginValue;
    eSpacing                            m_currentSpacingValue;
    eJustify                            m_currentJustifyValue;

    QButtonGroup*                       g_SizeButtons;
    QButtonGroup*                       g_FontButtons;
    QButtonGroup*                       g_MarginButtons;
    QButtonGroup*                       g_SpacingButtons;
    QButtonGroup*                       g_JustifyButtons;
};

#endif // VIEWERAPPEARANCEPOPUP_H
