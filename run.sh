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
    echo '    Run after building with compile.sh.'
    prim '    Options:'
    prim '        -d'
    echo '            Build in debug the debug build.'
}

BUILD_DIR='build'
DEBUG=false

while getopts ":dh" opt; do
  case $opt in
    d)
      DEBUG=true
      BUILD_DIR='build-debug'
      ;;
    h)
      usage
      exit 1
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      usage
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
      usage
      exit 1
      ;;
  esac
done
if [ ! -x $BUILD_DIR/game/$EXECUTABLE_NAME ]; then
    echo 'Cannot find an executable at '$PWD'/'$BUILD_DIR'/game/'$EXECUTABLE_NAME'.'
    echo 'Are you sure you have built it?'
    exit 1
fi
cd $BUILD_DIR/game
./$EXECUTABLE_NAME
