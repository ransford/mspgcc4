#!/bin/sh
#Usage: stripbin.sh <mspgcc prefix>

set -eu

if [ "$#" != 1 ]; then
	echo >&2 "Usage: $0 <mspgcc prefix>";
	exit 1
fi

case "$1" in
/*) ;;
*)	echo >&2 "mspgcc prefix must be absolute, got \"$1\". Abort."
	exit 1
	;;
esac

set +e	# there are some non-binary files in those directories (scripts),
	# we don't want to abort => set +e
cd "$1/bin" && strip *
cd "$1/msp430/bin" && strip *
cd "$1/libexec/gcc/msp430"/* && strip * install-tools/*
