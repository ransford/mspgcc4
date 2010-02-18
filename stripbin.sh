#!/bin/sh
#Usage: stripbin.sh <mspgcc prefix>

#set -eu

if [ "$1" = "" ]; then
	echo "Usage: stripbin.sh <mspgcc prefix>";
	exit
fi

cd $1/bin && strip *
cd $1/msp430/bin && strip *

cd $1/libexec/gcc/msp430
cd $(ls)
strip *
cd install-tools
strip *