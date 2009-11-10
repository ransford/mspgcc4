#!/bin/bash

# This work is partially financed by the European Commission under the
# Framework 6 Information Society Technologies Project
#  "Wirelessly Accessible Sensor Populations (WASP)".

#The following versions of GCC have been tested:

#3.2.3 (patch directory gcc-3.3)
#3.3.6 (patch directory gcc-3.4, additional configure.in patch involved)
#4.2.4
#4.3.4 (GMP 4.3.1; MPFR 2.4.1)

GCC_VERSION=4.3.4
GCC_PATCH_FOLDER=gcc-4.x
GMP_VERSION=4.3.1
MPFR_VERSION=2.4.1
GNU_MIRROR=ftp.uni-kl.de
BUILD_DIR=build
INITIAL_DIR=`pwd`

if [ 0$1 = 0 ]; then
	echo "Usage:   do-gcc.sh <toolchain target dir> [<gcc_version>] [<GNU mirror site>] [<build dir>] [<GCC patch folder>] [<GMP version>] [<MPFR version>] [--fetch-only/--no-fetch]"
	echo "Example: do-gcc.sh /opt/msp430-gcc-latest $GCC_VERSION $GNU_MIRROR build $GCC_PATCH_FOLDER $GMP_VERSION $MPFR_VERSION"
	echo "Specify '-' instead of GMP/MPFR version to skip downloading it"
	exit 1
fi

TARGET_LOCATION=$1

if [ 0$2 != 0 ]; then
	GCC_VERSION=$2
fi

if [ 0$3 != 0 ]; then
	GNU_MIRROR=$3
fi

if [ 0$4 != 0 ]; then
	BUILD_DIR=$4
fi

if [ 0$5 != 0 ]; then
	GCC_PATCH_FOLDER=$5
fi

if [ 0$6 != 0 ]; then
	GMP_VERSION=$6
fi

if [ 0$7 != 0 ]; then
	MPFR_VERSION=$7
fi

if [ 0$8 = 0--fetch-only ]; then
	FETCH_ONLY=1
fi

if [ 0$8 = 0--no-fetch ]; then
	NO_FETCH=1
fi

INSTALL_LAUNCHER=`sh do-detect-sudo.sh $TARGET_LOCATION` || exit 1

mkdir $BUILD_DIR
cd $BUILD_DIR || exit 1

export PATH=$PATH:$TARGET_LOCATION/bin
TARGET_LOCATION_SED=`echo $TARGET_LOCATION | sed -e "s/\//\\\\\\\\\//g"`

if [ 0$NO_FETCH != 01 ]; then
	if [ -e mspgcc ]
	then
		rm -rf mspgcc
	fi

	mkdir mspgcc
	cd mspgcc
	cvs -z3 -d:pserver:anonymous@mspgcc.cvs.sourceforge.net:/cvsroot/mspgcc co -P gcc || exit 2

	if [ -e $INITIAL_DIR/ports/gcc-4.x ]
	then
		echo "Copying gcc-4.x port"
		cp -r $INITIAL_DIR/ports/gcc-4.x gcc
	fi

	if [ -e $INITIAL_DIR/msp$GCC_PATCH_FOLDER.patch ]
	then
		cd gcc/$GCC_PATCH_FOLDER
		patch -p1 < $INITIAL_DIR/msp$GCC_PATCH_FOLDER.patch
		cd ../..
	fi

	cd ..
	wget -c ftp://$GNU_MIRROR/pub/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.bz2 || exit 3

	if [ x"$GMP_VERSION" != x"-" ]; then
		wget -c ftp://$GNU_MIRROR/pub/gnu/gmp/gmp-$GMP_VERSION.tar.bz2 || exit 4
	fi

	if [ x"$MPFR_VERSION" != x"-" ]; then
		wget -c http://www.mpfr.org/mpfr-$MPFR_VERSION/mpfr-$MPFR_VERSION.tar.bz2 || exit 4
	fi
fi

if [ 0$FETCH_ONLY = 01 ]; then
	echo "GCC $GCC_VERSION downloaded successfully"
	exit 0
fi

echo "Unpacking GCC..."
tar xjf gcc-$GCC_VERSION.tar.bz2

cd gcc-$GCC_VERSION

if [ -e $INITIAL_DIR/gcc-$GCC_VERSION.patch ]
then
	patch -p1 < $INITIAL_DIR/gcc-$GCC_VERSION.patch
fi

if [ x"$GMP_VERSION" != x"-" ]; then
	tar xjf ../gmp-$GMP_VERSION.tar.bz2 || exit 1
	rm -rf gmp
	mv gmp-$GMP_VERSION gmp || exit 1
fi

if [ x"$MPFR_VERSION" != x"-" ]; then
	echo "Unpacking MPFR..."
	tar xjf ../mpfr-$MPFR_VERSION.tar.bz2 || exit 1
	rm -rf mpfr
	mv mpfr-$MPFR_VERSION mpfr || exit 1
fi

cp -rf ../mspgcc/gcc/$GCC_PATCH_FOLDER/* . || exit 8
cd ..
mkdir gcc-$GCC_VERSION-build
cd gcc-$GCC_VERSION-build
`pwd`/../gcc-$GCC_VERSION/configure --prefix=$TARGET_LOCATION --target=msp430 --enable-languages=c,c++ || exit 9
make || exit 10
$INSTALL_LAUNCHER make install || exit 11

cd $INITIAL_DIR
