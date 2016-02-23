# Input
HEADERS +=  ./inc/Browser.h \
            ./inc/BrowserFavo.h \
            ./inc/BrowserFavoModel.h \
            ./inc/BrowserFavoItem.h \
            ./inc/BrowserWebPage.h \
            ./inc/BrowserCommonWebView.h \
            ./inc/BrowserNetworkAccessManager.h \
            ./inc/BrowserNetworkCookieJar.h \
            ./inc/BrowserNetworkProxyFactory.h \
            ./inc/BrowserNetworkDiskCache.h \
            ./inc/BrowserInputContext.h \
            ./inc/BrowserPageSlider.h \

SOURCES +=  ./src/Browser.cpp \
            ./src/BrowserFavo.cpp \
            ./src/BrowserFavoModel.cpp \
            ./src/BrowserFavoItem.cpp \
            ./src/BrowserWebPage.cpp \
            ./src/BrowserCommonWebView.cpp \
            ./src/BrowserNetworkAccessManager.cpp \
            ./src/BrowserNetworkCookieJar.cpp \
            ./src/BrowserNetworkProxyFactory.cpp \
            ./src/BrowserNetworkDiskCache.cpp \
            ./src/BrowserInputContext.cpp \
            ./src/BrowserPageSlider.cpp 

isEmpty(DISABLE_ADOBE_SDK) {
	HEADERS += ./inc/BrowserAdobeDrm.h
	SOURCES += ./src/BrowserAdobeDrm.cpp
}
FORMS +=    ./ui/Browser.ui \
            ./ui/BrowserFavo.ui \
            ./ui/BrowserPageSlider.ui \
            ./ui/BrowserFavoItem.ui

TRANSLATIONS =  ./tr/bqBrowser_es.ts \
                ./tr/bqBrowser_ca.ts \
                ./tr/bqBrowser_en.ts \
                ./tr/bqBrowser_pt.ts \
                ./tr/bqBrowser_gl.ts \
                ./tr/bqBrowser_eu.ts \
                ./tr/bqBrowser_fr.ts \
                ./tr/bqBrowser_de.ts \
                ./tr/bqBrowser_it.ts

QRC_FILES += Browser800.qrc Browser1024.qrc

OTHER_FILES += \
    bqBrowser/res/browser_styles_generic.qss \
    bqBrowser/res/800/browser_styles.qss \
    bqBrowser/res/1024/browser_styles.qss
