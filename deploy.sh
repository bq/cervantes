#!/bin/bash

export LANG=en_US.UTF-8
export LC_ALL=en_US.UTF-8

ROOT_DIR=$PWD

# setup options

REMOVE_LIBS=no
HACKERS=no
TEST=no
PREFIX=/app
PRIVATE=../qbookapp-private

# get target dir
TARGET_DIR="$1"

if [ -z $TARGET_DIR ]; then
    echo "target root dir must be specified!"
    exit 1
fi
shift

until [ $# -lt 1 ]; do
    if [ "$1" = "-release" ]; then
        REMOVE_LIBS=yes
    elif [ "$1" = "-test" ]; then
	echo "-test option is deprecated. Ignored."
    elif [ "$1" = "-hackers" ]; then
	echo "Using hackers edition deploy"
	HACKERS=yes
    elif [ "$1" = "-private" ]; then
        PRIVATE="$2"; shift ; shift;
        echo "Private repo in" $PRIVATE
    fi
    shift
done

# define install dir function
function install_dir
{
    local src=$1
    local dst=$2
    echo install to $dst

    ls -1 "$src" | while read name; do
        if [ -d "$src/$name" ]; then
            install_dir "$src/$name" "$dst/$name" || return $?
        elif [ -f "$src/$name" ]; then
            sudo install -D -m 644 "$src/$name" "$dst/$name" || return $?
        fi
    done
}

# define install so function
function install_so
{
    local src=$1
    local dst=$2
    echo install to $dst

    (cd $src; sudo tar cf - *.so* --owner 0 --group 0 --no-recursion) | (cd $dst; sudo tar xf -) || return $?
}

# copy files to root fs
echo install to $TARGET_DIR/$PREFIX/bin
sudo install -d $TARGET_DIR/$PREFIX/bin
sudo install -p -t $TARGET_DIR/$PREFIX/bin $ROOT_DIR/bin/QBookApp || exit $?
sudo install -p -m 755 -t $TARGET_DIR/$PREFIX/bin $ROOT_DIR/QHome/bin/* || exit $?
echo "install to $TARGET_DIR/$PREFIX/lib"
sudo install -d $TARGET_DIR/$PREFIX/lib
sudo install -p -t $TARGET_DIR/$PREFIX/lib $ROOT_DIR/ConnectionManager/lib/lib/libConnectionManager.so.1.0.0 || exit $?
sudo install -p -t $TARGET_DIR/$PREFIX/bin $ROOT_DIR/ConnectionManager/connect/bin/Connect || exit $?
sudo install -p -t $TARGET_DIR/$PREFIX/bin $ROOT_DIR/ConnectionManager/offline/bin/Offline || exit $?
sudo ln -sf libConnectionManager.so.1.0.0 $TARGET_DIR/$PREFIX/lib/libConnectionManager.so.1.0 || exit $?
sudo ln -sf libConnectionManager.so.1.0 $TARGET_DIR/$PREFIX/lib/libConnectionManager.so.1 || exit $?
sudo ln -sf libConnectionManager.so.1 $TARGET_DIR/$PREFIX/lib/libConnectionManager.so || exit $?

if [ $HACKERS != "yes" ]; then
        sudo install -p -t $TARGET_DIR/$PREFIX/lib $PRIVATE/bqClientServices/lib/libServices.so.1.0.0 || exit $?
        sudo ln -sf libServices.so.1.0.0 $TARGET_DIR/$PREFIX/lib/libServices.so.1.0 || exit $?
        sudo ln -sf libServices.so.1.0 $TARGET_DIR/$PREFIX/lib/libServices.so.1 || exit $?
        sudo ln -sf libServices.so.1 $TARGET_DIR/$PREFIX/lib/libServices.so || exit $?
        sudo install -p -t $TARGET_DIR/$PREFIX/bin $PRIVATE/syncHelper/bin/syncHelper || exit $?
fi

sudo install -p -t $TARGET_DIR/$PREFIX/lib $ROOT_DIR/bqUtils/lib/libbqUtils.so.1.0.0 || exit $?
sudo ln -sf libbqUtils.so.1.0.0 $TARGET_DIR/$PREFIX/lib/libbqUtils.so.1.0 || exit $?
sudo ln -sf libbqUtils.so.1.0 $TARGET_DIR/$PREFIX/lib/libbqUtils.so.1 || exit $?
sudo ln -sf libbqUtils.so.1 $TARGET_DIR/$PREFIX/lib/libbqUtils.so || exit $?

sudo install -p -t $TARGET_DIR/$PREFIX/lib $ROOT_DIR/model/lib/libbqModelLibrary.so.1.0.0 || exit $?
sudo ln -sf libbqModelLibrary.so.1.0.0 $TARGET_DIR/$PREFIX/lib/libbqModelLibrary.so.1.0 || exit $?
sudo ln -sf libbqModelLibrary.so.1.0 $TARGET_DIR/$PREFIX/lib/libbqModelLibrary.so.1 || exit $?
sudo ln -sf libbqModelLibrary.so.1 $TARGET_DIR/$PREFIX/lib/libbqModelLibrary.so || exit $?

install_dir $ROOT_DIR/QHome/fonts $TARGET_DIR/$PREFIX/fonts || exit $?
install_dir $ROOT_DIR/QHome/etc $TARGET_DIR/$PREFIX/etc || exit $?
install_dir $ROOT_DIR/QHome/res $TARGET_DIR/$PREFIX/res || exit $?
#install_dir $ROOT_DIR/QHome/books $TARGET_DIR/home/books || exit $?
install_dir $ROOT_DIR/bin/langs $TARGET_DIR/$PREFIX/langs || exit $?
install_dir $ROOT_DIR/QHome/share $TARGET_DIR/$PREFIX/share || exit $?

