git submodule update --recursive

cmake --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -B "./build_debug" -G "Visual Studio 16 2019" -A x64
cmake --build ./build_debug --config Debug
pause