#!/bin/bash

# Get the target machine
MACHINE=$(./scripts/machine.sh)

if [[ "$MACHINE" == "Linux" ]]; then
	# Get the directory
	ME="$(readlink -f "$0")"
	LOCATION="$(dirname "$ME")"
elif [[ "$MACHINE" == "Mac" ]]; then
	# Get the directory
	ME="$(greadlink -f "$0")"
	LOCATION="$(dirname "$ME")"
fi 

pushd $LOCATION/bin/debug
	./MapleVk.exe
popd
