#!/bin/bash

#SCRIPT CONFIG
PROJECT_FILE_RELATIVE_PATH="../VoxelGame.pro"

set -e
set -o pipefail
bold=$(tput bold)
normal=$(tput sgr0)

function prim { # PRint IMportant
    IFS="%"
    echo $bold$@$normal
    unset IFS
}

function usage {
    prim "Usage: "
    echo "    Configure the script accordingly and then run it from the project root directory."
    prim "    Options:"
    prim "        -d"
    echo "            Build in debug mode. This will enable consistency checks and debug logs."
    prim "        -r"
    echo "            Rebuild all. Will remove the previous build directory"
    echo "            and recreate the symbolic links."
}

REBUILD=false
DEBUG=false
BUILD_DIR="build"

while getopts ":rdh" opt; do
  case $opt in
    r)
      REBUILD=true
      ;;
    d)
      DEBUG=true
      BUILD_DIR="build-debug"
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
if [ $REBUILD = true ]; then
    prim "Rebuilding all projects"
    prim "Removing previous build..."
    rm -rf $BUILD_DIR
fi
mkdir -p $BUILD_DIR
cd $BUILD_DIR
prim "Running qmake..."

if [ $DEBUG = true ]; then
    prim "DEBUG BUILD"
    qmake $PROJECT_FILE_RELATIVE_PATH -r -spec linux-g++-64 2>&1 CONFIG+=debug | ccze -A
else
    prim "RELEASE BUILD"
    qmake $PROJECT_FILE_RELATIVE_PATH -r -spec linux-g++-64 2>&1 | ccze -A
fi

if [ ! -L "game/assets" ]; then
    prim "Creating symbolic link for assets folder..."
    ln -s $PWD/../game/assets game/assets
fi
prim "Building project..."
make -j8 2>&1 | ccze -A
