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

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QWidget>
#include <QQueue>
#include "ui_Keyboard.h"

#include "QBook.h"
#include "Screen.h"
#include "PowerManager.h"

#define MAX_KEYS_NO_REFRESH 15

// Predeclaration
class QChar;
class QString;
class QSignalMapper;
class QButtonGroup;
class MyQLineEdit;
class MyQTextEdit;
class KeyboardCharacterHandler;
class KeyboardButton;
class Keyboard;

class BaseKeyboard : public QWidget, protected Ui::Keyboard
{
    Q_OBJECT

public:
    BaseKeyboard(QWidget*, PowerManagerLock *);
    virtual ~BaseKeyboard();
    void                                    clearText                       ();
    void                                    setActionKeyText                ( const QString& text );
    void                                    setKeyboardCharacterHandler     (KeyboardCharacterHandler *newKeyboardCharacterHandler) { m_pKeyboardCharacterHandler = newKeyboardCharacterHandler;}

signals:
    void                                    pressedChar                     ( const QString & );// TODO: Pasarle un const QString&
    void                                    backSpacePressed                ();
    void                                    newLinePressed                  ();
    void                                    actionRequested                 ();
    void                                    cursorLeftPressed               ();
    void                                    cursorRightPressed              ();
    void                                    hideLayer                       ();
    void                                    switchUpperLowerKeyboard        ();
    void                                    switchNormalSymbolKeyboard      ();

public slots:
    virtual void                            setVisible                      (bool);

protected slots:
    void                                    handleKey                       ();

protected:
    void                                    handleSpecialCharacter          ( KeyboardButton* );
    void                                    changeKeyboard                  ( KeyboardButton* );
    void                                    initKeys                        ();
    virtual void                            setKeyboard                     () = 0;
    virtual void                            prepareAltKeys                  () = 0;

    QString                                 m_specialTextButtonLower;
    QString                                 m_specialTextButtonUpper;
    KeyboardCharacterHandler*               m_pKeyboardCharacterHandler;

    PowerManagerLock*                       m_powerLock;
};

class LowerCaseKeyboard : public BaseKeyboard
{
    Q_OBJECT

public:
    LowerCaseKeyboard(QWidget *, PowerManagerLock *);
    void                                    setComma                        (QString);

protected:
    virtual void                            setKeyboard                     ();
    virtual void                            prepareAltKeys                  ();
};

class UpperCaseKeyboard : public BaseKeyboard
{
    Q_OBJECT

public:

    UpperCaseKeyboard(QWidget *, PowerManagerLock *);
    void                                    setComma                        (QString);

protected:
    virtual void                            setKeyboard                     ();
    virtual void                            prepareAltKeys                  ();
};

class SymbolKeyboard : public BaseKeyboard
{
    Q_OBJECT
public:
    SymbolKeyboard(QWidget *, PowerManagerLock *);
    void                                    setChangeKeyboardButton         (const QString &);
    void                                    setL                            (QString);

protected:
    virtual void                            setKeyboard                     ();
    virtual void                            prepareAltKeys                  ();
};

class Keyboard : public QWidget
{
    Q_OBJECT

public:

    enum MODE   {
        NORMAL,
        EMAIL
    };

    static Keyboard *                       getInstance                     ();
    static void                             staticInit                      ( QWidget *parent = 0 );
    static void                             staticDone                      ();
    bool                                    isUpperCaseKeyboardVisible      () { return m_currentKeyboard == m_upperCaseKeyboard; }
    bool                                    isLowerCaseKeybardVisible       () { return m_currentKeyboard == m_lowerCaseKeyboard; }
    bool                                    isSymbolKeyboardVisible         () { return m_currentKeyboard == m_symbolKeyboard; }
    void                                    handleMyQLineEdit               ( MyQLineEdit* qLine );
    void                                    handleTextEdit                  ( MyQTextEdit* qText );
    bool                                    isVisible                       () const { return m_currentKeyboard->isVisible(); }
    void                                    setActionKeyText                ( const QString& text );

    void                                    justSendSignals                 ();
    void                                    startKeyboardMode               ();
    void                                    stopKeyboardMode                ();
    void                                    clearText                       ()  { m_currentKeyboard->clearText();}
    void                                    queueRefresh                    ( const QImage& image, const QPoint& point);
    bool                                    isCapitalLetter                 () {return (m_currentKeyboard == m_upperCaseKeyboard || m_lastKeyboard == m_upperCaseKeyboard);}
    void                                    move                            ( int x, int y );

public slots:
    void                                    show                            ( Keyboard::MODE keyboardMode );
    void                                    hide                            ();
    void                                    cleanQueue                      ();
    void                                    switchUpperLowerKeyboard        ();
    void                                    switchNormalSymbolKeyboard      ();

protected slots:
    void                                    refreshKeys                     ();
    /* http://qt-project.org/forums/viewthread/7340 */
    void                                    paintEvent                      (QPaintEvent *);

protected:
    struct RefreshQueued {
        QImage imageKey;
        QPoint pointKey;

        RefreshQueued( const QImage& image, const QPoint& point ) : imageKey(image), pointKey(point) {}
    };

    Keyboard(QWidget * parent = NULL);
    virtual ~Keyboard();

    LowerCaseKeyboard *                     m_lowerCaseKeyboard;
    UpperCaseKeyboard *                     m_upperCaseKeyboard;
    SymbolKeyboard *                        m_symbolKeyboard;

    BaseKeyboard *                          m_currentKeyboard;
    BaseKeyboard *                          m_lastKeyboard;

    QList<RefreshQueued>                    m_queue;
    QTimer                                  m_timer_queue;

    Screen::ScreenMode                      m_previousRefresh;
    KeyboardCharacterHandler*               m_pKeyboardCharacterHandler;
    PowerManagerLock*                       m_powerLock;

private:
    static Keyboard *                       s_instance;

signals:
    void                                    actionRequested                 ();
    void                                    pressedChar                     ( const QString & );// TODO: Pasarle un const QString&
    void                                    backSpacePressed                ();
    void                                    newLinePressed                  ();
    void                                    cursorLeftPressed               ();
    void                                    cursorRightPressed              ();
};

#endif // KEYBOARD_H
