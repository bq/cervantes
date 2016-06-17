#!/bin/bash

export TSLIB_CONSOLEDEVICE=none
export TSLIB_FBDEVICE=/dev/fb0
export TSLIB_TSDEVICE=/dev/input/event1
export TSLIB_CALIBFILE=/usr/etc/pointercal
export TSLIB_CONFFILE=/etc/ts.conf
export TSLIB_PLUGINDIR=/usr/lib/arm-linux-gnueabi/ts0/
export POINTERCAL_FILE=/usr/etc/pointercal

/app/bin/tsRefresh.sh &
bgpid=$!

ts_calibrate

kill -9 $bgpid
