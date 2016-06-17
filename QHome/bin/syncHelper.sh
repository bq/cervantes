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
export LD_LIBRARY_PATH=/usr/plugins/imageformats:/app/lib

cd /app/bin
nice -19 ./syncHelper  -qws
