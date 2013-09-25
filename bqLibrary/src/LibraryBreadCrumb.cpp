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


#include "LibraryBreadCrumb.h"

#include <QDebug>
#include <QLabel>
#include <QDir>
#include <QPaintEvent>
#include <QSignalMapper>


LibraryBreadCrumbButton::LibraryBreadCrumbButton( const QString& text, QWidget* parent ) :
    QPushButton(text, parent)
{
    m_realText = text;
}

void LibraryBreadCrumbButton::paintEvent(QPaintEvent* event)
{
    qDebug() << Q_FUNC_INFO << event->rect();
    if (isVisible() && m_realText == text() && event->rect().topLeft() == QPoint(0, 0))
    {
        QString buttonText = fontMetrics().elidedText(m_realText, Qt::ElideLeft, event->rect().width ());
        qDebug() << Q_FUNC_INFO << "Showing button " << text() << event->rect().width ();
        setText(buttonText);
    }
    QPushButton::paintEvent(event);
}

#define BREADCRUMB_HEIGHT   50

// Number of pixels needed for fix the margin of button
#define BUTTON_BORDER       20

LibraryBreadCrumb::LibraryBreadCrumb(QWidget* parent, const QString& path)
    : QWidget(parent)
    , m_signalMapper(NULL)
{
    setupUi(this);

    m_breadCrumbPath = path.split("/", QString::SkipEmptyParts);
    m_signalMapper = new QSignalMapper(this);
    connect(m_signalMapper, SIGNAL(mapped(QString)), this, SLOT(changeDirectorySLOT(QString)));
    m_buttonList.clear();
    qDebug() << Q_FUNC_INFO << ": Updating BreadCrumb to " << path;
    updateBreadCrumb();
}

LibraryBreadCrumb::~LibraryBreadCrumb()
{
    qDebug() << Q_FUNC_INFO << ": Cleaning old Buttons";
    while (!m_buttonList.isEmpty())
        delete m_buttonList.takeFirst();

    delete m_signalMapper;
    m_signalMapper = NULL;
}

void LibraryBreadCrumb::setDirectory( const QString& path )
{
    m_breadCrumbPath = path.split("/", QString::SkipEmptyParts);
    qDebug() << Q_FUNC_INFO << ": Updating BreadCrumb to " << path;
    updateBreadCrumb();
}

void LibraryBreadCrumb::updateBreadCrumb()
{
    qDebug() << Q_FUNC_INFO << ": Cleaning old Buttons";

    while (!m_buttonList.isEmpty())
    {
        LibraryBreadCrumbButton* button = m_buttonList.takeFirst();
        buttonLayout->removeWidget(button);
        m_signalMapper->removeMappings(button);
        delete button;
    }

    if (m_breadCrumbPath.size() <= 0)
        return;

    LibraryBreadCrumbButton* baseDirButton = new LibraryBreadCrumbButton(".../", this);
    QFontMetrics aFontMetric = baseDirButton->fontMetrics();
    QSize buttonSize = baseDirButton->size();
    buttonSize.setWidth(aFontMetric.width(baseDirButton->text()) + BUTTON_BORDER);
    buttonSize.setHeight(BREADCRUMB_HEIGHT);
    baseDirButton->setMinimumSize(buttonSize);
    baseDirButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    baseDirButton->setFocusPolicy(Qt::NoFocus);
    connect(baseDirButton, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
    m_signalMapper->setMapping(baseDirButton, "");
    buttonLayout->addWidget(baseDirButton);
    m_buttonList << baseDirButton;

    // Create an auxiliaryButton with text '...' that show the last hide directory
    LibraryBreadCrumbButton* auxButton = new LibraryBreadCrumbButton(".../", this);
    buttonSize = auxButton->size();
    buttonSize.setWidth(aFontMetric.width(auxButton->text()) + BUTTON_BORDER);
    buttonSize.setHeight(BREADCRUMB_HEIGHT);
    auxButton->setMinimumSize(buttonSize);
    auxButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    auxButton->setFocusPolicy(Qt::NoFocus);
    buttonLayout->addWidget(auxButton);
    m_buttonList << auxButton;

    QString path;
    int size = m_breadCrumbPath.size();
    for (int i = 0; i < size; ++i)
    {
        path += "/" + m_breadCrumbPath[i];
        path = QDir::cleanPath(path);
        qDebug() << Q_FUNC_INFO << ": Button " << m_breadCrumbPath[i] << " has path = " << path;
        LibraryBreadCrumbButton* button = new LibraryBreadCrumbButton(m_breadCrumbPath[i] + "/", this);
        buttonSize = button->size();
        button->setMinimumHeight(BREADCRUMB_HEIGHT);
        button->setFocusPolicy(Qt::NoFocus);
        connect(button, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
        m_signalMapper->setMapping(button, path);
        buttonLayout->addWidget(button);
        m_buttonList << button;
    }

    int maxWidth = maximumSize().width();
    qDebug() << Q_FUNC_INFO << "width = " << maxWidth;
    int currentWidth = baseDirButton->minimumSize().width();

    bool showingButton = true;

    path.clear();
    QList<LibraryBreadCrumbButton *>::iterator it = m_buttonList.end()-1;
    QList<LibraryBreadCrumbButton *>::iterator itEnd = m_buttonList.begin();
    for (; it >= itEnd; --it)
    {
        LibraryBreadCrumbButton* button(*it);
        if (button == auxButton)
        {
            if (showingButton)
                auxButton->hide();
            else
            {
                auxButton->show();
                qDebug() << Q_FUNC_INFO << "Showing button " << auxButton->text() << auxButton->minimumSize().width();
            }
        }
        else if ( button == baseDirButton)
        {
            // baseDirectory is always shown
            baseDirButton->show();
            qDebug() << Q_FUNC_INFO << "Showing button " << baseDirButton->text() << baseDirButton->minimumSize().width();
        }
        else
        {
            int buttonWidth = button->sizeHint().width();
            qDebug() << Q_FUNC_INFO << "Maxwidth " << maxWidth << currentWidth;

            if (((maxWidth > (currentWidth + buttonWidth)) && showingButton) || button == m_buttonList.last())
            {
                qDebug() << Q_FUNC_INFO << "Showing button " << button->text() << buttonWidth;
                // ShowButton
                button->show();
                currentWidth += buttonWidth;
            }
            else
            {
                qDebug() << Q_FUNC_INFO << "hiding button " << button->text() << buttonWidth;
                path.prepend(QDir::cleanPath(button->text())+"/");

                // hideButton
                button->hide();
                if (showingButton)
                    currentWidth += auxButton->width();
                showingButton = false;
            }
        }
    }

    connect(auxButton, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
    m_signalMapper->setMapping(auxButton, path);
    qDebug() << Q_FUNC_INFO << ": ommited path = " << path;
}

void LibraryBreadCrumb::changeDirectorySLOT( QString dir )
{
    qDebug() << Q_FUNC_INFO << ": " << QDir::cleanPath(dir);
    emit changeDirectory(QDir::cleanPath(dir));
}
