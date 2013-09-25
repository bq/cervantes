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

#include "KeyboardCharacterHandler.h"

#include <QKeyEvent>

#include "MyQLineEdit.h"
#include "MyQTextEdit.h"

KeyboardCharacterHandler_QLineEdit::KeyboardCharacterHandler_QLineEdit(MyQLineEdit* edit) : m_edit(edit)
{}

void KeyboardCharacterHandler_QLineEdit::addCharacter(const QString& character)
{
    m_edit->insert(character);
}

void KeyboardCharacterHandler_QLineEdit::removeCharacter()
{
    m_edit->backspace();
}

void KeyboardCharacterHandler_QLineEdit::clearText()
{
    m_edit->clear();
}

void KeyboardCharacterHandler_QLineEdit::cursorRight()
{
    m_edit->cursorForward(false, 1);
}

void KeyboardCharacterHandler_QLineEdit::cursorLeft()
{
    m_edit->cursorBackward(false, 1);
}

KeyboardCharacterHandler_QTextEdit::KeyboardCharacterHandler_QTextEdit(MyQTextEdit* edit) : m_edit(edit)
{

}

void KeyboardCharacterHandler_QTextEdit::addCharacter(const QString& character)
{
    m_edit->insertPlainText(character);
}

void KeyboardCharacterHandler_QTextEdit::removeCharacter()
{
    m_edit->textCursor().deletePreviousChar();
}

void KeyboardCharacterHandler_QTextEdit::clearText()
{
    m_edit->clear();
}

void KeyboardCharacterHandler_QTextEdit::newLine()
{
    addCharacter("\n");
}

void KeyboardCharacterHandler_QTextEdit::cursorRight()
{
    moveCursorPosition(QTextCursor::Right);
}

void KeyboardCharacterHandler_QTextEdit::cursorLeft()
{
    moveCursorPosition(QTextCursor::Left);
}

void KeyboardCharacterHandler_QTextEdit::moveCursorPosition(QTextCursor::MoveOperation moveOperation)
{
    QTextCursor textCursor = m_edit->textCursor();
    textCursor.movePosition(moveOperation);
    m_edit->setTextCursor(textCursor);
}
