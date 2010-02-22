#!/bin/sh
#Usage: stripbin.sh <mspgcc prefix>

set -eu

if [ "$#" != 1 ]; then
	echo >&2 "Usage: $0 <mspgcc prefix>";
	exit 1
fi

DEST="$1" ; shift

case "$DEST" in
/*) ;;
*)	echo >&2 "mspgcc prefix must be absolute, got \"$DEST\". Abort."
	exit 1
	;;
esac

INSTALL_LAUNCHER=$(sh do-detect-sudo.sh "$DEST")

set +e	# there are some non-binary files in those directories (scripts),
	# we don't want to abort => set +e
cd "$DEST/bin" && $INSTALL_LAUNCHER strip *
cd "$DEST/msp430/bin" && $INSTALL_LAUNCHER strip *
cd "$DEST/libexec/gcc/msp430"/* && $INSTALL_LAUNCHER strip * install-tools/*

true # pretend success
