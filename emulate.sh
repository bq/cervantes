#!/bin/sh

# Why are we doing this???
rm x86-bin/langs/*.rcc

MAKEFLAGS='-j14' ./build.sh $CMD -emulate $@
if [ $? = 0 ]; then
	killall syncHelper
        QBOOK_DEBUG=* LD_LIBRARY_PATH=./ConnectionManager/lib/x86-lib:./bqUtils/x86-lib:../qbookapp-private/bqClientServices/x86-lib:./model/x86-lib ../qbookapp-private/syncHelper/x86-bin/syncHelper &
        QBOOK_DEBUG=* LD_LIBRARY_PATH=./ConnectionManager/lib/x86-lib:./bqUtils/x86-lib:../qbookapp-private/bqClientServices/x86-lib:./model/x86-lib ./x86-bin/QBookApp
	killall syncHelper
else
	echo "Compilation failed"
fi

