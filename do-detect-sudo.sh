#!/bin/bash

# This work is partially financed by the European Commission under the
# Framework 6 Information Society Technologies Project
#  "Wirelessly Accessible Sensor Populations (WASP)".

TARGET_LOCATION=$1

if [ 0$TARGET_LOCATION = 0 ]; then
	echo "do-detect-sudo is used internally only. Do not call it manually" 1>&2
	echo "Usage: do-detect-sudo.sh <directory to check write access>" 1>&2
	exit 1
fi

INSTALL_LAUNCHER=

NEED_SUDO=0
if [ ! -e $TARGET_LOCATION ]; then
	mkdir $TARGET_LOCATION 2> /dev/null || NEED_SUDO=1
fi
touch $TARGET_LOCATION/test.dat 2>/dev/null || NEED_SUDO=1
rm $TARGET_LOCATION/test.dat 2> /dev/null || NEED_SUDO=1

if [ $NEED_SUDO = 1 ]; then
	echo "WARNING! Cannot write to $TARGET_LOCATION! 1>&2"
	echo "Please ensure your account is mentioned in /etc/sudoers and that sudo is installed 1>&2"
	echo "All binary installation tasks will be invoked using sudo. 1>&2"
	sudo sleep 0 || exit 1
	
	INSTALL_LAUNCHER=sudo

	if [ ! -e $TARGET_LOCATION ]; then
		$INSTALL_LAUNCHER mkdir $TARGET_LOCATION || exit 1
	fi
	$INSTALL_LAUNCHER touch $TARGET_LOCATION/test.dat || exit 1
	$INSTALL_LAUNCHER rm $TARGET_LOCATION/test.dat || exit 1

fi

echo "$INSTALL_LAUNCHER"
