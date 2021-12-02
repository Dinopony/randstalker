cmake_minimum_required(VERSION 3.1.0)

include("CMakeFunctions.txt")

project(randstalker VERSION 1.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

# Wrap JSON model files into automated .hxx headers
wrap_file(SOURCE_FILE "src/model/entity_type.json" VARIABLE_NAME "ENTITIES_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/model/item.json" VARIABLE_NAME "ITEMS_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/model/world_node.json" VARIABLE_NAME "WORLD_NODES_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/model/world_path.json" VARIABLE_NAME "WORLD_PATHS_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/model/item_source.json" VARIABLE_NAME "ITEM_SOURCES_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/model/spawn_location.json" VARIABLE_NAME "SPAWN_LOCATIONS_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/model/hint_source.json" VARIABLE_NAME "HINT_SOURCES_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/model/world_region.json" VARIABLE_NAME "WORLD_REGIONS_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/model/world_teleport_tree.json" VARIABLE_NAME "WORLD_TELEPORT_TREES_JSON" TEXT_FILE)

# Wrap assets files into automated .hxx headers
wrap_file(SOURCE_FILE "src/assets/fixed_hud_tilemap.bin" VARIABLE_NAME "FIXED_HUD_TILEMAP")
wrap_file(SOURCE_FILE "src/assets/blue_jewel.bin" VARIABLE_NAME "BLUE_JEWEL_SPRITE" SPRITE_FILE)
wrap_file(SOURCE_FILE "src/assets/green_jewel.bin" VARIABLE_NAME "GREEN_JEWEL_SPRITE" SPRITE_FILE)
wrap_file(SOURCE_FILE "src/assets/yellow_jewel.bin" VARIABLE_NAME "YELLOW_JEWEL_SPRITE" SPRITE_FILE)
wrap_file(SOURCE_FILE "src/assets/game_strings.json" VARIABLE_NAME "GAME_STRINGS_JSON" TEXT_FILE)

add_compile_definitions(RELEASE="${PROJECT_VERSION}")
add_compile_definitions(MAJOR_RELEASE="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

set(SOURCES
    "src/constants/entity_type_codes.hpp"
    "src/constants/item_codes.hpp"
    "src/constants/map_codes.hpp"
    "src/constants/offsets.hpp"

    "src/extlibs/base64.hpp"
    "src/extlibs/base64.cpp"
    "src/extlibs/json.hpp"
   
    "src/tools/argument_dictionary.hpp"
    "src/tools/bitfield.hpp"
    "src/tools/bitpack.hpp"
    "src/tools/byte_array.hpp"
    "src/tools/game_text.hpp"
    "src/tools/game_text.cpp"
    "src/tools/graphviz.hpp"
    "src/tools/graphviz.cpp"
    "src/tools/unsorted_set.hpp"
    "src/tools/tools.hpp"
    "src/tools/textbanks_decoder.hpp"
    "src/tools/textbanks_decoder.cpp"
    "src/tools/textbanks_encoder.hpp"
    "src/tools/textbanks_encoder.cpp"

        "src/tools/huffman/tree.hpp"
        "src/tools/huffman/tree_node.hpp"
        "src/tools/huffman/symbols.hpp"

        "src/tools/megadrive/code.hpp"
        "src/tools/megadrive/code.cpp"
        "src/tools/megadrive/rom.hpp"
        "src/tools/megadrive/rom.cpp"
        "src/tools/megadrive/types.hpp"

    "src/model/entity_type.hpp"
    "src/model/entity_type.cpp"
    "src/model/entity.hpp"
    "src/model/entity.cpp"
    "src/model/hint_source.hpp"
    "src/model/hint_source.cpp"
    "src/model/item_source.hpp"
    "src/model/item_source.cpp"
    "src/model/item.hpp"
    "src/model/item.cpp"
    "src/model/map_connection.hpp"
    "src/model/map_palette.hpp"
    "src/model/map_palette.cpp"
    "src/model/map.hpp"
    "src/model/map.cpp"
    "src/model/spawn_location.hpp"
    "src/model/world_node.hpp"
    "src/model/world_path.hpp"
    "src/model/world_path.cpp"
    "src/model/world_region.hpp"
    "src/model/world_region.cpp"
    "src/model/world_teleport_tree.hpp"

    "src/patches/patches.hpp"
    "src/patches/game_init.cpp"
    "src/patches/item_behavior.cpp"
    "src/patches/quality_of_life.cpp"
    "src/patches/rando_adaptations.cpp"
    "src/patches/story_dependencies.cpp"
    "src/patches/world_edits.cpp"
    
    "src/patches/kaizo/kaizo.hpp"
    "src/patches/kaizo/kaizo.cpp"
    "src/patches/kaizo/00_engine.cpp"
    "src/patches/kaizo/01_safety_pass_arc.cpp"
    "src/patches/kaizo/02_mercator.cpp"
    "src/patches/kaizo/03_mercator_castle.cpp"

    "src/exceptions.hpp"
    "src/randomizer_options.hpp"
    "src/randomizer_options.cpp"
    "src/world.cpp"
    "src/world.hpp"
    "src/world_randomizer.hpp"
    "src/world_randomizer.cpp"
    "src/world_reader.hpp"
    "src/world_reader.cpp"
    "src/world_solver.hpp"
    "src/world_solver.cpp"
    "src/world_writer.hpp"
    "src/world_writer.cpp"
    "src/main.cpp"
)

add_executable(randstalker "${SOURCES}")
