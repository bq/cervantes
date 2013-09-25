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

#include "Keyboard.h"
#include "KeyboardCharacterHandler.h"
#include "KeyboardButton.h"
#include "Screen.h"
#include "PowerManager.h"

#include <QDebug>
#include <QList>
#include <QStringList>
#include <QButtonGroup>
#include <QFile>
#include <QPainter>

/**
  * use http://www.ascii.cl/htmlcodes.htm as ascii reference
  * NOTE: Nowadays 06/09/12 the € symbol are not available at lineEdit font and we don't know howto change font
  */

BaseKeyboard::BaseKeyboard(QWidget *parent, PowerManagerLock *powerLock) :
    QWidget(parent, Qt::Tool | Qt::FramelessWindowHint)
  , m_pKeyboardCharacterHandler(NULL)
  , m_powerLock(powerLock)
{
        qDebug() << Q_FUNC_INFO << parent;
        setupUi(this);

        QWidget::hide();

        initKeys();

        // Signals
        QList<KeyboardButton*> buttons = findChildren<KeyboardButton*>();
        for(int i = 0; i < buttons.size(); ++i)
        {
            KeyboardButton *button = buttons.at(i);
            button->setKeyboard(this);
            connect(button,SIGNAL(released()),this,SLOT(handleKey()), Qt::UniqueConnection);
            connect(button,SIGNAL(released()),this,SIGNAL(hideLayer()), Qt::UniqueConnection);
            connect(button, SIGNAL(pressed()), this, SIGNAL(hideLayer()), Qt::UniqueConnection);
        }


        QFile fileSpecific(":/res/ui_styles.qss");
        QFile fileCommons(":/res/ui_styles_generic.qss");
        fileSpecific.open(QFile::ReadOnly);
        fileCommons.open(QFile::ReadOnly);

        QString styles = QLatin1String(fileSpecific.readAll() + fileCommons.readAll());
        setStyleSheet(styles);
        space->setDehighlightDisabled(true);

}

BaseKeyboard::~BaseKeyboard()
{
    qDebug() << "--->" << Q_FUNC_INFO;
}

void BaseKeyboard::handleKey()
{
    qDebug() << "--->" << Q_FUNC_INFO;

    m_powerLock->activate();

    KeyboardButton *button = (KeyboardButton*)sender();

    if(!button->isSpecialKey()){
        qDebug() << Q_FUNC_INFO << "Character: " << button->text();
        m_pKeyboardCharacterHandler->addCharacter(button->text());
        emit pressedChar(button->text());
    }
    else
    {
        handleSpecialCharacter(button);
    }
}

void BaseKeyboard::handleSpecialCharacter(KeyboardButton* button)
{
    qDebug() << Q_FUNC_INFO << button;

    if (button == enter)
    {
        emit actionRequested();
    }
    else if(button == newLineBtn)
    {
        m_pKeyboardCharacterHandler->newLine();
        emit newLinePressed();
    }
    else if(button == cursorRight)
    {
        m_pKeyboardCharacterHandler->cursorRight();
        emit cursorRightPressed();
    }
    else if(button == cursorLeft)
    {
        m_pKeyboardCharacterHandler->cursorLeft();
        emit cursorLeftPressed();
    }
    else if(button == space)
    {
        m_pKeyboardCharacterHandler->addCharacter(" ");

        emit pressedChar(" ");
    }
    else if(button == deleteChar)
    {
        m_pKeyboardCharacterHandler->removeCharacter();
        emit backSpacePressed();
    }
    else
    {
        Screen::getInstance()->setMode(Screen::MODE_SAFE,true,Q_FUNC_INFO);
        Screen::getInstance()->lockScreen();
        changeKeyboard(button);
        Screen::getInstance()->releaseScreen();
    }
}

void BaseKeyboard::changeKeyboard(KeyboardButton* button){
    qDebug() << Q_FUNC_INFO << button;
    if(button == shift) {
        emit switchUpperLowerKeyboard();

    } else if(button == specialChar)
        emit switchNormalSymbolKeyboard();
}

void BaseKeyboard::clearText()
{
    qDebug() << "--->" << Q_FUNC_INFO;
    m_pKeyboardCharacterHandler->clearText();
}

void BaseKeyboard::setActionKeyText(const QString& text)
{
    qDebug() << Q_FUNC_INFO << text;
    enter->setText(text);
}

void BaseKeyboard::initKeys()
{
    qDebug() << Q_FUNC_INFO;
    // Init special keys
    shift->setSpecialKey(true);
    specialChar->setSpecialKey(true);
    space->setSpecialKey(true);
    cursorLeft->setSpecialKey(true);
    cursorRight->setSpecialKey(true);
    enter->setSpecialKey(true);
    newLineBtn->setSpecialKey(true);
    deleteChar->setSpecialKey(true);

    // Remove press style from shift, new line, action and delete keys
    shift->setPressStyle(false);
    enter->setPressStyle(false);
    newLineBtn->setPressStyle(false);
    deleteChar->setPressStyle(false);
    cursorLeft->setPressStyle(false);
    cursorRight->setPressStyle(false);
}

void BaseKeyboard::setVisible(bool visible)
{
    qDebug() << Q_FUNC_INFO << visible;
    if (visible == false)
    {
        emit hideLayer();
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, Q_FUNC_INFO);
        QWidget::setVisible(false);
    }
    else
    {
        Screen::getInstance()->setMode(Screen::MODE_SAFE, true, Q_FUNC_INFO);
        QWidget::setVisible(true);
    }
}

LowerCaseKeyboard::LowerCaseKeyboard(QWidget *parent, PowerManagerLock *powerLock)
    : BaseKeyboard(parent, powerLock)
{
    qDebug() << Q_FUNC_INFO << parent;
    setKeyboard();
    prepareAltKeys();
}
void LowerCaseKeyboard::setComma(QString value)
{
        comma->setText(value);
}

/*virtual*/ void LowerCaseKeyboard::setKeyboard()
{
    qDebug() << Q_FUNC_INFO << "To lower keyboard";

    a->setText("a"); b->setText("b"); c->setText("c"); d->setText("d"); e->setText("e"); f->setText("f");
    g->setText("g"); h->setText("h"); i->setText("i"); j->setText("j"); k->setText("k"); l->setText("l");
    m->setText("m"); n->setText("n"); o->setText("o"); p->setText("p"); q->setText("q"); r->setText("r");
    s->setText("s"); t->setText("t"); u->setText("u"); v->setText("v"); w->setText("w"); Ui_Keyboard::x->setText("x");
    Ui_Keyboard::y->setText("y"); z->setText("z");

    num1->setText("1");
    num2->setText("2");
    num3->setText("3");
    num4->setText("4");
    num5->setText("5");
    num6->setText("6");
    num7->setText("7");
    num8->setText("8");
    num9->setText("9");
    num0->setText("0");
    comma->setText(",");
    dot->setText(".");
}

/*virtual*/ void LowerCaseKeyboard::prepareAltKeys()
{
    qDebug() << Q_FUNC_INFO;

    // symbols. no capital version
    QStringList buttonsInSymUp;
    buttonsInSymUp.append(";");
    buttonsInSymUp.append("(");
    buttonsInSymUp.append(")");
    buttonsInSymUp.append("-");
    buttonsInSymUp.append("+");
    buttonsInSymUp.append(QChar(161)); // ¡
    buttonsInSymUp.append("!");

    QStringList buttonsInSymDown;
    buttonsInSymDown.append(":");
    buttonsInSymDown.append("/");
    buttonsInSymDown.append("@");
    buttonsInSymDown.append("'");
    buttonsInSymDown.append(QChar(34)); // "
    buttonsInSymDown.append(QChar(191)); // ¿
    buttonsInSymDown.append("?");

    comma->setAltKeys(buttonsInSymUp, buttonsInSymDown, this);

    QStringList emptyList;

    // E
    QStringList buttonsInEUp;
    buttonsInEUp.append(QChar(232));
    buttonsInEUp.append(QChar(233));
    buttonsInEUp.append(QChar(234));
    buttonsInEUp.append(QChar(235));
    e->setAltKeys(buttonsInEUp, emptyList, this);

    // U
    QStringList buttonsInUUp;
    buttonsInUUp.append(QChar(249));
    buttonsInUUp.append(QChar(250));
    buttonsInUUp.append(QChar(251));
    buttonsInUUp.append(QChar(252));
    u->setAltKeys(buttonsInUUp, emptyList, this);

    // I
    QStringList buttonsInIUp;
    buttonsInIUp.append(QChar(236));
    buttonsInIUp.append(QChar(237));
    buttonsInIUp.append(QChar(238));
    buttonsInIUp.append(QChar(239));
    i->setAltKeys(buttonsInIUp, emptyList, this);

    //O
    QStringList buttonsInOUp;
    buttonsInOUp.append(QChar(242));
    buttonsInOUp.append(QChar(243));
    buttonsInOUp.append(QChar(244));
    buttonsInOUp.append(QChar(245));
    buttonsInOUp.append(QChar(246));
//        buttonsInOUp.append(QString(QChar(339))); Ommited. Problem showing this char (œ)
    buttonsInOUp.append(QChar(248));
    o->setAltKeys(buttonsInOUp, emptyList, this);

    //A
    QStringList buttonsInAUp;
    buttonsInAUp.append(QChar(224));
    buttonsInAUp.append(QChar(225));
    buttonsInAUp.append(QChar(226));
    buttonsInAUp.append(QChar(227));
    buttonsInAUp.append(QChar(228));
    buttonsInAUp.append(QChar(229));
    buttonsInAUp.append(QChar(230));
    a->setAltKeys(buttonsInAUp, emptyList, this);

    //S
    QStringList buttonsInSUp;
    buttonsInSUp.append(QChar(167)); // § don't have a clear capital version
    buttonsInSUp.append(QChar(223));
    s->setAltKeys(buttonsInSUp, emptyList, this);

    // C
    QStringList buttonsInCUp;
    buttonsInCUp.append(QChar(231));
    c->setAltKeys(buttonsInCUp, emptyList, this);

    // N
    QStringList buttonsInNUp;
    buttonsInNUp.append(QChar(241));
    n->setAltKeys(buttonsInNUp, emptyList, this);

}

UpperCaseKeyboard::UpperCaseKeyboard(QWidget *parent, PowerManagerLock *powerLock)
    : BaseKeyboard(parent, powerLock)
{
    qDebug() << Q_FUNC_INFO << parent;
    setKeyboard();
    prepareAltKeys();
}

/*virtual*/ void UpperCaseKeyboard::setKeyboard()
{
    qDebug() << Q_FUNC_INFO << "To upper keyboard";

    a->setText("A"); b->setText("B"); c->setText("C"); d->setText("D"); e->setText("E"); f->setText("F");
    g->setText("G"); h->setText("H"); i->setText("I"); j->setText("J"); k->setText("K"); l->setText("L");
    m->setText("M"); n->setText("N"); o->setText("O"); p->setText("P"); q->setText("Q"); r->setText("R");
    s->setText("S"); t->setText("T"); u->setText("U"); v->setText("V"); w->setText("W"); Ui_Keyboard::x->setText("X");
    Ui_Keyboard::y->setText("Y"); z->setText("Z");

    num1->setText("1");
    num2->setText("2");
    num3->setText("3");
    num4->setText("4");
    num5->setText("5");
    num6->setText("6");
    num7->setText("7");
    num8->setText("8");
    num9->setText("9");
    num0->setText("0");
    comma->setText(",");
    dot->setText(".");
}

void UpperCaseKeyboard::setComma(QString value)
{
        comma->setText(value);
}

/*virtual*/ void UpperCaseKeyboard::prepareAltKeys()
{
    qDebug() << Q_FUNC_INFO;

    // symbols. no capital version
    QStringList buttonsInSymUp;
    buttonsInSymUp.append(";");
    buttonsInSymUp.append("(");
    buttonsInSymUp.append(")");
    buttonsInSymUp.append("-");
    buttonsInSymUp.append("+");
    buttonsInSymUp.append(QChar(161)); // ¡
    buttonsInSymUp.append("!");

    QStringList buttonsInSymDown;
    buttonsInSymDown.append(":");
    buttonsInSymDown.append("/");
    buttonsInSymDown.append("@");
    buttonsInSymDown.append("'");
    buttonsInSymDown.append(QChar(34)); // "
    buttonsInSymDown.append(QChar(191)); // ¿
    buttonsInSymDown.append("?");

    comma->setAltKeys(buttonsInSymUp, buttonsInSymDown, this);

    QStringList emptyList;

    // E
    QStringList buttonsInEUp;
    buttonsInEUp.append(QChar(200));
    buttonsInEUp.append(QChar(201));
    buttonsInEUp.append(QChar(202));
    buttonsInEUp.append(QChar(203));
    e->setAltKeys(buttonsInEUp, emptyList, this);

    // U
    QStringList buttonsInUUp;
    buttonsInUUp.append(QChar(217));
    buttonsInUUp.append(QChar(218));
    buttonsInUUp.append(QChar(219));
    buttonsInUUp.append(QChar(220));
    u->setAltKeys(buttonsInUUp, emptyList, this);

    // I
    QStringList buttonsInIUp;
    buttonsInIUp.append(QChar(204));
    buttonsInIUp.append(QChar(205));
    buttonsInIUp.append(QChar(206));
    buttonsInIUp.append(QChar(207));
    i->setAltKeys(buttonsInIUp, emptyList, this);

    //O
    QStringList buttonsInOUp;
    buttonsInOUp.append(QChar(210));
    buttonsInOUp.append(QChar(211));
    buttonsInOUp.append(QChar(212));
    buttonsInOUp.append(QChar(213));
    buttonsInOUp.append(QChar(214));
//        buttonsInOUp.append(QString(QChar(338))); Ommited. Problem showing this char (Œ)
    buttonsInOUp.append(QChar(216));
    o->setAltKeys(buttonsInOUp, emptyList, this);

    //A
    QStringList buttonsInAUp;
    buttonsInAUp.append(QChar(192));
    buttonsInAUp.append(QChar(193));
    buttonsInAUp.append(QChar(194));
    buttonsInAUp.append(QChar(195));
    buttonsInAUp.append(QChar(196));
    buttonsInAUp.append(QChar(197));
    buttonsInAUp.append(QChar(198));
    a->setAltKeys(buttonsInAUp, emptyList, this);

    //S
    QStringList buttonsInSUp;
    buttonsInSUp.append(QChar(167)); // § don't have a clear capital version
    buttonsInSUp.append(QChar(223)); // ß don't have a capital version http://es.wikipedia.org/wiki/%C3%9F
    s->setAltKeys(buttonsInSUp, emptyList, this);

    // C
    QStringList buttonsInCUp;
    buttonsInCUp.append(QChar(199));
    c->setAltKeys(buttonsInCUp, emptyList, this);

    // N
    QStringList buttonsInNUp;
    buttonsInNUp.append(QChar(209));
    n->setAltKeys(buttonsInNUp, emptyList, this);
}

SymbolKeyboard::SymbolKeyboard(QWidget *parent, PowerManagerLock *powerLock)
    : BaseKeyboard(parent, powerLock)
{
    qDebug() << Q_FUNC_INFO << parent;
    setKeyboard();
    prepareAltKeys();
}

/*virtual*/ void SymbolKeyboard::setKeyboard()
{
    qDebug() << Q_FUNC_INFO;

    setChangeKeyboardButton("a b c");

    num1->setText("*");
    num2->setText(QChar(38));   // &
    num3->setText("#");
    num4->setText("<");
    num5->setText(">"); //
    num6->setText("[");
    num7->setText("]");
    num8->setText("{");
    num9->setText("}");
    num0->setText("%");

    q->setText("_");
    w->setText(QChar(92)); // '\' character
    e->setText("|"); //
    r->setText(QChar(126)); // '~' character
    t->setText(QChar(0x00BF)); // '¿' character

    Ui::Keyboard::y->setText("?"); // Special notation. y is an attribute of every widget
    u->setText("$");
    i->setText(QChar(163)); // '£' character
    o->setText(QChar(165)); // '¥' character
    p->setText("=");

    a->setText("+");
    s->setText(QChar(247)); // '÷' character
    d->setText("-");
    f->setText(QChar(186)); // º
    g->setText(QChar(174)); // ®
    h->setText(QChar(169)); // ©
    j->setText(QChar(0x00A1)); // '¡' character
    k->setText("!");
    l->setText("@");

    z->setText("(");
    Ui::Keyboard::x->setText(")");
    c->setText("/");
    v->setText(QChar(183)); // · Middle dot
    b->setText(":");
    n->setText("'");
    m->setText(";");

    comma->setText("^");
    dot->setText(".");
}
void SymbolKeyboard::setL(QString value)
{
        l->setText(value);
}
/*virtual*/ void SymbolKeyboard::prepareAltKeys()
{
    qDebug() << Q_FUNC_INFO;

    QStringList emptyList;

    // º
    QStringList buttonsInFUp;
    buttonsInFUp.append(QChar(170)); // ª
    f->setAltKeys(buttonsInFUp, emptyList, this);
}

void SymbolKeyboard::setChangeKeyboardButton(const QString & text)
{
    specialChar->setText(text);
}

Keyboard* Keyboard::s_instance = NULL;

Keyboard *Keyboard::getInstance()
{
    return s_instance;
}

void Keyboard::staticInit(QWidget* parent)
{
    qDebug() << Q_FUNC_INFO;
    if (!s_instance)
        s_instance = new Keyboard(parent);
}

void Keyboard::staticDone()
{
    qDebug() << Q_FUNC_INFO;
    delete s_instance;
    s_instance = NULL;
}


void Keyboard::handleMyQLineEdit(MyQLineEdit *qLineEdit)
{
    qDebug() << Q_FUNC_INFO << qLineEdit;

    if(m_pKeyboardCharacterHandler)
    {
        delete m_pKeyboardCharacterHandler;
        m_pKeyboardCharacterHandler = NULL;
    }

    m_pKeyboardCharacterHandler = new KeyboardCharacterHandler_QLineEdit(qLineEdit);
    m_lowerCaseKeyboard->setKeyboardCharacterHandler(m_pKeyboardCharacterHandler);
    m_upperCaseKeyboard->setKeyboardCharacterHandler(m_pKeyboardCharacterHandler);
    m_symbolKeyboard->setKeyboardCharacterHandler(m_pKeyboardCharacterHandler);
}

void Keyboard::handleTextEdit(MyQTextEdit *qTextEdit)
{
    qDebug() << Q_FUNC_INFO << qTextEdit;

    if(m_pKeyboardCharacterHandler)
    {
        delete m_pKeyboardCharacterHandler;
        m_pKeyboardCharacterHandler = NULL;
    }

    m_pKeyboardCharacterHandler = new KeyboardCharacterHandler_QTextEdit(qTextEdit);
    m_lowerCaseKeyboard->setKeyboardCharacterHandler(m_pKeyboardCharacterHandler);
    m_upperCaseKeyboard->setKeyboardCharacterHandler(m_pKeyboardCharacterHandler);
    m_symbolKeyboard->setKeyboardCharacterHandler(m_pKeyboardCharacterHandler);
}

void Keyboard::setActionKeyText(const QString &text)
{
    m_lowerCaseKeyboard->setActionKeyText(text);
    m_upperCaseKeyboard->setActionKeyText(text);
    m_symbolKeyboard->setActionKeyText(text);
}

void Keyboard::queueRefresh( const QImage& image, const QPoint& point)
{
    qDebug() << Q_FUNC_INFO;
    m_queue.append(RefreshQueued(image, point));
}

void Keyboard::justSendSignals()
{
    qDebug() << Q_FUNC_INFO;
    if(m_pKeyboardCharacterHandler)
    {
        delete m_pKeyboardCharacterHandler;
        m_pKeyboardCharacterHandler = NULL;
    }

    m_pKeyboardCharacterHandler = new KeyboardCharacterHandler_JustSignal();
    m_lowerCaseKeyboard->setKeyboardCharacterHandler(m_pKeyboardCharacterHandler);
    m_upperCaseKeyboard->setKeyboardCharacterHandler(m_pKeyboardCharacterHandler);
    m_symbolKeyboard->setKeyboardCharacterHandler(m_pKeyboardCharacterHandler);
}

void Keyboard::startKeyboardMode()
{
    qDebug() << Q_FUNC_INFO;
    m_powerLock->setTimeOut(10); // Awake period to avoid low responsiveness in first key
    m_powerLock->activate();

    m_previousRefresh = Screen::getInstance()->getCurrentMode();
    Screen::getInstance()->setMode(Screen::MODE_QUICK,false,Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_QUEUE,false);
    m_timer_queue.start(100);
}

void Keyboard::stopKeyboardMode()
{
    qDebug() << Q_FUNC_INFO;
    Screen::getInstance()->setMode(m_previousRefresh,false,Q_FUNC_INFO);
    Screen::getInstance()->setUpdateScheme(Screen::SCHEME_MERGE,false);

    m_powerLock->release();
}

void Keyboard::show(Keyboard::MODE keyboardMode)
{
        qDebug() << Q_FUNC_INFO;

        if(keyboardMode == EMAIL){
            m_lowerCaseKeyboard->setComma("@");
            m_upperCaseKeyboard->setComma("@");
            m_symbolKeyboard->setL(",");
        }

        startKeyboardMode();
        m_currentKeyboard = m_lowerCaseKeyboard;
        m_currentKeyboard->show();
}

void Keyboard::hide()
{
        qDebug() << Q_FUNC_INFO;

        m_lowerCaseKeyboard->setComma(",");
        m_upperCaseKeyboard->setComma(",");
        m_symbolKeyboard->setL("@");

        /* Flush pending updates, to separate the keyboard hiding*/
        m_currentKeyboard->hide();
        stopKeyboardMode();
        disconnect();
        cleanQueue();
}

void Keyboard::cleanQueue()
{
    qDebug() << Q_FUNC_INFO;
    m_queue.clear();
}

void Keyboard::switchUpperLowerKeyboard()
{
    Screen::getInstance()->queueUpdates();

    qDebug() << Q_FUNC_INFO << sender();
    m_currentKeyboard->hide();

    if (dynamic_cast<LowerCaseKeyboard*>(sender()))
        m_currentKeyboard = m_upperCaseKeyboard;
    else if (dynamic_cast<UpperCaseKeyboard*> (sender()))
        m_currentKeyboard = m_lowerCaseKeyboard;

    m_lastKeyboard = m_currentKeyboard;

    m_currentKeyboard->show();

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void Keyboard::switchNormalSymbolKeyboard()
{
    Screen::getInstance()->queueUpdates();

    qDebug() << Q_FUNC_INFO << sender();
    m_currentKeyboard->hide();
    cleanQueue();

    if (dynamic_cast<SymbolKeyboard*>(sender()))
        m_currentKeyboard = m_lastKeyboard;
    else
    {
        if (m_currentKeyboard == m_upperCaseKeyboard)
            m_symbolKeyboard->setChangeKeyboardButton("A B C");
        else
            m_symbolKeyboard->setChangeKeyboardButton("a b c");
        m_currentKeyboard = m_symbolKeyboard;
    }
    m_currentKeyboard->show();

    Screen::getInstance()->setMode(Screen::MODE_SAFE, true, Q_FUNC_INFO);
    Screen::getInstance()->flushUpdates();
}

void Keyboard::refreshKeys()
{
    if(m_queue.size() == 0)
        return;
    RefreshQueued first = m_queue.takeFirst();
    Screen::getInstance()->setMode(Screen::MODE_FASTEST,true,Q_FUNC_INFO);
    Screen::getInstance()->flushImage(first.imageKey, first.pointKey);
}


Keyboard::Keyboard(QWidget *parent)
    : QWidget(parent)
    , m_lowerCaseKeyboard(NULL)
    , m_upperCaseKeyboard(NULL)
    , m_symbolKeyboard(NULL)
    , m_pKeyboardCharacterHandler(NULL)
    , m_powerLock(PowerManager::getNewLock(this))
{
    qDebug() << Q_FUNC_INFO << parent;

    m_lowerCaseKeyboard = new LowerCaseKeyboard(this, m_powerLock);
    connect(m_lowerCaseKeyboard, SIGNAL(switchUpperLowerKeyboard()), this, SLOT(switchUpperLowerKeyboard()));
    connect(m_lowerCaseKeyboard, SIGNAL(switchNormalSymbolKeyboard()), this, SLOT(switchNormalSymbolKeyboard()));
    connect(m_lowerCaseKeyboard, SIGNAL(actionRequested()), this, SIGNAL(actionRequested()));
    connect(m_lowerCaseKeyboard, SIGNAL(pressedChar(const QString &)), this, SIGNAL(pressedChar(const QString &)));
    connect(m_lowerCaseKeyboard, SIGNAL(backSpacePressed()), this, SIGNAL(backSpacePressed()));
    connect(m_lowerCaseKeyboard, SIGNAL(newLinePressed()), this, SIGNAL(newLinePressed()));
    connect(m_lowerCaseKeyboard, SIGNAL(cursorLeftPressed()), this, SIGNAL(cursorLeftPressed()));
    connect(m_lowerCaseKeyboard, SIGNAL(cursorRightPressed()), this, SIGNAL(cursorRightPressed()));

    m_upperCaseKeyboard = new UpperCaseKeyboard(this, m_powerLock);
    connect(m_upperCaseKeyboard, SIGNAL(switchUpperLowerKeyboard()), this, SLOT(switchUpperLowerKeyboard()));
    connect(m_upperCaseKeyboard, SIGNAL(switchNormalSymbolKeyboard()), this, SLOT(switchNormalSymbolKeyboard()));
    connect(m_upperCaseKeyboard, SIGNAL(actionRequested()), this, SIGNAL(actionRequested()));
    connect(m_upperCaseKeyboard, SIGNAL(pressedChar(const QString &)), this, SIGNAL(pressedChar(const QString &)));
    connect(m_upperCaseKeyboard, SIGNAL(backSpacePressed()), this, SIGNAL(backSpacePressed()));
    connect(m_upperCaseKeyboard, SIGNAL(newLinePressed()), this, SIGNAL(newLinePressed()));
    connect(m_upperCaseKeyboard, SIGNAL(cursorLeftPressed()), this, SIGNAL(cursorLeftPressed()));
    connect(m_upperCaseKeyboard, SIGNAL(cursorRightPressed()), this, SIGNAL(cursorRightPressed()));

    m_symbolKeyboard = new SymbolKeyboard(this, m_powerLock);
    connect(m_symbolKeyboard, SIGNAL(switchNormalSymbolKeyboard()), this, SLOT(switchNormalSymbolKeyboard()));
    connect(m_symbolKeyboard, SIGNAL(actionRequested()), this, SIGNAL(actionRequested()));
    connect(m_symbolKeyboard, SIGNAL(pressedChar(const QString &)), this, SIGNAL(pressedChar(const QString &)));
    connect(m_symbolKeyboard, SIGNAL(backSpacePressed()), this, SIGNAL(backSpacePressed()));
    connect(m_symbolKeyboard, SIGNAL(newLinePressed()), this, SIGNAL(newLinePressed()));
    connect(m_symbolKeyboard, SIGNAL(cursorLeftPressed()), this, SIGNAL(cursorLeftPressed()));
    connect(m_symbolKeyboard, SIGNAL(cursorRightPressed()), this, SIGNAL(cursorRightPressed()));

    m_currentKeyboard = m_lowerCaseKeyboard;
    m_lastKeyboard = m_lowerCaseKeyboard;

    connect(&m_timer_queue, SIGNAL(timeout()),this,SLOT(refreshKeys()));

    justSendSignals();

}


Keyboard::~Keyboard()
{
    qDebug() << Q_FUNC_INFO;
    delete m_lowerCaseKeyboard;
    m_lowerCaseKeyboard = NULL;
    delete m_upperCaseKeyboard;
    m_upperCaseKeyboard = NULL;
    delete m_symbolKeyboard;
    m_symbolKeyboard = NULL;
    delete m_pKeyboardCharacterHandler;
    m_pKeyboardCharacterHandler = NULL;

    if(m_powerLock)
    {
        delete m_powerLock;
        m_powerLock = NULL;
    }
}

void Keyboard::move(int x, int y)
{
        qDebug() << Q_FUNC_INFO;

        m_lowerCaseKeyboard->move(x,y);
        m_upperCaseKeyboard->move(x,y);
        m_symbolKeyboard->move(x,y);

}



void Keyboard::paintEvent (QPaintEvent *){
         QStyleOption opt;
         opt.init(this);
         QPainter p(this);
         style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }
