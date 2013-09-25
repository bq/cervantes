#!/bin/bash -e

./armBuild.sh $*
./deploy.sh ../target/rootfs $*

