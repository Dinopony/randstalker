cmake --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -B "./build" -G "Visual Studio 16 2019"
cmake --build ./build
pause
