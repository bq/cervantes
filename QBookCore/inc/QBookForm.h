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

#ifndef QBOOKFORM_H
#define QBOOKFORM_H

#include <QWidget>

class QBookForm : public QWidget
{
	Q_OBJECT

public:
    enum 
    {
        FLAG_SHARED = 0x01     ///< form is shared, don't delete it after pop
    };
    
    explicit QBookForm(QWidget* parent = 0);
    virtual ~QBookForm();

    unsigned int                        formFlags                           () const { return m_flags; }
    void                                setFormFlags                        ( unsigned int flags, bool on = true );

    virtual void                        activateForm                        () = 0;
    virtual void                        deactivateForm                      () = 0;

private:
    unsigned int                        m_flags;

protected:
    virtual void                        keyReleaseEvent                     ( QKeyEvent * event );

signals:
    void                                goToHome                            ();
    void                                hideMe                              ();

};

#endif
