git submodule update --init --recursive

cmake --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -B "./build_release_win64" -G "Visual Studio 16 2019" -A x64
cmake --build ./build_release_win64 --config Release
xcopy /S /Y /E "release_package" "build_release_win64/Release"
del ".\build_release_win64\Release\seeds\.gitignore"

cmake --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -B "./build_release_win32" -G "Visual Studio 16 2019" -A Win32
cmake --build ./build_release_win32 --config Release
xcopy /S /Y /E "release_package" "build_release_win32/Release"
del ".\build_release_win32\Release\seeds\.gitignore"
