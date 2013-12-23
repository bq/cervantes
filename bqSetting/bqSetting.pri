#Header files
HEADERS +=  ./inc/Settings.h \
            ./inc/SettingsMyAccountMenu.h \
            ./inc/SettingsDevelopmentOptions.h \
            ./inc/SettingsUnLinkStoreDevice.h \
            ./inc/SettingsGeneralTermsInfo.h \
            ./inc/SettingsDeviceInfo.h \
            ./inc/SettingsTechnicalInfo.h \
            ./inc/SettingsUpdateDeviceAuto.h \
            ./inc/SettingsDeviceOptionsMenu.h \
            ./inc/SettingsDateTimeMenu.h \
            ./inc/SettingsDateTimeMenuItem.h \
            ./inc/SettingsLanguageMenu.h \
            ./inc/SettingsSleepTimeMenu.h \
            ./inc/SettingsReaderMenu.h \
            ./inc/SettingsReaderPageTurning.h \
            ./inc/SettingsReaderProgressBar.h \
            ./inc/SettingsPagerCont.h \
            ./inc/SettingsBrightness.h \
            ./inc/SettingsWiFiList.h \
            ./inc/SettingsWiFiListItem.h \
            ./inc/SettingsWiFiDetail.h \
            ./inc/SettingsWiFiPasswordMenu.h \
            ./inc/SettingsConfigureNetwork.h \
            ./inc/SettingConfigureNetworkListActions.h \
            ./inc/SettingsQuickSettingsPopup.h \
            ./inc/SettingsEnergySaving.h \
            ./inc/SettingsSwitchOffTimeMenu.h \
            ./inc/TimeZones.h \
            ./inc/SettingsPageScreenSaver.h \
            ./inc/SettingsAboutUs.h

isEmpty(HACKERS_EDITION) {
HEADERS += ./inc/SettingsDictionariesMenu.h \
            ./inc/SettingsDictionariesMenuItem.h \
            ./inc/SettingsDictionariesInfo.h
}




#Source files
SOURCES +=  ./src/Settings.cpp \
            ./src/SettingsMyAccountMenu.cpp \
            ./src/SettingsDevelopmentOptions.cpp \
            ./src/SettingsUnLinkStoreDevice.cpp \
            ./src/SettingsGeneralTermsInfo.cpp \
            ./src/SettingsDeviceInfo.cpp \
            ./src/SettingsTechnicalInfo.cpp \
            ./src/SettingsUpdateDeviceAuto.cpp \
            ./src/SettingsDeviceOptionsMenu.cpp \
            ./src/SettingsDateTimeMenu.cpp \
            ./src/SettingsDateTimeMenuItem.cpp \
            ./src/SettingsLanguageMenu.cpp \
            ./src/SettingsSleepTimeMenu.cpp \
            ./src/SettingsReaderMenu.cpp \
            ./src/SettingsReaderPageTurning.cpp \
            ./src/SettingsReaderProgressBar.cpp \
            ./src/SettingsPagerCont.cpp \
            ./src/SettingsBrightness.cpp \
            ./src/SettingsWiFiList.cpp \
            ./src/SettingsWiFiListItem.cpp \
            ./src/SettingsWiFiDetail.cpp \
            ./src/SettingsWiFiPasswordMenu.cpp \
            ./src/SettingsConfigureNetwork.cpp \
            ./src/SettingConfigureNetworkListActions.cpp \
            ./src/SettingsQuickSettingsPopup.cpp \
            ./src/SettingsPageScreenSaver.cpp \
            ./src/SettingsEnergySaving.cpp \
            ./src/SettingsSwitchOffTimeMenu.cpp \
            ./src/SettingsAboutUs.cpp

isEmpty(DISABLE_ADOBE_SDK) {
	HEADERS += 	./inc/SettingsAdobeDrmLinkDevice.h \
			./inc/SettingsAdobeDrmUnLinkDevice.h \
			./inc/SettingsGeneralTermsInfoAdobe.h
	SOURCES +=	./src/SettingsGeneralTermsInfoAdobe.cpp \
		        ./src/SettingsAdobeDrmLinkDevice.cpp \
			./src/SettingsAdobeDrmUnLinkDevice.cpp
}

isEmpty(HACKERS_EDITION) {
SOURCES +=  ./src/SettingsDictionariesMenu.cpp \
            ./src/SettingsDictionariesMenuItem.cpp \
            ./src/SettingsDictionariesInfo.cpp
}


#Forms
FORMS += ./ui/Settings.ui \
        ./ui/SettingsMyAccountMenu.ui \
        ./ui/SettingsDevelopmentOptions.ui \
        ./ui/SettingsUnLinkStoreDevice.ui \
        ./ui/SettingsAdobeDrmLinkDevice.ui \
        ./ui/SettingsAdobeDrmUnLinkDevice.ui \
        ./ui/SettingsGeneralTermsInfo.ui \
        ./ui/SettingsGeneralTermsInfoAdobe.ui \
        ./ui/SettingsDeviceInfo.ui \
        ./ui/SettingsTechnicalInfo.ui \
        ./ui/SettingsDeviceOptionsMenu.ui \
        ./ui/SettingsSleepTimeMenu.ui \
        ./ui/SettingsLanguageMenu.ui \
        ./ui/SettingsReaderMenu.ui \
        ./ui/SettingsReaderPageTurning.ui \
        ./ui/SettingsReaderProgressBar.ui \
        ./ui/SettingsDateTimeMenu.ui \
        ./ui/SettingsDateTimeMenuItem.ui \
        ./ui/SettingsUpdateDeviceAuto.ui \
        ./ui/SettingsPagerCont.ui \
        ./ui/SettingsBrightness.ui \
        ./ui/SettingsWiFiList.ui \
        ./ui/SettingsWiFiListItem.ui \
        ./ui/SettingsWiFiDetail.ui \
        ./ui/SettingsWiFiPasswordMenu.ui \
        ./ui/SettingsQuickSettingsPopup.ui \
        ./ui/SettingsPageScreenSaver.ui \
        ./ui/SettingsConfigureNetwork.ui \
        ./ui/SettingConfigureNetworkListActions.ui \
        ./ui/SettingsEnergySaving.ui \
        ./ui/SettingsSwitchOffTimeMenu.ui \
        ./ui/SettingsAboutUs.ui

isEmpty(HACKERS_EDITION) {
FORMS += ./ui/SettingsDictionariesMenu.ui \
         ./ui/SettingsDictionariesMenuItem.ui \
         ./ui/SettingsDictionariesInfo.ui
}

TRANSLATIONS =  ./tr/bqSetting_es.ts \
                ./tr/bqSetting_ca.ts \
                ./tr/bqSetting_en.ts \
                ./tr/bqSetting_pt.ts \
                ./tr/bqSetting_gl.ts \
                ./tr/bqSetting_eu.ts

DEPENDPATH+= ../bq/inc


QRC_FILES += Settings800.qrc Settings1024.qrc

OTHER_FILES += \
    bqSetting/res/800/settings_styles.qss \
    bqSetting/res/1024/settings_styles.qss \
    bqSetting/res/settings_styles_generic.qss

