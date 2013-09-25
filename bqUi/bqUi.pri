TRANSLATIONS =  ./tr/bqUi_es.ts \
                ./tr/bqUi_ca.ts \
                ./tr/bqUi_en.ts \
                ./tr/bqUi_pt.ts \
                ./tr/bqUi_gl.ts \
                ./tr/bqUi_eu.ts


HEADERS += ./inc/PopUp.h \
           ./inc/FullScreenWidget.h \
           ./inc/HorizontalPagination.h \
           ./inc/Keyboard.h \
           ./inc/MouseFilter.h \
           ./inc/DegreesLabel45.h \
           ./inc/DegreesLabel90.h \
           ./inc/DegreesLabel270.h \
           ./inc/DegreesPushButton.h \
           ./inc/SelectionDialog.h \
           ./inc/InfoDialog.h \
           ./inc/ConfirmDialog.h \
           ./inc/PowerOffDialog.h \
           ./inc/ProgressDialog.h \
           ./inc/KeyboardCharacterHandler.h \
           ./inc/KeyboardButton.h \
           ./inc/CorruptedDialog.h \
           ./inc/GestureWidget.h  \
           ./inc/FastQPushButton.h \
           ./inc/SilentQPushButton.h \
           ./inc/StatusBar.h \
           ./inc/bqVerticalPageHandler.h \
           ./inc/ElfHorizontalPagination.h \
           ./inc/ElidedLabel.h \
           ./inc/SwipeableQPushButton.h \
           ./inc/HoverWidget.h \
           ./inc/LongPressWidget.h \
           ./inc/BrightnessSlider.h \
           ./inc/BrightnessSliderItem.h \
           ./inc/LongPressedQPushButton.h



SOURCES += ./src/PopUp.cpp \
           ./src/FullScreenWidget.cpp \
           ./src/Keyboard.cpp \
           ./src/HorizontalPagination.cpp \
           ./src/MouseFilter.cpp \
           ./src/DegreesLabel45.cpp \
           ./src/DegreesLabel90.cpp \
           ./src/DegreesLabel270.cpp \
           ./src/DegreesPushButton.cpp \
           ./src/SelectionDialog.cpp \
           ./src/InfoDialog.cpp \
           ./src/ConfirmDialog.cpp \
           ./src/PowerOffDialog.cpp \
           ./src/ProgressDialog.cpp \
           ./src/KeyboardCharacterHandler.cpp \
           ./src/KeyboardButton.cpp \
           ./src/CorruptedDialog.cpp \
           ./src/GestureWidget.cpp \
           ./src/FastQPushButton.cpp \
           ./src/SilentQPushButton.cpp \
           ./src/StatusBar.cpp \
           ./src/bqVerticalPageHandler.cpp \
           ./src/ElfHorizontalPagination.cpp \
           ./src/ElidedLabel.cpp \
           ./src/SwipeableQPushButton.cpp \
           ./src/HoverWidget.cpp \
           ./src/LongPressWidget.cpp \
           ./src/BrightnessSlider.cpp \
           ./src/BrightnessSliderItem.cpp \
           ./src/LongPressedQPushButton.cpp


FORMS +=   ./ui/HorizontalPagination.ui \
           ./ui/Keyboard.ui \
           ./ui/KeyboardLayer.ui \
           ./ui/ConfirmDialog.ui \
           ./ui/CorruptedDialog.ui \
           ./ui/InfoDialog.ui \
           ./ui/bqVerticalPageHandler.ui \
           ./ui/ElfHorizontalPagination.ui \
           ./ui/SelectionDialog.ui \
           ./ui/ProgressDialog.ui \
           ./ui/ElfPowerOffDialog.ui \
           ./ui/StatusBar.ui




QRC_FILES += Ui800.qrc Ui1024.qrc

OTHER_FILES += \
    bqUi/res/ui_styles_generic.qss \
    bqUi/res/1024/ui_styles.qss \
    bqUi/res/800/ui_styles.qss
