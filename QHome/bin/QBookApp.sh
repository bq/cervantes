#!/bin/bash

export TSLIB_CONSOLEDEVICE=none
export TSLIB_FBDEVICE=/dev/fb0
export TSLIB_TSDEVICE=/dev/input/event1
export TSLIB_CALIBFILE=/usr/etc/pointercal
export TSLIB_CONFFILE=/etc/ts.conf
export TSLIB_PLUGINDIR=/usr/lib/arm-linux-gnueabi/ts0/
export QWS_DISPLAY=einkfb
export QWS_MOUSE_PROTO=tslib:/dev/input/event1
export QT_QWS_FONTDIR=/usr/lib/fonts
export POINTERCAL_FILE=/usr/etc/pointercal
export QWS_KEYBOARD=eb600keypad:/dev/input/event0

#AdobeDRM

export ADEPT_DEVICE_NAME=eBook
export ADEPT_DEVICE_TYPE=mobile
export ADEPT_ACTIVATION_DIR=/mnt/public/.adobe-digital-editions/
export ADEPT_ACTIVATION_FILE=/mnt/public/.adobe-digital-editions/activation.xml
export ADEPT_DEVICE_FILE=/mnt/public/.adobe-digital-editions/device.xml
export ADOBE_DE_DOC_FOLDER=/mnt/public/"Digital Editions"
export ADOBE_SI_FILE_FOLDER=/mnt/public/screensaver-images
export ADOBE_DE_ROOT_FOLDER=/
export ADOBE_DE_MOBILE=1

export LD_LIBRARY_PATH=/usr/plugins/imageformats:/app/lib

export HOME=/mnt/data

# run app until normal quit or shutdown
err=-1
until [ $err -eq 199 ] || [ $err -eq 255 ]; do
    rmmod g_file_storage
    (cd /app/bin; nice --20 ./QBookApp  -qws $err)
    err=$?
    sleep 2
done

# 199 means shutdown now
if [ $err -eq 199 ]; then
    sync
    shutdown -h now
fi

# 255 means reboot for OTA upgrade
if [ $err -eq 255 ]; then
    sync
    reboot
fi
