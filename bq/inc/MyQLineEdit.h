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

#ifndef MYQLINEEDIT_H
#define MYQLINEEDIT_H

#include <QLineEdit>

class QWidget;

class MyQLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    MyQLineEdit(QWidget* parent=0);

signals:
    void clicked();

protected:
    virtual void mouseReleaseEvent( QMouseEvent * e );

public slots:
    virtual void clear();

private slots:
    void textChanged(const QString &);
};

#endif // MYQLINEEDIT_H



