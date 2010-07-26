#!/bin/sh

# This work is partially financed by the European Commission under the
# Framework 6 Information Society Technologies Project
#  "Wirelessly Accessible Sensor Populations (WASP)".

#The following versions of BINUTILS have been tested:
# 6.8
# 7.0
# insight 6.8-1

# Note that insight 6.8-1 doesn't compile on Cygwin 1.7.1.
# Workaround that let us proceed a bit:
# make CC="gcc -D__USE_W32_SOCKETS" -C tcl/win
# make CC="gcc -include winsock2.h" -C tk/win  # this is insufficient.
# HELP SOLICITED!


set -eu

# source utilities
. ./buildgcc.subr

PKG_VERSION=7.0
PKG_NAME=gdb	#can also be insight
GNU_MIRROR=http://ftp.uni-kl.de
INSIGHT_MIRROR=http://gd.tuwien.ac.at/gnu/sourceware/insight/releases/
BUILD_DIR=build
INITIAL_DIR="$(pwd)"
FETCH_ONLY=0

if [ $# -lt 1 ]; then
	echo "Usage:   do-gdb.sh <toolchain target dir> [<gdb_version>] [<GNU mirror site>] [<build dir>] [gdb/insight] [--fetch-only]"
	echo "Example: do-gdb.sh /opt/msp430-gcc-latest $PKG_VERSION $GNU_MIRROR build"
	exit 1
fi

TARGET_LOCATION="$1"
shift

if [ $# -ge 1 ] && [ -n "$1" ]; then PKG_VERSION="$1" ; shift ; fi
if [ $# -ge 1 ] && [ -n "$1" ]; then GNU_MIRROR="$1" ; shift ; fi
if [ $# -ge 1 ] && [ -n "$1" ]; then BUILD_DIR="$1" ; shift ; fi
if [ $# -ge 1 ] && [ -n "$1" ]; then PKG_NAME="$1" ; shift ; fi

while [ $# -ge 1 ] ; do
	case "$1" in
		--fetch-only)	FETCH_ONLY=1 ;;
		*)		echo >&2 "Ambiguous options $@." ; exit 1 ;;
	esac
	shift
done

case "$PKG_NAME" in
gdb)
	PKG_SRC_URL="$GNU_MIRROR/pub/gnu/gdb/gdb-$PKG_VERSION.tar.bz2"
	GDB_CFG_EXTRA_FLAGS=
	;;
insight)
	PKG_SRC_URL="$INSIGHT_MIRROR/insight-$PKG_VERSION.tar.bz2"
	GDB_CFG_EXTRA_FLAGS="--enable-gdbtk"
	;;
*)
	echo "Package name must be either gdb or insight."
	exit 1
	;;
esac

INSTALL_LAUNCHER=$(sh do-detect-sudo.sh "$TARGET_LOCATION")

GNUMAKE=$(find_gnumake)
if [ -z "$GNUMAKE" ] ; then
	echo >&2 "GNU make not found, aborting!"
	exit 1
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

export PATH="$PATH:$TARGET_LOCATION/bin"

wget -c "$PKG_SRC_URL" -O "$PKG_NAME-$PKG_VERSION.tar.bz2"
echo "Unpacking $PKG_NAME..."
tar xjf "$PKG_NAME-$PKG_VERSION.tar.bz2"

cd "$PKG_NAME-$PKG_VERSION"
cp -rf "$INITIAL_DIR"/ports/gdb-6-and-7/* .

if [ $FETCH_ONLY = 1 ]; then
	echo "$PKG_NAME $PKG_VERSION downloaded successfully"
	exit 0
fi

if test -e "$INITIAL_DIR/$PKG_NAME-$PKG_VERSION.patch" ; then
       patch -p1 < "$INITIAL_DIR/$PKG_NAME-$PKG_VERSION.patch"
fi

cat >try$$.c <<'_EOF'
#include <curses.h>

int main(void) { initscr(); return 0; }
_EOF
a=0
cc -c -o try$$.o try$$.c || a=$?
rm -f try$$.o try$$.c
if test $a != 0 ; then
	echo >&2 "=============================================================="
	echo >&2 "Note that you must have libcurses developer headers installed."
	echo >&2 "Abort."
	echo >&2 "=============================================================="
	exit 1
fi

cd ..
mkdir -p "$PKG_NAME-$PKG_VERSION-build"
cd "$PKG_NAME-$PKG_VERSION-build"

"$(pwd)/../$PKG_NAME-$PKG_VERSION/configure" \
	"--prefix=$TARGET_LOCATION" \
	--target=msp430 \
	--disable-werror $GDB_CFG_EXTRA_FLAGS
$GNUMAKE -j$(num_cpus) MAKE=$GNUMAKE -e
$INSTALL_LAUNCHER $GNUMAKE install MAKE=$GNUMAKE -e

cd "$INITIAL_DIR"
