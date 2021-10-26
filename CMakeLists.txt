cmake_minimum_required(VERSION 3.1.0)

include("CMakeFunctions.txt")

project(randstalker VERSION 1.3.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

# Wrap JSON model files into automated .hxx headers
wrap_file(SOURCE_FILE "src/model/item.json" VARIABLE_NAME "ITEMS_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/model/world_region.json" VARIABLE_NAME "WORLD_REGIONS_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/model/world_path.json" VARIABLE_NAME "WORLD_PATHS_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/model/item_source.json" VARIABLE_NAME "ITEM_SOURCES_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/model/spawn_location.json" VARIABLE_NAME "SPAWN_LOCATIONS_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/model/hint_source.json" VARIABLE_NAME "HINT_SOURCES_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/model/world_macro_region.json" VARIABLE_NAME "WORLD_MACRO_REGIONS_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/model/world_teleport_tree.json" VARIABLE_NAME "WORLD_TELEPORT_TREES_JSON" TEXT_FILE)

# Wrap binary assets files into automated .hxx headers
wrap_file(SOURCE_FILE "src/assets/blue_jewel.bin" VARIABLE_NAME "BLUE_JEWEL_SPRITE" SPRITE_FILE)
wrap_file(SOURCE_FILE "src/assets/green_jewel.bin" VARIABLE_NAME "GREEN_JEWEL_SPRITE" SPRITE_FILE)
wrap_file(SOURCE_FILE "src/assets/yellow_jewel.bin" VARIABLE_NAME "YELLOW_JEWEL_SPRITE" SPRITE_FILE)

add_compile_definitions(RELEASE="${PROJECT_VERSION}")
add_compile_definitions(MAJOR_RELEASE="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

set(SOURCES
    "src/extlibs/base64.hpp"
    "src/extlibs/base64.cpp"
    "src/extlibs/json.hpp"

    "src/huffman/huffman_tree.hpp"
    "src/huffman/huffman_tree_node.hpp"
    "src/huffman/symbols.hpp"
    "src/huffman/text_decoder.hpp"
    "src/huffman/text_encoder.hpp"
   
    "src/md_tools/code.hpp"
    "src/md_tools/code.cpp"
    "src/md_tools/rom.hpp"
    "src/md_tools/rom.cpp"
    "src/md_tools/types.hpp"

    "src/tools/argument_dictionary.hpp"
    "src/tools/bitfield.hpp"
    "src/tools/unsorted_set.hpp"
    "src/tools/tools.hpp"

    "src/model/hint_source.hpp"
    "src/model/item_source.hpp"
    "src/model/item_source.cpp"
    "src/model/item.hpp"
    "src/model/item.cpp"
    "src/model/spawn_location.hpp"
    "src/model/world_macro_region.hpp"
    "src/model/world_path.hpp"
    "src/model/world_path.cpp"
    "src/model/world_region.hpp"
    "src/model/world_teleport_tree.hpp"

    "src/assets.cpp"
    "src/assets.hpp"
    "src/exceptions.hpp"
    "src/game_patches.cpp"
    "src/game_patches.hpp"
    "src/game_text.hpp"
    "src/game_text.cpp"
    "src/randomizer_options.hpp"
    "src/randomizer_options.cpp"
    "src/world.cpp"
    "src/world.hpp"
    "src/world_randomizer.hpp"
    "src/world_randomizer.cpp"
    "src/world_solver.hpp"
    "src/world_solver.cpp"
    
    "src/main.cpp"
)

add_executable(randstalker "${SOURCES}")

