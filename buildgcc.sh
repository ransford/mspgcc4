#!/bin/bash

# This work is partially financed by the European Commission under the
# Framework 6 Information Society Technologies Project
#  "Wirelessly Accessible Sensor Populations (WASP)".

#The following versions of GCC have been tested:

#3.2.3 (patch directory gcc-3.3)
#3.3.6 (patch directory gcc-3.4, additional configure.in patch involved)
#4.2.4
#4.3.4
#4.4.2


DIALOG=`which dialog`
if [ 0$DIALOG = 0 -a 0$1 != 0--defaults ]; then
	echo Dialog package not found and --defaults not specified
	echo Either install dialog package, or run 'buildgcc --defaults'
fi

#Defaults
GCC_VERSION=4.4.2
GCC_PATCH_FOLDER=gcc-4.x
GMP_VERSION=4.3.1
MPFR_VERSION=2.4.1
TARGET_LOCATION=/opt/msp430-gcc-$GCC_VERSION

BINUTILS_VERSION=2.19.1
GNU_MIRROR=ftp.uni-kl.de
BINPACKAGE_NAME=

GDB_VERSION=7.0
INSIGHT_VERSION=6.8-1

BASEDIR=`pwd`

if [ 0$1 != 0--defaults ]; then
	$DIALOG --menu "Select GCC version to build" 13 50 6 1 "gcc-4.4.2" 2 "gcc-4.3.4" 3 "gcc-4.2.4" 4 "gcc-3.3.6" 5 "gcc-3.2.3" 6 "none" 2>/tmp/dialog.ans
	if [ $? = 0 -a -e /tmp/dialog.ans ]; then
		case `cat /tmp/dialog.ans` in
		1)
			GCC_VERSION=4.4.2
			GCC_PATCH_FOLDER=gcc-4.x
			GMP_VERSION=4.3.1
			MPFR_VERSION=2.4.1 ;;
		2)
			GCC_VERSION=4.3.4
			GCC_PATCH_FOLDER=gcc-4.x
			GMP_VERSION=4.3.1
			MPFR_VERSION=2.4.1 ;;
		3)
			GCC_VERSION=4.2.4
			GCC_PATCH_FOLDER=gcc-4.x ;;
		4)
			GCC_VERSION=3.3.6
			GCC_PATCH_FOLDER=gcc-3.4 ;;
		5)
			GCC_VERSION=3.2.3
			GCC_PATCH_FOLDER=gcc-3.3 ;;
		6)
			GCC_VERSION= ;;
		esac
		rm /tmp/dialog.ans
		TARGET_LOCATION=/opt/msp430-gcc-$GCC_VERSION
	else
		echo Build cancelled
		exit
	fi
	
	$DIALOG --menu "Select GDB version to build" 10 50 3 1 "gdb-7.0" 2 "gdb-6.8" 3 "none" 2>/tmp/dialog.ans
	if [ $? = 0 -a -e /tmp/dialog.ans ]; then
		case `cat /tmp/dialog.ans` in
		1)
			GDB_VERSION=7.0
			GDB_PACKAGE_SUFFIX=_gdb_$GDB_VERSION ;;
		2)
			GDB_VERSION=6.8
			GDB_PACKAGE_SUFFIX=_gdb_$GDB_VERSION ;;
		3)
			GDB_VERSION=
			GDB_PACKAGE_SUFFIX= ;;
		esac
		rm /tmp/dialog.ans
	else
		echo Build cancelled
		exit
	fi

	$DIALOG --menu "Select GNU Insight version to build" 10 50 3 1 "insight-6.8-1" 2 "none" 2>/tmp/dialog.ans
	if [ $? = 0 -a -e /tmp/dialog.ans ]; then
		case `cat /tmp/dialog.ans` in
		1)
			INSIGHT_VERSION=6.8-1 ;;
		2)
			INSIGHT_VERSION= ;;
		esac
		rm /tmp/dialog.ans
	else
		echo Build cancelled
		exit
	fi

	$DIALOG --inputbox "Enter target toolchain path" 7 50 "$TARGET_LOCATION" 2>/tmp/dialog.ans
	if [ $? = 0 -a -e /tmp/dialog.ans ]; then
		TARGET_LOCATION=`cat /tmp/dialog.ans`
		rm /tmp/dialog.ans
	else
		echo Build cancelled
		exit
	fi
	
	if [ -e $TARGET_LOCATION/bin/msp430-as ]; then
		$DIALOG --yesno "Looks like binutils are already installed in $TARGET_LOCATION. Skip building binutils?" 5 50
		if [ $? = 0 ]; then
			SKIP_BINUTILS=1
		fi
	fi

	$DIALOG --yesno "Create binary package after build?" 5 50
	if [ $? = 0 ]; then
		BINPACKAGE_NAME=mspgcc-$GCC_VERSION$GDB_PACKAGE_SUFFIX.tbz
		$DIALOG --inputbox "Enter binary package name" 7 50 "$BINPACKAGE_NAME" 2>/tmp/dialog.ans
		if [ $? = 0 -a -e /tmp/dialog.ans ]; then
			BINPACKAGE_NAME=`cat /tmp/dialog.ans`
			rm /tmp/dialog.ans
		else
			echo Build cancelled
			exit
		fi
	fi
fi

echo ---------------------------------------------------------------
echo Building GCC $GCC_VERSION
echo GDB version: $GDB_VERSION
echo Target location: $TARGET_LOCATION
echo Binary package name: $BINPACKAGE_NAME
echo ---------------------------------------------------------------

BUILD_DIR=build

if [ x"$GCC_VERSION" != x"" ]; then
	if [ x"$SKIP_BINUTILS" != x"1" ]; then
		sh do-binutils.sh $TARGET_LOCATION $BINUTILS_VERSION $GNU_MIRROR $BUILD_DIR || exit 1
	fi
	sh do-gcc.sh $TARGET_LOCATION $GCC_VERSION $GNU_MIRROR $BUILD_DIR $GCC_PATCH_FOLDER $GMP_VERSION $MPFR_VERSION || exit 1
	sh do-libc.sh $TARGET_LOCATION $BUILD_DIR || exit 1
fi

if [ 0$INSIGHT_VERSION != 0 ]; then
	sh do-gdb.sh $TARGET_LOCATION $INSIGHT_VERSION $GNU_MIRROR $BUILD_DIR insight || exit 1
fi

if [ 0$GDB_VERSION != 0 ]; then
	sh do-gdb.sh $TARGET_LOCATION $GDB_VERSION $GNU_MIRROR $BUILD_DIR gdb || exit 1
fi

if [ 0$BINPACKAGE_NAME != 0 ]; then
	echo Creating binary package...
	cd $TARGET_LOCATION
	tar cjf $BASEDIR/$BINPACKAGE_NAME *
	cd $BASEDIR
fi

echo ---------------------------------------------------------------------------------
echo Build succeeded
echo Do not forget to add $TARGET_LOCATION/bin to your PATH environment variable
echo ---------------------------------------------------------------------------------