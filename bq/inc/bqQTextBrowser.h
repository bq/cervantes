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

#ifndef BQQTEXTBROWSER_H
#define BQQTEXTBROWSER_H

#include <QWidget>
#include <QTextBrowser>
#include <qevent.h>

class MouseFilter;
class TouchEvent;

class bqQTextBrowser: public QTextBrowser
{
    Q_OBJECT
    public:
                                    bqQTextBrowser                          (QWidget* parent);
        virtual                     ~bqQTextBrowser                         ();

        void                        applyDocument                           (const QString& html);
        void                        setTappable                             (bool);

    signals:
        void                        wordClicked                             ( const QString& word, const QString& context);
        void                        pressed                                 ();

    protected:
        MouseFilter*                m_mouseFilter;
        virtual void                customEvent                             (QEvent*);
        void                        handleTap                               (TouchEvent*);
        bool                        m_tappable;
};

#endif // BQQTEXTBROWSER_H
