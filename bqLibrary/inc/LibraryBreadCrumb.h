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

#ifndef LIBRARYBREADCRUMB_H
#define LIBRARYBREADCRUMB_H

#include <QWidget>
#include "ui_LibraryBreadCrumb.h"
#include <QPushButton>

#include <QStringList>
#include <QList>

// Predeclarations
class QSignalMapper;
class QPaintEvent;

class LibraryBreadCrumbButton : public QPushButton
{
public:
    LibraryBreadCrumbButton( const QString& text, QWidget* parent = 0 );

protected:
    virtual void                        paintEvent                          ( QPaintEvent* event );
    QString                             m_realText;
};


class LibraryBreadCrumb : public QWidget, protected Ui::LibraryBreadCrumb
{
    Q_OBJECT

public:
    LibraryBreadCrumb(QWidget*, const QString& path = "");
    virtual ~LibraryBreadCrumb();
    void                                setDirectory                        ( const QString& path );

protected slots:
    void                                changeDirectorySLOT                 ( QString );

signals:
    void                                changeDirectory                     ( QString path );

protected:
    void                                updateBreadCrumb                    ();

    QStringList                         m_breadCrumbPath;
    QList<LibraryBreadCrumbButton*>     m_buttonList;
    QSignalMapper* m_signalMapper;
};

#endif // LIBRARYBREADCRUMB_H
