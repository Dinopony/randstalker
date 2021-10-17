cmake_minimum_required(VERSION 3.1.0)

project(randstalker VERSION 1.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

add_compile_definitions(RELEASE="${PROJECT_VERSION}")
add_compile_definitions(MAJOR_RELEASE="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

set(SOURCES     
    "src/Enums/ItemCodes.hpp"
    "src/Enums/ItemSourceCodes.hpp"
    "src/Enums/RegionCodes.hpp"
    "src/Enums/SpawnLocations.hpp"
    "src/Enums/SpawnLocations.cpp" 

    "src/Extlibs/Base64.hpp"
    "src/Extlibs/Base64.cpp"

    "src/Huffman/Bitfield.hpp"
    "src/Huffman/HuffmanTree.hpp"
    "src/Huffman/HuffmanTreeNode.hpp"
    "src/Huffman/Symbols.hpp"
    "src/Huffman/TextDecoder.hpp"
    "src/Huffman/TextEncoder.hpp"
   
    "src/MegadriveTools/MdCode.hpp"
    "src/MegadriveTools/MdCode.cpp"
    "src/MegadriveTools/MdRom.hpp"
    "src/MegadriveTools/MdRom.cpp"
    "src/MegadriveTools/MdTypes.hpp"

    "src/ArgumentDictionary.hpp"
    "src/Assets.cpp"
    "src/Assets.hpp"
    "src/Exceptions.hpp"
    "src/GameAlterations.cpp"
    "src/GameAlterations.hpp"
    "src/GameText.hpp"
    "src/GameText.cpp"
    "src/HintSign.hpp"
    "src/Item.hpp"
    "src/ItemSources.hpp"
    "src/RandomizerOptions.hpp"
    "src/RandomizerOptions.cpp"
    "src/SpoilerLog.hpp"
    "src/Tools.hpp"
    "src/TreeMap.hpp"
    "src/UnsortedSet.hpp"
    "src/World.cpp"
    "src/World.hpp"
    "src/WorldMacroRegion.hpp"
    "src/WorldRandomizer.hpp"
    "src/WorldRandomizer.cpp"
    "src/WorldRegion.hpp"
    "src/main.cpp"
)

add_executable(randstalker "${SOURCES}")

