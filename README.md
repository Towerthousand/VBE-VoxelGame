# VBE-VoxelGame

Voxel Game

## Requirements

This demo uses OpenGL 4.3. Appropriate drivers should be installed. It also uses [GLEW](http://glew.sourceforge.net/) (`libglew-dev`, tested with >=1.10) and [SDL 2](https://www.libsdl.org/) (`libsdl2-dev`, tested with >=2.0.1) for window managing and input/audio devices.

To build these projects you will need both `make` and `qmake` installed.

The buils/run scripts use the [ccze](https://github.com/cornet/ccze) (`ccze`) utility to color the output of the build/run process.

## Setup

After cloning initially from `https://github.com/Towethousand/VBE-VoxelGame.git` make sure to run:

    git submodule init
    git submodule update

## Building

You can build this project with the `compile.sh` script. For build options, see `./compile.sh -h`.

Builds directories are `./build/` for the release build and `./build-debug/` for the debug build.

## Running

Run the demo with the `run.sh` script once you've built it successfully. Use `-d` to run the debug build.
