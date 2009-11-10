#!/bin/bash

# This work is partially financed by the European Commission under the
# Framework 6 Information Society Technologies Project
#  "Wirelessly Accessible Sensor Populations (WASP)".

TARGET_LOCATION="$1"
if [ -z "$TARGET_LOCATION" ]; then
	cat >&2 <<_EOF
do-detect-sudo is used internally only. Do not call it manually
Usage: do-detect-sudo.sh <directory to check write access>
_EOF
	exit 1
fi

set -eu

istgtwritable() {
	(
		set -eu
		$INSTALL_LAUNCHER mkdir -p "$TARGET_LOCATION"
		$INSTALL_LAUNCHER touch "$TARGET_LOCATION"/test.dat
		rc=$?
		$INSTALL_LAUNCHER rm -f "$TARGET_LOCATION"/test.dat
		exit $rc
	) 2>/dev/null
}

INSTALL_LAUNCHER=

NEED_SUDO=0
istgtwritable || NEED_SUDO=1

if [ $NEED_SUDO = 1 ]; then
	cat >&2 <<_EOF
Cannot write to $TARGET_LOCATION. Trying to use sudo. Please ensure that
1. sudo is installed
2. your account is privileged in /etc/sudoers
All binary installation tasks will be invoked using sudo.
_EOF
	
	INSTALL_LAUNCHER=sudo
	if ! sudo true || ! istgtwritable ; then
		cat >&2 <<_EOF
Even using sudo, $TARGET_LOCATION is not writable. Aborting.
_EOF
		exit 1
	fi
fi

echo "$INSTALL_LAUNCHER"
