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

#ifndef FULLSCREENWIDGET_H
#define FULLSCREENWIDGET_H

#include <QWidget>

class QMouseEvent;

class FullScreenWidget : public QWidget
{
    Q_OBJECT

public:
    FullScreenWidget(QWidget*);
    virtual ~FullScreenWidget();

    void                setNavigationPrevious (FullScreenWidget* navPrevious) { m_navigationPrevious = navPrevious; }
    FullScreenWidget*   getNavigationPrevious() const { return m_navigationPrevious; }
    FullScreenWidget*   getNavigationParent() const { return (FullScreenWidget*)parentWidget(); }

protected:
//    virtual void mousePressEvent(QMouseEvent*);
    virtual void keyReleaseEvent(QKeyEvent *event);

private:
    FullScreenWidget* m_navigationPrevious;

signals:

    void    hideMe();
};

#endif // FULLSCREENWIDGET_H
