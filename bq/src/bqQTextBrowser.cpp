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

#include "bqQTextBrowser.h"

#include <QDebug>
#include <QMouseEvent>
#include <QTextDocumentFragment>
#include <QAbstractTextDocumentLayout>
#include <QMargins>
#include <QScrollBar>
#include "MouseFilter.h"
#include "QBookApp.h"

#define MAX_ERROR_X 10
#define MAX_ERROR_Y 8

bqQTextBrowser::bqQTextBrowser(QWidget* parent) : QTextBrowser(parent)
{
    qDebug() << Q_FUNC_INFO;
    horizontalScrollBar()->hide();
    verticalScrollBar()->hide();
    m_mouseFilter = new MouseFilter(this);
    this->installEventFilter(m_mouseFilter);
    setTappable(false);
    setLinksAccepted(false);

    QString css;
    QFont font("Lato");

    if(QBook::getInstance()->getResolution() == QBook::RES758x1024)
    {
        css  =  "* {"
                "font-size: 24pt ;"
                "font-family: 'Lato';"
                "padding-bottom:5pt;"
                "color:#000 !important;"
                "}";

        font.setPointSize(24);
    }
    else
    {
        css  = "* {"
               "font-size: 19pt ;"
               "font-family: 'Lato';"
               "padding-bottom:3pt;"
               "color:#000 !important;"
                "}";

        font.setPointSize(19);
    }

   // Create a QTextDocument with the defined HTML and CSS
   m_document = new QTextDocument;
   m_document->setDefaultStyleSheet(css);
   m_document->setDefaultFont(font);
}

bqQTextBrowser::~bqQTextBrowser()
{
    delete m_mouseFilter;
    m_mouseFilter = NULL;
}

void bqQTextBrowser::setTappable(bool tappable)
{
    qDebug() << Q_FUNC_INFO << tappable;
    m_tappable = tappable;
}

void bqQTextBrowser::setLinksAccepted(bool accepted)
{
    qDebug() << Q_FUNC_INFO << accepted;
    m_linkAccepted = accepted;
}

void bqQTextBrowser::handleTap(TouchEvent *event)
{
    qDebug() << Q_FUNC_INFO <<"posiciones: " << event->pos().x() << " y: " << event->pos().y();

    if(!m_tappable)
        emit pressed();

    // Fine tuning of touch precission. We take off the parent widget top margin to get more precission.
    QPoint point(0, 0);
    qDebug() << Q_FUNC_INFO << height();

    point.setX(event->pos().x() - contentsMargins().left());
    point.setY(event->pos().y() - contentsMargins().top());

    QTextCursor cursor = cursorForPosition(point);
    // This detects when we press inside the TextBrowser but outside the bounds of the words.
    if(document()->documentLayout()->hitTest( point, Qt::ExactHit ) == -1)
    {
        QPoint point2 = cursorRect(cursor).center();
        qDebug() << Q_FUNC_INFO << "posiciones en el error x: " << point2.x() << " y: " << point2.y();
        if(fabs(point2.x() - point.x())> MAX_ERROR_X || fabs(point2.y() - point.y()) > MAX_ERROR_Y )
            return;
    }
    cursor.select(QTextCursor::WordUnderCursor);

    if (!cursor.selectedText().isEmpty())
    {
        qDebug() << Q_FUNC_INFO << "cursor.selectedText(): " << cursor.selectedText();

        int pos = cursor.position();
        QChar currentChar = cursor.document()->characterAt(pos);
        while(currentChar.isLetter() || currentChar == '\'')
        {
            qDebug() << " current Char: " << currentChar;

            cursor.setPosition(++pos, QTextCursor::KeepAnchor);
            currentChar = cursor.document()->characterAt(pos);
        }
        QString context;
        QString wordToSearch = cursor.selectedText();
        cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
        cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::MoveAnchor);
        cursor.select(QTextCursor::WordUnderCursor);

        if (!cursor.selectedText().isEmpty())
        {
            qDebug() << Q_FUNC_INFO << "cursor.selectedText(): " << cursor.selectedText();

            int pos = cursor.position();
            QChar currentChar = cursor.document()->characterAt(pos);
            while(currentChar.isLetter() || currentChar == '\'')
            {
                qDebug() << " current Char: " << currentChar;

                cursor.setPosition(++pos, QTextCursor::KeepAnchor);
                currentChar = cursor.document()->characterAt(pos);
            }
        }
        context = cursor.selectedText();
        qDebug() << Q_FUNC_INFO << "previous word: " << context;
        cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        cursor.movePosition(QTextCursor::NextWord, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::NextWord, QTextCursor::MoveAnchor);
        cursor.select(QTextCursor::WordUnderCursor);

        if (!cursor.selectedText().isEmpty())
        {
            qDebug() << Q_FUNC_INFO << "cursor.selectedText(): " << cursor.selectedText();

            int pos = cursor.position();
            QChar currentChar = cursor.document()->characterAt(pos);
            while(currentChar.isLetter() || currentChar == '\'')
            {
                qDebug() << " current Char: " << currentChar;

                cursor.setPosition(++pos, QTextCursor::KeepAnchor);
                currentChar = cursor.document()->characterAt(pos);
            }
        }
        context = context + " <w> " + cursor.selectedText();
        qDebug() << Q_FUNC_INFO << "word and context: " << wordToSearch << context;
        emit wordClicked(wordToSearch, context);

        event->accept();
    }
}

void bqQTextBrowser::customEvent(QEvent* received)
{
    qDebug() << Q_FUNC_INFO;


    if (received->type() != MouseFilter::TOUCH_EVENT){
        qDebug() << Q_FUNC_INFO << "UNEXPECTED TYPE";
        QWidget::customEvent(received);
        return;
    }


    TouchEvent *event = static_cast<TouchEvent*>(received);
    switch(event->touchType()){

    case MouseFilter::TAP:
        qDebug() << Q_FUNC_INFO << "TAP";

        if(m_linkAccepted)
        {
            if(!anchorAt(event->pos()).isEmpty())
                handleLinkPressed(anchorAt(event->pos()));
            return;
        }else
            handleTap(event);

        break;

    case MouseFilter::LONGPRESS_START:{
        qDebug() << Q_FUNC_INFO << "LONGPRESS_START";
        return;
    }

    case MouseFilter::LONGPRESS_END:{
        qDebug() << Q_FUNC_INFO << "LONGPRESS_END";
        return;
    }

    case MouseFilter::SWIPE_R2L:
    case MouseFilter::SWIPE_L2R:
    case MouseFilter::SWIPE_D2U:
    case MouseFilter::SWIPE_U2D:
        qDebug() << Q_FUNC_INFO << "SWIPE";                    ;
        QBookApp::instance()->emitSwipe(event->touchType());
        return;

    default:
        qDebug() << "--->" << Q_FUNC_INFO << "UNEXPECTED EXIT";
        break;
    }
    QWidget::customEvent(received);
}

void bqQTextBrowser::applyDocument( const QString& html )
{
    qDebug() << Q_FUNC_INFO;
    QString finalHtml = html;
    finalHtml.replace("font-family: Verdana,Arial,Helvetica,sans-serif; font-size: 0.7em; margin: 6px 0pt; padding: 0pt; text-align: left;","");
    finalHtml.replace("font-family: Verdana,Arial,Helvetica,sans-serif; font-size: 0.7em; margin: 6px 0pt; padding: 0pt; text-align: left;","");

    m_document->setHtml(finalHtml); // binds the HTML to the QTextDocument
    setDocument(m_document);
}

void bqQTextBrowser::applyWikipediaInfoStyle( QString& content )
{
    qDebug() << Q_FUNC_INFO;
    m_document->setHtml(content); // binds the HTML to the QTextDocument
    setDocument(m_document);

}

void bqQTextBrowser::handleLinkPressed(const QUrl & url)
{
    QString tokenToSearch = QUrl::fromPercentEncoding(url.toString().toUtf8());
    qDebug() << Q_FUNC_INFO << tokenToSearch;

    emit linkPressed(tokenToSearch);
}

void bqQTextBrowser::disableInteractionFlags()
{
    qDebug() << Q_FUNC_INFO;
    setTextInteractionFlags(Qt::NoTextInteraction);
}
