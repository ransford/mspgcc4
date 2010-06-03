#!/bin/sh

# This work is partially financed by the European Commission under the
# Framework 6 Information Society Technologies Project
#  "Wirelessly Accessible Sensor Populations (WASP)".

set -eu

. ./buildgcc.subr

INITIAL_DIR="$(pwd)"
BUILD_DIR=build
FETCH_ONLY=0

if [ $# -lt 1 ]; then
	echo "Usage:   do-libc.sh <toolchain target dir> [<build dir>] [--fetch-only] [URL]"
	echo "Example: do-libc.sh /opt/msp430-gcc-latest"
	exit 1
fi

TARGET_LOCATION="$1"
shift

if [ $# -ge 1 ] ; then
	BUILD_DIR="$1"
	shift
fi

if [ $# -ge 1 ] && [ "_$1" = "_--fetch-only" ]; then
	FETCH_ONLY=1
	shift
fi

OVERRIDE_URL=
if [ $# -ge 1 ] ; then
	OVERRIDE_URL="$1"
	shift
fi

INSTALL_LAUNCHER="$(sh do-detect-sudo.sh "$TARGET_LOCATION")"

GNUMAKE=$(find_gnumake)
if [ -z "$GNUMAKE" ] ; then
	echo >&2 "GNU make not found, aborting!"
	exit 1
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

export PATH="$TARGET_LOCATION/bin:$PATH"
TARGET_LOCATION_SED="$(echo "$TARGET_LOCATION" | sed -e "s,/,\\\\/,g")"
# Ensure updated path is provided to sudo
if [ -n "${INSTALL_LAUNCHER}" ] ; then
  INSTALL_LAUNCHER="${INSTALL_LAUNCHER} PATH=${PATH}"
fi

mkdir -p mspgcc
cd mspgcc

if [ "$OVERRIDE_URL" = "" ] ; then
	cvs -z3 -d:pserver:anonymous@mspgcc.cvs.sourceforge.net:/cvsroot/mspgcc co -n -A -P -d msp430-libc.pristine msp430-libc
	rm -rf msp430-libc
	mkdir msp430-libc
	cd msp430-libc
	( cd ../msp430-libc.pristine && tar cf - .) | ( tar xpf - )
	if [ -e "$INITIAL_DIR/msp430-libc.patch" ] ; then 
		patch -p1 -N -i "$INITIAL_DIR/msp430-libc.patch"
	fi
	mkdir -p src/msp1 src/msp2
	cd ../../mspgcc
	cd msp430-libc/src
	sed -e "s/\/usr\/local\/msp430/$TARGET_LOCATION_SED/" Makefile > Makefile.new
	mv Makefile.new Makefile
else
	ARCHIVE_NAME=$(echo $OVERRIDE_URL | sed s/^.*\\\/\\\([^\\\/]*\\\)\$/\\\1/;)
	ARCHIVE_BASE=$(echo $OVERRIDE_URL | sed s/^.*\\\/\\\([^\\\/.]*\\\)\\..*\$/\\\1/;)
	wget -c "$OVERRIDE_URL"
	
	test -e $ARCHIVE_BASE && rm -rf $ARCHIVE_BASE
	tar xjf $ARCHIVE_NAME
	cd 	$ARCHIVE_BASE/src
fi


if [ "_$FETCH_ONLY" = _1 ]; then
	echo "msp430 libc downloaded successfully"
	exit 0
fi

# some versions of libc fail the parallel build; so retry
# a serial build if the parallel build fails.
$GNUMAKE -j$(num_cpus) PREFIX=$TARGET_LOCATION || \
$GNUMAKE               PREFIX=$TARGET_LOCATION
$INSTALL_LAUNCHER PATH="$PATH" $GNUMAKE install PREFIX=$TARGET_LOCATION

echo '!<arch>' > 0lib.tmp
$INSTALL_LAUNCHER cp 0lib.tmp "$TARGET_LOCATION/lib/libstdc++.a"
rm 0lib.tmp
$INSTALL_LAUNCHER cp "$TARGET_LOCATION/msp430/include/sys/inttypes.h" "$TARGET_LOCATION/msp430/include/inttypes.h"

#cd "$TARGET_LOCATION/msp430/lib/ldscripts"
#$INSTALL_LAUNCHER tar xjf "$INITIAL_DIR/ports/ldscripts-new.tar.bz2"

cd "$INITIAL_DIR"
