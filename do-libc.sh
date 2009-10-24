#!/bin/bash

# This work is partially financed by the European Commission under the
# Framework 6 Information Society Technologies Project
#  "Wirelessly Accessible Sensor Populations (WASP)".

INITIAL_DIR=`pwd`

if [ 0$1 = 0 ]; then
	echo "Usage:   do-libc.sh <toolchain target dir> [--fetch-only]"
	echo "Example: do-libc.sh /opt/msp430-gcc-latest"
	exit 1
fi

TARGET_LOCATION=$1

if [ 0$2 = 0--fetch-only ]; then
	FETCH_ONLY=1
fi

INSTALL_LAUNCHER=`sh do-detect-sudo.sh $TARGET_LOCATION` || exit 1

mkdir $BUILD_DIR
cd $BUILD_DIR || exit 1

export PATH=$PATH:$TARGET_LOCATION/bin
TARGET_LOCATION_SED=`echo $TARGET_LOCATION | sed -e "s/\//\\\\\\\\\//g"`

mkdir mspgcc
cd mspgcc

cvs -z3 -d:pserver:anonymous@mspgcc.cvs.sourceforge.net:/cvsroot/mspgcc co -P msp430-libc || exit 2
cd msp430-libc
test -e $INITIAL_DIR/msp430-libc.patch && patch -p1 < $INITIAL_DIR/msp430-libc.patch 
mkdir src/msp1
mkdir src/msp2
cd ../..
cd mspgcc
cd msp430-libc/src
sed -e "s/\/usr\/local\/msp430/$TARGET_LOCATION_SED/" Makefile > Makefile.new
mv Makefile.new Makefile

if [ 0$FETCH_ONLY = 01 ]; then
	echo msp430 libc downloaded successfully
	exit 0
fi

make
$INSTALL_LAUNCHER make install || exit 13

echo "!<arch>" > 0lib.tmp
$INSTALL_LAUNCHER cp 0lib.tmp $TARGET_LOCATION/lib/libstdc++.a || exit 1
rm 0lib.tmp
$INSTALL_LAUNCHER cp $TARGET_LOCATION/msp430/include/sys/inttypes.h $TARGET_LOCATION/msp430/include/inttypes.h  || exit 1

cd $TARGET_LOCATION/msp430/lib/ldscripts
$INSTALL_LAUNCHER tar xjf $INITIAL_DIR/ports/ldscripts-new.tbz || exit 14

cd $INITIAL_DIR