#!/bin/sh
git submodule update --recursive
cmake --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -B "./build" -G "Unix Makefiles"
cmake --build ./build
