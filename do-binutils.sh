#!/bin/sh

# This work is partially financed by the European Commission under the
# Framework 6 Information Society Technologies Project
#  "Wirelessly Accessible Sensor Populations (WASP)".

#The following versions of BINUTILS have been tested:
# 2.19
# 2.19.1

. ./buildgcc.subr

VERSION_TAG=$(cat _version_tag.txt)
BINUTILS_VERSION=2.19.1
GNU_MIRROR=http://ftp.uni-kl.de
BUILD_DIR=build
INITIAL_DIR="$(pwd)"
FETCH_ONLY=0
WIN32_OPTS=

case "$(uname -s)" in
MINGW*)
	WIN32_OPTS=--enable-win32-registry=MSP430-GCC-$VERSION_TAG ;;
esac

set -eu

if [ $# = 0 ]; then
	echo "Usage:   do-binutils.sh <toolchain target dir> [<binutils_version>] [<GNU mirror site>] [<build dir>] [--fetch-only]"
	echo "Example: do-binutils.sh /opt/msp430-gcc-latest $BINUTILS_VERSION $GNU_MIRROR build"
	exit 1
fi

TARGET_LOCATION="$1" ; shift
if [ $# -ge 1 ] ; then BINUTILS_VERSION="$1" ; shift ; fi
if [ $# -ge 1 ] ; then GNU_MIRROR="$1" ; shift ; fi
if [ $# -ge 1 ] ; then BUILD_DIR="$1" ; shift ; fi

while [ $# -ge 1 ] ; do
	case "$1" in
		--fetch-only)	FETCH_ONLY=1 ;;
		*)	echo "Unknown options $@. Abort." ; exit 1;
	esac
	shift
done

INSTALL_LAUNCHER=$(sh do-detect-sudo.sh $TARGET_LOCATION)

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

export "PATH=$PATH:$TARGET_LOCATION/bin"

wget -c "$GNU_MIRROR/pub/gnu/binutils/binutils-$BINUTILS_VERSION.tar.bz2"

if [ $FETCH_ONLY = 1 ]; then
	echo "Binutils $BINUTILS_VERSION downloaded successfully"
	exit 0
fi

echo "Unpacking binutils..."
tar xjf "binutils-$BINUTILS_VERSION.tar.bz2"

cd "binutils-$BINUTILS_VERSION"

if [ -e "$INITIAL_DIR/binutils-$BINUTILS_VERSION.patch" ]
then
	patch -p1 < "$INITIAL_DIR/binutils-$BINUTILS_VERSION.patch"
fi

cd ..
mkdir -p "binutils-$BINUTILS_VERSION-build"
cd "binutils-$BINUTILS_VERSION-build"

"$(pwd)/../binutils-$BINUTILS_VERSION/configure" "--prefix=$TARGET_LOCATION" --target=msp430 --disable-werror $WIN32_OPTS --disable-nls
make -j$(num_cpus)
$INSTALL_LAUNCHER make install

cd "$INITIAL_DIR"
