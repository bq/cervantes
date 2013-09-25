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

#ifndef KEYBOARDCHARACTERHANDLER_H
#define KEYBOARDCHARACTERHANDLER_H

#include <QTextCursor>

// Predeclaration
class MyQLineEdit;
class MyQTextEdit;

class KeyboardCharacterHandler
{
    public:
        virtual void addCharacter(const QString& character) = 0;
        virtual void removeCharacter() = 0;
        virtual void clearText() = 0;
        virtual void newLine() = 0;
        virtual void cursorRight() = 0;
        virtual void cursorLeft() = 0;
};

class KeyboardCharacterHandler_JustSignal : public KeyboardCharacterHandler
{
    public:
        virtual void addCharacter(const QString&) {}
        virtual void removeCharacter() {}
        virtual void clearText() {}
        virtual void newLine() {}
        virtual void cursorRight() {}
        virtual void cursorLeft() {}
};

class KeyboardCharacterHandler_QLineEdit : public KeyboardCharacterHandler
{
    public:
        KeyboardCharacterHandler_QLineEdit(MyQLineEdit* edit);

        virtual void addCharacter(const QString& character);
        virtual void removeCharacter();
        virtual void clearText();
        virtual void newLine() {}
        virtual void cursorRight();
        virtual void cursorLeft();

    private:
        MyQLineEdit*    m_edit;
};

class KeyboardCharacterHandler_QTextEdit : public KeyboardCharacterHandler
{
    public:
        KeyboardCharacterHandler_QTextEdit(MyQTextEdit* edit);

        virtual void addCharacter(const QString& character);
        virtual void removeCharacter();
        virtual void clearText();
        virtual void newLine();
        virtual void cursorRight();
        virtual void cursorLeft();

    private:
        MyQTextEdit*    m_edit;

        void moveCursorPosition(QTextCursor::MoveOperation);
};

#endif // KEYBOARDCHARACTERHANDLER_H
