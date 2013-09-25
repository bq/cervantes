#!/bin/bash

export LC_ALL=C
cd /
find | egrep -i "\./bin/|\./sbin/|\./boot|\./lib|\./usr" | grep -v pointercal | sort > deviceRootfs.manifest
diff rootfs.manifest deviceRootfs.manifest | grep '^> ' | sed 's/> //' > removedFiles.list
rm -v `cat removedFiles.list`
rm deviceRootfs.manifest
