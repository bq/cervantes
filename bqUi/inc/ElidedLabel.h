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

#ifndef ELIDEDLABEL_H
#define ELIDEDLABEL_H

#include <QLabel>
#include <QFrame>

/* This class is used to elide the text when it
  doesn't enter in the space reserved for it.
  For work properly the minimum size of the label
  must be defined.*/

class QString;

class ElidedLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(bool isElided READ isElided)

public:
    ElidedLabel(QWidget *parent);
    virtual ~ElidedLabel();
    void setText(const QString &text);
    bool isElided() const { return elided; }
    QString m_lastText;
    QString m_elidedText;
    int m_lastWidth;

protected:
    void paintEvent(QPaintEvent *event);

private:
    bool elided;
    bool valide;
    QString content;
};

#endif // ELIDEDLABEL_H
