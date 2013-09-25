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

#ifndef WizardWelcome_H
#define WizardWelcome_H

#include "QBookForm.h"

#include "ui_WizardWelcome.h"


// Predeclarations
class FullScreenWidget;
class WizardGeneralTerms;
class WizardLanguage;
class WizardDateTime;
class WizardFinish;

class WizardWelcome : public QBookForm, protected Ui::WizardWelcome

{
    Q_OBJECT

public:
    WizardWelcome(QWidget* parent = 0);
    virtual ~WizardWelcome();

    virtual void activateForm();
    virtual void deactivateForm();

private slots:

    void goWizardFinished();
    void showRegisterFailed();
    void goToCloseWizard();
    void hideTerms();
    void loadWelcomePage();
    void requestConnection();
    void connectWifiObserver();
    void disconnectWifiObserver();
    void goToWifi();
    void connectivityProblem();
    void serialInvalidHandle();
    void headersProblem();
    void hideLanguageSettings();
    void hideWizardDateTime();
    void skipWizard();

signals:
    void closeWizard();

private:
    FullScreenWidget*       current_widget;
    WizardLanguage*      m_WizardLanguage;
    WizardDateTime*      m_WizardDateTime;
    WizardFinish*        m_elfFinishWizard;

    void showElement(FullScreenWidget*);

protected:

    /* http://qt-project.org/forums/viewthread/7340 */
    void                paintEvent                      (QPaintEvent *);
};

#endif // WizardWelcome_H
