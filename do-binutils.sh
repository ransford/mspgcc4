#!/bin/bash

# This work is partially financed by the European Commission under the
# Framework 6 Information Society Technologies Project
#  "Wirelessly Accessible Sensor Populations (WASP)".

#The following versions of BINUTILS have been tested:
# 2.19
# 2.19.1

BINUTILS_VERSION=2.19.1
GNU_MIRROR=ftp.uni-kl.de
BUILD_DIR=build
INITIAL_DIR=`pwd`

if [ 0$1 = 0 ]; then
	echo "Usage:   do-binutils.sh <toolchain target dir> [<binutils_version>] [<GNU mirror site>] [<build dir>] [--fetch-only]"
	echo "Example: do-binutils.sh /opt/msp430-gcc-latest $BINUTILS_VERSION $GNU_MIRROR build"
	exit 1
fi

TARGET_LOCATION=$1

if [ 0$2 != 0 ]; then
	BINUTILS_VERSION=$2
fi

if [ 0$3 != 0 ]; then
	GNU_MIRROR=$3
fi

if [ 0$4 != 0 ]; then
	BUILD_DIR=$4
fi

if [ 0$5 = 0--fetch-only ]; then
	FETCH_ONLY=1
fi

INSTALL_LAUNCHER=`sh do-detect-sudo.sh $TARGET_LOCATION` || exit 1

mkdir $BUILD_DIR
cd $BUILD_DIR || exit 1

export PATH=$PATH:$TARGET_LOCATION/bin

wget -c ftp://$GNU_MIRROR/pub/gnu/binutils/binutils-$BINUTILS_VERSION.tar.bz2 || exit 1

if [ 0$FETCH_ONLY = 01 ]; then
	echo "Binutils $BINUTILS_VERSION downloaded successfully"
	exit 0
fi

echo "Unpacking binutils..."
tar xjf binutils-$BINUTILS_VERSION.tar.bz2

cd binutils-$BINUTILS_VERSION

if [ -e $INITIAL_DIR/binutils-$BINUTILS_VERSION.patch ]
then
	patch -p1 < $INITIAL_DIR/binutils-$BINUTILS_VERSION.patch
fi

cd ..
mkdir binutils-$BINUTILS_VERSION-build
cd binutils-$BINUTILS_VERSION-build

`pwd`/../binutils-$BINUTILS_VERSION/configure --prefix=$TARGET_LOCATION --target=msp430 || exit 1
make || exit 1
$INSTALL_LAUNCHER make install || exit 1

cd $INITIAL_DIR
