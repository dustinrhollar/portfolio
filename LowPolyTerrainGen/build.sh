#!/bin/bash

# Get the target machine
MACHINE=$(./scripts/machine.sh)

if [[ "$MACHINE" == "Linux" ]]; then
	COPY="cp -ru"

	# Get the directory
	ME="$(readlink -f "$0")"
	LOCATION="$(dirname "$ME")"

elif [[ "$MACHINE" == "Mac" ]]; then
	COPY="gcp"

	# Get the directory
	ME="$(greadlink -f "$0")"
	LOCATION="$(dirname "$ME")"

fi 

tput setaf 11;
echo "################################################################"
echo "Running Maple-Merchant Build Script"
echo "################################################################"
echo; tput sgr0

# Check directories

if [ ! -d $LOCATION/bin/debug ];
then
	mkdir -p $LOCATION/bin/debug
fi

if [ ! -d $LOCATION/bin/release ];
then
	mkdir -p $LOCATION/bin/release
fi

if [ ! -d $LOCATION/bin/lib ];
then
	mkdir $LOCATION/bin/lib
fi

if [ ! -d $LOCATION/bin/include ];
then
	mkdir $LOCATION/bin/include
fi

# Copy over resources
# NOTE(Dustin): Copying only to debug right now
$COPY $LOCATION/data/shaders $LOCATION/bin/debug

tput setaf 5;
echo "################################################################"
echo "##################  BUILDING MAPLE-MERCHANT "
echo "################################################################"
echo;tput sgr0

# For now, assume we are compiling for debug
RUNTIME_LIBPATH=
DEBUG_FLAGS="-no-pie -g -rdynamic -Wall -Wno-sequence-point -Wno-unused-function"
#DEBUG_FLAGS="-g -rdynamic -Wall -Wno-sequence-point"
COMMON_FLAGS="-I../include -I../../src/ -I../../ext/ -Wl,-rpath=$RUNTIME_LIBPATH"
LINKER_FLAGS="-lm -ldl -lX11 -lxcb -lxcb-keysyms -lGL"

pushd bin/debug
	echo gcc $DEBUG_FLAGS $COMMON_FLAGS -o MapleVk.exe ../../src/UnityBuild.c $LINKER_FLAGS
	gcc $DEBUG_FLAGS $COMMON_FLAGS -o MapleVk.exe ../../src/UnityBuild.c $LINKER_FLAGS
popd

echo;

tput setaf 11;
echo "################################################################"
echo "Build complete"
echo "################################################################"
echo; tput sgr0

echo;tput sgr0
