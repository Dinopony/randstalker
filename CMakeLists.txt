cmake_minimum_required(VERSION 3.1.0)

include("CMakeFunctions.txt")

project(randstalker VERSION 1.6.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

# Wrap JSON model files into automated .hxx headers
wrap_file(SOURCE_FILE "src/world_model/data/entity_type.json" VARIABLE_NAME "ENTITIES_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/world_model/data/item.json" VARIABLE_NAME "ITEMS_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/world_model/data/item_source.json" VARIABLE_NAME "ITEM_SOURCES_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/world_model/data/spawn_location.json" VARIABLE_NAME "SPAWN_LOCATIONS_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/world_model/data/world_teleport_tree.json" VARIABLE_NAME "WORLD_TELEPORT_TREES_JSON" TEXT_FILE)

wrap_file(SOURCE_FILE "src/logic_model/data/world_node.json" VARIABLE_NAME "WORLD_NODES_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/logic_model/data/world_path.json" VARIABLE_NAME "WORLD_PATHS_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/logic_model/data/world_region.json" VARIABLE_NAME "WORLD_REGIONS_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/logic_model/data/hint_source.json" VARIABLE_NAME "HINT_SOURCES_JSON" TEXT_FILE)

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

    "src/world_model/entity_type.hpp"
    "src/world_model/entity_type.cpp"
    "src/world_model/entity.hpp"
    "src/world_model/entity.cpp"
    "src/world_model/item_source.hpp"
    "src/world_model/item_source.cpp"
    "src/world_model/item.hpp"
    "src/world_model/item.cpp"
    "src/world_model/map_connection.hpp"
    "src/world_model/map_palette.hpp"
    "src/world_model/map_palette.cpp"
    "src/world_model/map.hpp"
    "src/world_model/map.cpp"
    "src/world_model/spawn_location.hpp"
    "src/world_model/world_teleport_tree.hpp"
    "src/world_model/world.cpp"
    "src/world_model/world.hpp"

    "src/logic_model/hint_source.hpp"
    "src/logic_model/hint_source.cpp"
    "src/logic_model/world_node.hpp"
    "src/logic_model/world_path.hpp"
    "src/logic_model/world_path.cpp"
    "src/logic_model/world_region.hpp"
    "src/logic_model/world_region.cpp"
    "src/logic_model/world_logic.hpp"
    "src/logic_model/world_logic.cpp"

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

    "src/writers/graphviz_writer.cpp"
    "src/writers/model_writer.cpp"
    "src/writers/spoiler_writer.cpp"
    "src/writers/writers.hpp"

    "src/exceptions.hpp"
    "src/randomizer_options.hpp"
    "src/randomizer_options.cpp"
    "src/randomizer_patches_extern.cpp"
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
