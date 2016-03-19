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

#ifndef KEYBOARDBUTTON_H
#define KEYBOARDBUTTON_H

#include <QLabel>
#include <QList>
#include <QTimer>

#define BUTTONS_PER_ROW_MAX 7

class Ui_KeyboardLayer;
class BaseKeyboard;
class QTimerEvent;
class QMouseEvent;

class KeyboardButton : public QLabel
{
    Q_OBJECT

public:
    KeyboardButton(QWidget* parent);
    KeyboardButton(QWidget*, QString);
    virtual ~KeyboardButton();
    virtual void setText(const QString text);

    void setKeyboard(BaseKeyboard *mykbd)               { m_kbd = mykbd; }

    bool isSpecialKey()                             { return b_special; }
    void setSpecialKey(bool special)                { b_special = special; }
    bool hasPressStyle()                            { return b_pressStyle; }
    void setPressStyle(bool press)                  { b_pressStyle = press; }
    bool hasAltKeys()                               { return m_altKeys.size() > 0; }
    void setAltKeys(QStringList, QStringList, BaseKeyboard* kbd = NULL);
    QList<KeyboardButton*> getAltKeys()             { return m_altKeys; }

    bool isDehighlightDisabled()                    { return b_disableDehighlight; }
    void setDehighlightDisabled(bool disable)       { b_disableDehighlight = disable; }

public slots:
    void pressBtn();
    void releaseBtn();
    void resetButton();
    void dehighlightButton();

signals:
    void pressed();
    void released();

protected slots:
    void hideLayer();
    void showLayer();

protected:
    void init();
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);
    bool b_special;
    bool b_pressStyle;
    bool b_pressed;
    bool b_disableDehighlight;
    BaseKeyboard* m_kbd;
    QList<KeyboardButton*> m_altKeys;
    Ui_KeyboardLayer* m_ui_kbdLayer;
    QWidget* m_kbdLayer;
    QTimer m_timer_longpress;
    
    QImage viewInactive;
    QImage viewActive;

};

#endif // KEYBOARDBUTTON_H
