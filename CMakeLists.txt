cmake_minimum_required(VERSION 3.1.0)

project(randstalker VERSION 1.0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

add_executable(randstalker
    src/MegadriveTools/MdCode.cpp
    src/MegadriveTools/MdRom.cpp
    src/GameStrings.cpp
    src/GameText.cpp
    src/RandomizerOptions.cpp
    src/World.cpp
    src/WorldRandomizer.cpp
    src/GameAlterations.cpp
    src/main.cpp
)
