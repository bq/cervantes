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

#include "ProgressDialog.h"
#include <QPainter>
#include <QDebug>

ProgressDialog::ProgressDialog(QWidget * parent, QString textStr) :
    QDialog(parent,Qt::Popup | Qt::Dialog)
{
    setupUi(this);
    textLbl->setText(textStr);
    setProgressBar(0);
    connect(cancelBtn,SIGNAL(clicked()),this,SIGNAL(cancel()), Qt::UniqueConnection);
    connect(hideBtn,SIGNAL(clicked()),this,SIGNAL(hideDialog()), Qt::UniqueConnection);

    QFile fileSpecific(":/res/ui_styles.qss");
    QFile fileCommons(":/res/ui_styles_generic.qss");
    fileSpecific.open(QFile::ReadOnly);
    fileCommons.open(QFile::ReadOnly);

    QString styles = QLatin1String(fileSpecific.readAll() + fileCommons.readAll());
    setStyleSheet(styles);
    hideBtn->hide();
}

ProgressDialog::~ProgressDialog()
{
}

void ProgressDialog::setText(QString textStr)
{
    textLbl->setText(textStr);
}

void ProgressDialog::setProgressBar(int value)
{
    qDebug() << Q_FUNC_INFO;
    progressBar->setValue(value);
}

void ProgressDialog::setTotalProgressBar(int value)
{
    progressBar->setMaximum(value);
}

void ProgressDialog::setTextValue(bool value){
    progressBar->setTextVisible(value);
}

void ProgressDialog::hideProgressBar(){
    progressBar->hide();
}

void ProgressDialog::showProgressBar(){
    progressBar->show();
}

void ProgressDialog::hideCancelButton()
{
    cancelBtn->hide();
}

void ProgressDialog::keyPressEvent(QKeyEvent *event)
{
    qDebug() << Q_FUNC_INFO;
    event->accept();
}

void ProgressDialog::keyReleaseEvent(QKeyEvent* event)
{
    qDebug() << Q_FUNC_INFO;
    event->accept();
}

void ProgressDialog::mousePressEvent(QMouseEvent* event)
{
        qDebug() << Q_FUNC_INFO;
        event->accept();
}

void ProgressDialog::mouseReleaseEvent(QMouseEvent* event)
{
        qDebug() << Q_FUNC_INFO;
        event->accept();
}

void ProgressDialog::paintEvent (QPaintEvent *){
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }

void ProgressDialog::setHideBtn(bool value)
{
    if(value)
        hideBtn->show();
    else
        hideBtn->hide();
}

