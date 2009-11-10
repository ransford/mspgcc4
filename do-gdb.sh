#!/bin/bash

# This work is partially financed by the European Commission under the
# Framework 6 Information Society Technologies Project
#  "Wirelessly Accessible Sensor Populations (WASP)".

#The following versions of BINUTILS have been tested:
# 6.8
# 7.0
# insight 6.8-1

PKG_VERSION=7.0
PKG_NAME=gdb	#can also be insight
GNU_MIRROR=ftp.uni-kl.de
BUILD_DIR=build
INITIAL_DIR=`pwd`

if [ 0$1 = 0 ]; then
	echo "Usage:   do-gdb.sh <toolchain target dir> [<gdb_version>] [<GNU mirror site>] [<build dir>] [gdb/insight] [--fetch-only]"
	echo "Example: do-gdb.sh /opt/msp430-gcc-latest $PKG_VERSION $GNU_MIRROR build"
	exit 1
fi

TARGET_LOCATION=$1

if [ 0$2 != 0 ]; then
	PKG_VERSION=$2
fi

if [ 0$3 != 0 ]; then
	GNU_MIRROR=$3
fi

if [ 0$4 != 0 ]; then
	BUILD_DIR=$4
fi

if [ 0$5 != 0 ]; then
	PKG_NAME=$5
fi

if [ 0$6 = 0--fetch-only ]; then
	FETCH_ONLY=1
fi

case $PKG_NAME in
gdb)
	PKG_SRC_URL=ftp://$GNU_MIRROR/pub/gnu/gdb/gdb-$PKG_VERSION.tar.bz2
	;;
insight)
	PKG_SRC_URL=ftp://sourceware.org/pub/insight/releases/insight-$PKG_VERSION.tar.bz2
	;;
*)
	echo "Package name shoulw be either gdb or insight".
	exit 1
	;;
esac



INSTALL_LAUNCHER=`sh do-detect-sudo.sh $TARGET_LOCATION` || exit 1

mkdir $BUILD_DIR
cd $BUILD_DIR || exit 1

export PATH=$PATH:$TARGET_LOCATION/bin

wget -c $PKG_SRC_URL -O $PKG_NAME-$PKG_VERSION.tar.bz2 || exit 1
echo "Unpacking $PKG_NAME..."
tar xjf $PKG_NAME-$PKG_VERSION.tar.bz2 || exit 1

cd $PKG_NAME-$PKG_VERSION
cp -rf $INITIAL_DIR/ports/gdb-6-and-7/* . || exit 1

if [ 0$FETCH_ONLY = 01 ]; then
	echo "$PKG_NAME $PKG_VERSION downloaded successfully"
	exit 0
fi

test -e $INITIAL_DIR/$PKG_NAME-$PKG_VERSION.patch && patch -p1 < $INITIAL_DIR/$PKG_NAME-$PKG_VERSION.patch

cd ..
mkdir $PKG_NAME-$PKG_VERSION-build
cd $PKG_NAME-$PKG_VERSION-build

`pwd`/../$PKG_NAME-$PKG_VERSION/configure --prefix=$TARGET_LOCATION --target=msp430 || exit 1
make || exit 1
$INSTALL_LAUNCHER make install || exit 1

cd $INITIAL_DIR
