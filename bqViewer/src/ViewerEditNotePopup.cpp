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

#include "ViewerEditNotePopup.h"

#include "Viewer.h"
#include "Keyboard.h"

#include <QDebug>
#include <QPainter>

ViewerEditNotePopup::ViewerEditNotePopup(QWidget *parent): PopUp(parent)
{
    qDebug() << Q_FUNC_INFO;
    setupUi(this);
    connect(closeBtn,    SIGNAL(clicked()), this, SIGNAL(hideMe()));

    noteTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

ViewerEditNotePopup::~ViewerEditNotePopup()
{
    qDebug() << Q_FUNC_INFO;
}

void ViewerEditNotePopup::setUpCreateNote()
{
    titlePopup->setText(tr("Create note"));
    noteTextEdit->clear();
    noteTextEdit->setReadOnly(false);

    Screen::getInstance()->queueUpdates();

    Viewer* parentViewer = (Viewer*)parentWidget();
    parentViewer->showElement(this);

    QTextCursor noteCursor = noteTextEdit->textCursor();
    noteCursor.setPosition(0);
    noteTextEdit->setFocus();
    noteTextEdit->setTextCursor(noteCursor);
    noteTextEdit->ensureCursorVisible();

    Keyboard* keyboard = parentViewer->showKeyboard(tr("Save"));
    Screen::getInstance()->flushUpdates();
    keyboard->handleTextEdit(noteTextEdit);
    connect(keyboard, SIGNAL(actionRequested()), this, SIGNAL(saveNote()));
}

void ViewerEditNotePopup::setUpEditNote( const QString& note )
{
    titlePopup->setText(tr("Edit note"));
    noteTextEdit->setText(note);
    noteTextEdit->setReadOnly(false);
    QTextCursor noteCursor = noteTextEdit->textCursor();
    noteCursor.setPosition(noteTextEdit->toPlainText().length());

    noteTextEdit->setFocus();
    noteTextEdit->setTextCursor(noteCursor);
    noteTextEdit->ensureCursorVisible();

    Viewer* parentViewer = (Viewer*)parentWidget();
    parentViewer->showElement(this);

    Keyboard* keyboard = parentViewer->showKeyboard(tr("Save"));
    keyboard->handleTextEdit(noteTextEdit);
    connect(keyboard, SIGNAL(actionRequested()), this, SIGNAL(saveNote()));
}

QString ViewerEditNotePopup::getNoteText() const
{
    return noteTextEdit->toPlainText();
}

void ViewerEditNotePopup::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
