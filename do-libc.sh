#!/bin/bash

# This work is partially financed by the European Commission under the
# Framework 6 Information Society Technologies Project
#  "Wirelessly Accessible Sensor Populations (WASP)".

set -eu

. ./buildgcc.subr

INITIAL_DIR="$(pwd)"
BUILD_DIR=build
FETCH_ONLY=0

if [ $# -lt 1 ]; then
	echo "Usage:   do-libc.sh <toolchain target dir> [<build dir>] [--fetch-only]"
	echo "Example: do-libc.sh /opt/msp430-gcc-latest"
	exit 1
fi

TARGET_LOCATION="$1"
shift

if [ $# -ge 1 ] ; then
	BUILD_DIR="$2"
	shift
fi

if [ $# -ge 1 ] && [ "_$1" = "--fetch-only" ]; then
	FETCH_ONLY=1
fi

INSTALL_LAUNCHER="$(sh do-detect-sudo.sh "$TARGET_LOCATION")"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

export PATH="$TARGET_LOCATION/bin:$PATH"
TARGET_LOCATION_SED="$(echo "$TARGET_LOCATION" | sed -e "s,/,\\\\/,g")"

mkdir -p mspgcc
cd mspgcc

cvs -z3 -d:pserver:anonymous@mspgcc.cvs.sourceforge.net:/cvsroot/mspgcc co -n -A -P -d msp430-libc.pristine msp430-libc
rm -rf msp430-libc
mkdir msp430-libc
cd msp430-libc
( cd msp430-libc.pristine && tar cf - .) | ( tar xpf - )
if [ -e "$INITIAL_DIR/msp430-libc.patch" ] ; then 
	patch -p1 -N -i "$INITIAL_DIR/msp430-libc.patch"
fi
mkdir -p src/msp1 src/msp2
cd ../../mspgcc
cd msp430-libc/src
sed -e "s/\/usr\/local\/msp430/$TARGET_LOCATION_SED/" Makefile > Makefile.new
mv Makefile.new Makefile

if [ "_$FETCH_ONLY" = _1 ]; then
	echo "msp430 libc downloaded successfully"
	exit 0
fi

make -j$(num_cpus)
$INSTALL_LAUNCHER make install

echo '!<arch>' > 0lib.tmp
$INSTALL_LAUNCHER cp 0lib.tmp "$TARGET_LOCATION/lib/libstdc++.a"
rm 0lib.tmp
$INSTALL_LAUNCHER cp "$TARGET_LOCATION/msp430/include/sys/inttypes.h" "$TARGET_LOCATION/msp430/include/inttypes.h"

cd "$TARGET_LOCATION/msp430/lib/ldscripts"
$INSTALL_LAUNCHER tar xjf "$INITIAL_DIR/ports/ldscripts-new.tbz"

cd "$INITIAL_DIR"
