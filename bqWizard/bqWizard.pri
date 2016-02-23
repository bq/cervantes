TRANSLATIONS = tr/bqWizard_es.ts \
                tr/bqWizard_ca.ts \
		tr/bqWizard_en.ts \
                tr/bqWizard_pt.ts \
                tr/bqWizard_gl.ts \
                tr/bqWizard_eu.ts \
                tr/bqWizard_fr.ts \
                tr/bqWizard_de.ts \
                tr/bqWizard_it.ts



HEADERS += ./inc/WizardWelcome.h \
           ./inc/WizardFinish.h \
           ./inc/WizardGeneralTerms.h \
           ./inc/WizardLanguage.h \
           ./inc/WizardDateTime.h \
           ./inc/WizardDateTimeItem.h \
           ./inc/WebWizard.h \
           ./inc/WizardPagerCont.h

SOURCES += ./src/WizardWelcome.cpp \
           ./src/WizardFinish.cpp \
           ./src/WizardGeneralTerms.cpp \
           ./src/WizardLanguage.cpp \
           ./src/WizardDateTime.cpp \
           ./src/WizardDateTimeItem.cpp \
           ./src/WebWizard.cpp \
           ./src/WizardPagerCont.cpp

FORMS += ./ui/WizardWelcome.ui \
         ./ui/WizardFinish.ui \
         ./ui/WizardGeneralTerms.ui \
         ./ui/WizardLanguage.ui \
         ./ui/WizardDateTime.ui \
         ./ui/WizardDateTimeItem.ui \
         ./ui/WizardPagerCont.ui


QRC_FILES += Wizard800.qrc Wizard1024.qrc

OTHER_FILES += \
    bqWizard/res/wizard_styles_generic.qss \
    bqWizard/res/800/wizard_styles.qss \
    bqWizard/res/1024/wizard_styles.qss


