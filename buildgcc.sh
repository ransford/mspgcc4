#!/bin/bash
#The following versions of GCC have been tested:

#3.2.3 (patch directory gcc-3.3)
#3.3.6 (patch directory gcc-3.4, additional configure.in patch involved)

DIALOG=`which dialog`
if [ 0$DIALOG = 0 -a 0$1 != 0--defaults ]; then
	echo Dialog package not found and --defaults not specified
	echo Either install dialog package, or run 'buildgcc --defaults'
fi

#Defaults
GCC_VERSION=4.3.4
GCC_PATCH_FOLDER=gcc-4.x
GMP_VERSION=4.3.1
MPFR_VERSION=2.4.1
TARGET_LOCATION=/opt/msp430-gcc-$GCC_VERSION

BINUTILS_VERSION=2.19.1
GNU_MIRROR=ftp.uni-kl.de
BINPACKAGE_NAME=

GDB_VERSION=5.1.1
GDB_SRC_URL=http://downloads.sourceforge.net/project/mspgcc4/gdb-5.1.1.tar.gz?use_mirror=master
GDB_PATCH_FOLDER=gdb-5.1.1
GDB_PACKAGE_SUFFIX=_gdb_$GDB_VERSION

if [ 0$1 != 0--defaults ]; then
	$DIALOG --menu "Select GCC version to build" 12 50 5 1 "gcc-4.3.4" 2 "gcc-4.2.4" 3 "gcc-3.3.6" 4 "gcc-3.2.3" 5 "none" 2>/tmp/dialog.ans
	if [ $? == 0 -a -e /tmp/dialog.ans ]; then
		if [ `cat /tmp/dialog.ans` = 1 ]; then
			GCC_VERSION=4.3.4
			GCC_PATCH_FOLDER=gcc-4.x
			GMP_VERSION=4.3.1
			MPFR_VERSION=2.4.1
		fi
		if [ `cat /tmp/dialog.ans` = 2 ]; then
			GCC_VERSION=4.2.4
			GCC_PATCH_FOLDER=gcc-4.x
		fi
		if [ `cat /tmp/dialog.ans` = 3 ]; then
			GCC_VERSION=3.3.6
			GCC_PATCH_FOLDER=gcc-3.4
		fi
		if [ `cat /tmp/dialog.ans` = 4 ]; then
			GCC_VERSION=3.2.3
			GCC_PATCH_FOLDER=gcc-3.3
		fi
		if [ `cat /tmp/dialog.ans` = 5 ]; then
			GCC_VERSION=
		fi
		rm /tmp/dialog.ans
		TARGET_LOCATION=/opt/msp430-gcc-$GCC_VERSION
	else
		echo Build cancelled
		exit
	fi
	
	$DIALOG --menu "Select GDB version to build" 10 50 3 1 "gdb-5.1.1" 2 "gdb-6.8" 3 "none" 2>/tmp/dialog.ans
	if [ $? == 0 -a -e /tmp/dialog.ans ]; then
		if [ `cat /tmp/dialog.ans` = 2 ]; then
			GDB_VERSION=6.8
			GDB_PATCH_FOLDER=gdb-6.x
			GDB_SRC_URL=ftp://$GNU_MIRROR/pub/gnu/gdb/gdb-$GDB_VERSION.tar.gz
			GDB_PACKAGE_SUFFIX=_gdb_$GDB_VERSION
		fi
		if [ `cat /tmp/dialog.ans` = 3 ]; then
			GDB_VERSION=
			GDB_PACKAGE_SUFFIX=
		fi
		rm /tmp/dialog.ans
	else
		echo Build cancelled
		exit
	fi


	$DIALOG --inputbox "Enter target GCC toolchain path" 7 50 "$TARGET_LOCATION" 2>/tmp/dialog.ans
	if [ $? == 0 -a -e /tmp/dialog.ans ]; then
		TARGET_LOCATION=`cat /tmp/dialog.ans`
		rm /tmp/dialog.ans
	else
		echo Build cancelled
		exit
	fi

	$DIALOG --yesno "Create binary package after build?" 5 50
	if [ $? == 0 ]; then
		BINPACKAGE_NAME=mspgcc-$GCC_VERSION$GDB_PACKAGE_SUFFIX.tbz
		$DIALOG --inputbox "Enter binary package name" 7 50 "$BINPACKAGE_NAME" 2>/tmp/dialog.ans
		if [ $? == 0 -a -e /tmp/dialog.ans ]; then
			BINPACKAGE_NAME=`cat /tmp/dialog.ans`
			rm /tmp/dialog.ans
		else
			echo Build cancelled
			exit
		fi
	fi
fi

INSTALL_LAUNCHER=

NEED_SUDO=0
if [ ! -e $TARGET_LOCATION ]; then
	mkdir $TARGET_LOCATION 2> /dev/null || NEED_SUDO=1
fi
touch $TARGET_LOCATION/test.dat 2>/dev/null || NEED_SUDO=1
rm $TARGET_LOCATION/test.dat 2> /dev/null || NEED_SUDO=1

if [ $NEED_SUDO == 1 ]; then
	echo WARNING! Cannot write to $TARGET_LOCATION!
	echo Please ensure your account is mentioned in /etc/sudoers and that sudo is installed
	echo All binary installation tasks will be invoked using sudo.
	sudo sleep 0 || exit 1
	
	INSTALL_LAUNCHER=sudo

	if [ ! -e $TARGET_LOCATION ]; then
		$INSTALL_LAUNCHER mkdir $TARGET_LOCATION || exit 1
	fi
	$INSTALL_LAUNCHER touch $TARGET_LOCATION/test.dat || exit 1
	$INSTALL_LAUNCHER rm $TARGET_LOCATION/test.dat || exit 1
	
fi

echo ---------------------------------------------------------------
echo Building GCC $GCC_VERSION
echo GDB version: $GDB_VERSION
echo Target location: $TARGET_LOCATION
echo Binary package name: $BINPACKAGE_NAME
echo ---------------------------------------------------------------

mkdir build
cd build

TARGET_LOCATION_SED=`echo $TARGET_LOCATION | sed -e "s/\//\\\\\\\\\//g"`
BASEDIR=`pwd`

export PATH=$PATH:$TARGET_LOCATION/bin

echo !!! If prompted for a password, just press ENTER !!!
if [ -e mspgcc ]
then
	rm -r mspgcc
fi

mkdir mspgcc

if [ x"$GCC_VERSION" != x"" ]; then
	cd mspgcc
	cvs -z3 -d:pserver:anonymous@mspgcc.cvs.sourceforge.net:/cvsroot/mspgcc co -P gcc || exit 2

	if [ -e ../../ports/gcc-4.x ]
	then
		echo Copying gcc-4.x port
		cp -r ../../ports/gcc-4.x gcc
	fi

	if [ -e ../../msp$GCC_PATCH_FOLDER.patch ]
	then
		cd gcc/$GCC_PATCH_FOLDER
		patch -p1 < ../../../../msp$GCC_PATCH_FOLDER.patch
		cd ../..
	fi

	cd ..
	wget -c ftp://$GNU_MIRROR/pub/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.bz2 || exit 3
	wget -c ftp://$GNU_MIRROR/pub/gnu/binutils/binutils-$BINUTILS_VERSION.tar.bz2 || exit 4

	if [ x"$GMP_VERSION" != x"" ]; then
		wget -c ftp://$GNU_MIRROR/pub/gnu/gmp/gmp-$GMP_VERSION.tar.bz2 || exit 4
	fi

	if [ x"$MPFR_VERSION" != x"" ]; then
		wget -c http://www.mpfr.org/mpfr-$MPFR_VERSION/mpfr-$MPFR_VERSION.tar.bz2 || exit 4
	fi

	echo Unpacking binutils...
	tar xjf binutils-$BINUTILS_VERSION.tar.bz2

	cd binutils-$BINUTILS_VERSION
	./configure --prefix=$TARGET_LOCATION --target=msp430 || exit 5
	make || exit 6
	$INSTALL_LAUNCHER make install || exit 7
	cd ..

	echo Unpacking GCC...
	tar xjf gcc-$GCC_VERSION.tar.bz2

	cd gcc-$GCC_VERSION

	if [ -e ../../gcc-$GCC_VERSION.patch ]
	then
		patch -p1 < ../../gcc-$GCC_VERSION.patch
	fi

	if [ x"$GMP_VERSION" != x"" ]; then
		tar xjf ../gmp-$GMP_VERSION.tar.bz2 || exit 1
		mv gmp-$GMP_VERSION gmp || exit 1
	fi

	if [ x"$MPFR_VERSION" != x"" ]; then
		tar xjf ../mpfr-$MPFR_VERSION.tar.bz2 || exit 1
		mv mpfr-$MPFR_VERSION mpfr || exit 1
	fi

	cp -r ../mspgcc/gcc/$GCC_PATCH_FOLDER/* . || exit 8
	cd ..
	mkdir buildgcc-$GCC_VERSION
	cd buildgcc-$GCC_VERSION
	`pwd`/../gcc-$GCC_VERSION/configure --prefix=$TARGET_LOCATION --target=msp430 --enable-languages=c,c++ || exit 9
	make || exit 10
	$INSTALL_LAUNCHER make install || exit 11
	cd ..

	cd mspgcc
	cvs -z3 -d:pserver:anonymous@mspgcc.cvs.sourceforge.net:/cvsroot/mspgcc co -P msp430-libc || exit 2
	cd msp430-libc
	test -e ../../../msp430-libc.patch && patch -p1 < ../../../msp430-libc.patch 
	mkdir src/msp1
	mkdir src/msp2
	cd ../..
	cat binutils-$BINUTILS_VERSION/gas/config/tc-msp430.c | perl ../fixprocs.pl 
	cd mspgcc
	cd msp430-libc/src
	sed -e "s/\/usr\/local\/msp430/$TARGET_LOCATION_SED/" Makefile > Makefile.new
	mv Makefile.new Makefile
	make
	$INSTALL_LAUNCHER make install || exit 13

	echo "!<arch>" > 0lib.tmp
	$INSTALL_LAUNCHER cp 0lib.tmp $TARGET_LOCATION/lib/libstdc++.a || exit 1
	rm 0lib.tmp
	$INSTALL_LAUNCHER cp $TARGET_LOCATION/msp430/include/sys/inttypes.h $TARGET_LOCATION/msp430/include/inttypes.h  || exit 1

	cd $TARGET_LOCATION/msp430/lib/ldscripts
	$INSTALL_LAUNCHER tar xjf $BASEDIR/../ports/ldscripts-new.tbz || exit 14

	cd $BASEDIR
fi

if [ 0$GDB_VERSION != 0 ]; then
	test -e mspgcc || mkdir mspgcc
	cd mspgcc
	cvs -z3 -d:pserver:anonymous@mspgcc.cvs.sourceforge.net:/cvsroot/mspgcc co -P gdb || exit 1
	cd gdb
	
	if [ -e ../../../ports/gdb-6.x ]
	then
		echo Copying gdb-6.x port
		cp -r ../../../ports/gdb-6.x .
	fi

	test -e ../../../msp430-gdb6x.tbz && tar xjf ../../../msp430-gdb6x.tbz
	cd ../..
	wget -c $GDB_SRC_URL -O gdb-$GDB_VERSION.tar.gz || exit 1
	echo Unpacking GDB...
	tar xzf gdb-$GDB_VERSION.tar.gz || exit 1

	cd gdb-$GDB_VERSION
	cp -r ../mspgcc/gdb/$GDB_PATCH_FOLDER/* .
	test -e ../../gdb-$GDB_VERSION.patch && patch -p1 < ../../gdb-$GDB_VERSION.patch
	./configure --prefix=$TARGET_LOCATION --target=msp430 || exit 1
	make || exit 1
	$INSTALL_LAUNCHER make install || exit 1
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