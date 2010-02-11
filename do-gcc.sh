#!/bin/sh

# This work is partially financed by the European Commission under the
# Framework 6 Information Society Technologies Project
#  "Wirelessly Accessible Sensor Populations (WASP)".

#The following versions of GCC have been tested:

#3.2.3 (patch directory gcc-3.3)
#3.3.6 (patch directory gcc-3.4, additional configure.in patch involved)
#4.2.4
#4.3.4 (GMP 4.3.1; MPFR 2.4.1)

# source helper functions:
. ./buildgcc.subr

set -eu

VERSION_TAG=$(cat _version_tag.txt)
GCC_VERSION="4.3.4"
GCC_PATCH_FOLDER="gcc-4.x"
GMP_VERSION="4.3.1"
MPFR_VERSION="2.4.1"
GNU_MIRROR="ftp.uni-kl.de"
BUILD_DIR="build"
INITIAL_DIR="$(pwd)"
FETCH_ONLY=0
NO_FETCH=0

WIN32_OPTS=
if [ $(uname -o) = Msys ]; then
	WIN32_OPTS=--enable-win32-registry=MSP430-GCC-$VERSION_TAG
fi

if [ $# = 0 ]; then
	echo "Usage:   do-gcc.sh <toolchain target dir> [<gcc_version>] [<GNU mirror site>] [<build dir>] [<GCC patch folder>] [<GMP version>] [<MPFR version>] [--fetch-only/--no-fetch]"
	echo "Example: do-gcc.sh /opt/msp430-gcc-latest $GCC_VERSION $GNU_MIRROR build $GCC_PATCH_FOLDER $GMP_VERSION $MPFR_VERSION"
	echo "Specify '-' instead of GMP/MPFR version to skip downloading it"
	exit 1
fi

TARGET_LOCATION="$1" ; shift
if [ $# -ge 1 ] ; then GCC_VERSION="$1" ; shift ; fi
if [ $# -ge 1 ] ; then GNU_MIRROR="$1" ; shift ; fi
if [ $# -ge 1 ] ; then BUILD_DIR="$1" ; shift ; fi
if [ $# -ge 1 ] ; then GCC_PATCH_FOLDER="$1" ; shift ; fi
if [ $# -ge 1 ] ; then GMP_VERSION="$1" ; shift ; fi
if [ $# -ge 1 ] ; then MPFR_VERSION="$1" ; shift ; fi

while [ $# -ge 1 ] ; do
	case "$1" in
		--fetch-only)	FETCH_ONLY=1 ;;
		--no-fetch)	NO_FETCH=1   ;;
		*)		echo "Unknown options $@. Abort." ; exit 1 ;;
	esac
	shift
done

INSTALL_LAUNCHER="$(sh do-detect-sudo.sh "$TARGET_LOCATION")"

GNUMAKE=$(find_gnumake)
if [ -z "$GNUMAKE" ] ; then
	echo >&2 "GNU make not found, aborting!"
	exit 1
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

export PATH="$PATH:$TARGET_LOCATION/bin"
TARGET_LOCATION_SED="$(echo $TARGET_LOCATION | sed -e "s,/,\\\\,g")"

if [ $NO_FETCH != 1 ]; then
	rm -rf mspgcc

	mkdir -p mspgcc
	cd mspgcc
	if [ $GCC_PATCH_FOLDER != gcc-4.x ]; then
		cvs -z3 -d:pserver:anonymous@mspgcc.cvs.sourceforge.net:/cvsroot/mspgcc co -P gcc
	else
		mkdir gcc
	fi

	if [ -e "$INITIAL_DIR/ports/gcc-4.x" ]
	then
		echo "Copying gcc-4.x port"
		cp -r "$INITIAL_DIR/ports/gcc-4.x" gcc
	fi

	if [ -e "$INITIAL_DIR/msp$GCC_PATCH_FOLDER.patch" ]
	then
		cd "gcc/$GCC_PATCH_FOLDER"
		patch -p1 < "$INITIAL_DIR/msp$GCC_PATCH_FOLDER.patch"
		cd ../..
	fi

	cd ..
	wget -c "ftp://$GNU_MIRROR/pub/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.bz2"

	if [ x"$GMP_VERSION" != x"-" ]; then
		wget -c "ftp://$GNU_MIRROR/pub/gnu/gmp/gmp-$GMP_VERSION.tar.bz2"
	fi

	if [ x"$MPFR_VERSION" != x"-" ]; then
		wget "http://www.mpfr.org/mpfr-$MPFR_VERSION/mpfr-$MPFR_VERSION.tar.bz2"
	fi
fi

if [ $FETCH_ONLY = 1 ]; then
	echo "GCC $GCC_VERSION downloaded successfully"
	exit 0
fi

echo "Unpacking GCC..."
tar xjf "gcc-$GCC_VERSION.tar.bz2"

cd "gcc-$GCC_VERSION"

if [ -e "$INITIAL_DIR/gcc-$GCC_VERSION.patch" ]
then
	patch -p1 < "$INITIAL_DIR/gcc-$GCC_VERSION.patch"
fi

if [ x"$GMP_VERSION" != x"-" ]; then
	tar xjf "../gmp-$GMP_VERSION.tar.bz2"
	rm -rf gmp
	mv "gmp-$GMP_VERSION" gmp
fi

if [ x"$MPFR_VERSION" != x"-" ]; then
	echo "Unpacking MPFR..."
	tar xjf "../mpfr-$MPFR_VERSION.tar.bz2"
	rm -rf mpfr
	mv "mpfr-$MPFR_VERSION" mpfr
	if [ $(uname -o) = Msys ]; then
		echo "echo \"#!/bin/sh\" > libtool" >> mpfr/configure
		echo "echo \"/bin/libtool \\\"\\\$@\\\"\" >> libtool" >> mpfr/configure
	fi
fi

cp -rf ../mspgcc/gcc/"$GCC_PATCH_FOLDER"/* .
cd ..
mkdir -p "gcc-$GCC_VERSION-build"
cd "gcc-$GCC_VERSION-build"

if [ $(uname -o) = Msys ]; then
	"$(pwd -W)/../gcc-$GCC_VERSION/configure" --prefix="$TARGET_LOCATION" --target=msp430 --enable-languages=c,c++ $WIN32_OPTS --disable-nls --with-pkgversion="MSPGCC4_$VERSION_TAG"
	GNUMAKE=mingw32-make
else
	"$(pwd)/../gcc-$GCC_VERSION/configure" --prefix="$TARGET_LOCATION" --target=msp430 --enable-languages=c,c++ $WIN32_OPTS --with-pkgversion="MSPGCC4_$VERSION_TAG"
fi

if [ $(uname -o) = Msys ]; then
	../../mingw32-gccwa.pl $TARGET_LOCATION
else
	$GNUMAKE -j$(num_cpus)
fi

$INSTALL_LAUNCHER $GNUMAKE install

cd "$INITIAL_DIR"
