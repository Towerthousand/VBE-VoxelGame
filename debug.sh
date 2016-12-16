#!/bin/bash

#SCRIPT CONFIG
EXECUTABLE_NAME='VoxelGame'

set -e
bold=$(tput bold)
normal=$(tput sgr0)

function prim { # PRint IMportant
    IFS="%"
    echo $bold$@$normal
    unset IFS
}

function usage {
    prim 'Usage:'
    echo '    Run after building with compile.sh -d.'
}

BUILD_DIR='build-debug'

if [ ! -x $BUILD_DIR/game/$EXECUTABLE_NAME ]; then
    echo 'Cannot find an executable at '$PWD'/'$BUILD_DIR'/game/'$EXECUTABLE_NAME'.'
    echo 'Are you sure you have built it?'
    exit 1
fi
cd $BUILD_DIR/game
gdb $EXECUTABLE_NAME
