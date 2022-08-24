cmake_minimum_required(VERSION 3.21)

project(randstalker VERSION 1.7.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(RELEASE="${PROJECT_VERSION}")
add_compile_definitions(MAJOR_RELEASE=${PROJECT_VERSION_MAJOR}${PROJECT_VERSION_MINOR})

if(DEBUG)
    add_compile_definitions(DEBUG)
endif()

add_subdirectory(extlibs/landstalker_lib landstalker_lib)
include_directories("extlibs/")

macro(WRAPPED_DEPENDENCY)
    message(STATUS "Defining wrapped dependency for ${CMAKE_CURRENT_SOURCE_DIR}/${ARGV1}")
    add_custom_command(
            OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/${ARGV1}.hxx"
            COMMAND "${CMAKE_COMMAND}"
            ARGS -DINPUT_FILE="${ARGV1}" -DVARIABLE_NAME="${ARGV2}" -DPROCESS_TYPE="${ARGV0}" -P "extlibs/landstalker_lib/wrap_file.cmake"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${ARGV1}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endmacro()

# Wrap JSON model files and assets into automated .hxx headers
wrapped_dependency(TEXT     src/logic_model/data/world_node.json            WORLD_NODES_JSON)
wrapped_dependency(TEXT     src/logic_model/data/world_path.json            WORLD_PATHS_JSON)
wrapped_dependency(TEXT     src/logic_model/data/world_region.json          WORLD_REGIONS_JSON)
wrapped_dependency(TEXT     src/logic_model/data/hint_source.json           HINT_SOURCES_JSON)
wrapped_dependency(TEXT     src/logic_model/data/spawn_location.json        SPAWN_LOCATIONS_JSON)
wrapped_dependency(TEXT     src/logic_model/data/item.json                  ITEMS_JSON)
wrapped_dependency(TEXT     src/logic_model/data/world_teleport_tree.json   WORLD_TELEPORT_TREES_JSON)
wrapped_dependency(TEXT     src/assets/game_strings.json                    GAME_STRINGS_JSON)
wrapped_dependency(RAW      src/assets/christmas_moneybag.bin               CHRISTMAS_MONEYBAG)

wrapped_dependency(RAW      src/assets/titlescreen/title1_layout.rle        TITLE1_LAYOUT)
wrapped_dependency(RAW      src/assets/titlescreen/title1_tiles.lz77        TITLE1_TILES)
wrapped_dependency(RAW      src/assets/titlescreen/title2_layout.rle        TITLE2_LAYOUT)
wrapped_dependency(RAW      src/assets/titlescreen/title2_tiles.lz77        TITLE2_TILES)
wrapped_dependency(RAW      src/assets/titlescreen/title3_layout.rle        TITLE3_LAYOUT)
wrapped_dependency(RAW      src/assets/titlescreen/title3_tiles.lz77        TITLE3_TILES)

set(SOURCES
        # --- Auto-generated dependencies ----------------------------------------
        "src/assets/game_strings.json.hxx"
        "src/assets/christmas_moneybag.bin.hxx"
        "src/assets/titlescreen/title1_layout.rle.hxx"
        "src/assets/titlescreen/title1_tiles.lz77.hxx"
        "src/assets/titlescreen/title2_layout.rle.hxx"
        "src/assets/titlescreen/title2_tiles.lz77.hxx"
        "src/assets/titlescreen/title3_layout.rle.hxx"
        "src/assets/titlescreen/title3_tiles.lz77.hxx"

        "src/logic_model/data/world_node.json.hxx"
        "src/logic_model/data/world_path.json.hxx"
        "src/logic_model/data/world_region.json.hxx"
        "src/logic_model/data/hint_source.json.hxx"
        "src/logic_model/data/spawn_location.json.hxx"
        "src/logic_model/data/item.json.hxx"
        "src/logic_model/data/world_teleport_tree.json.hxx"

        # --- I/O ----------------------------------------
        "src/io/io.hpp"
        "src/io/graphviz_writer.cpp"
        "src/io/model_writer.cpp"
        "src/io/spoiler_writer.cpp"
        "src/io/world_json_parser.cpp"

        # --- Logic model ----------------------------------------
        "src/logic_model/hint_source.hpp"
        "src/logic_model/hint_source.cpp"
        "src/logic_model/item_distribution.hpp"
        "src/logic_model/world_node.hpp"
        "src/logic_model/world_path.hpp"
        "src/logic_model/world_path.cpp"
        "src/logic_model/world_region.hpp"
        "src/logic_model/world_region.cpp"
        "src/logic_model/world_teleport_tree.hpp"
        "src/logic_model/randomizer_world.hpp"
        "src/logic_model/randomizer_world.cpp"

        # --- Patches ----------------------------------------
        "src/patches/alter_hint_provider_dialogues.cpp"
        "src/patches/alter_randomizer_credits.cpp"
        "src/patches/alter_randomizer_title.cpp"
        "src/patches/handle_fox_hints.cpp"
        "src/patches/handle_record_book.cpp"
        "src/patches/handle_spell_book_teleport.cpp"
        "src/patches/handle_hint_providing_items.cpp"
        "src/patches/optimize_maps.cpp"
        "src/patches/rando_world_edits.cpp"
        "src/patches/replace_copy_by_hash.cpp"
        "src/patches/shorten_cutscenes.cpp"
        "src/patches/event_christmas.cpp"
        
        "src/patches/patches.hpp"
        "src/patches/rando_adaptations.cpp"

        # --- Tools ----------------------------------------
        "src/tools/base64.hpp"
        "src/tools/base64.cpp"
        "src/tools/extract_nigel_sprites.hpp"

        # --- Project root ----------------------------------------
        "src/apply_randomizer_options.hpp"
        "src/apply_randomizer_options.cpp"
        "src/personal_settings.hpp"
        "src/personal_settings.cpp"
        "src/randomizer_options.hpp"
        "src/randomizer_options.cpp"
        "src/world_shuffler.hpp"
        "src/world_shuffler.cpp"
        "src/world_solver.hpp"
        "src/world_solver.cpp"
        "src/main.cpp"
        )

add_executable(randstalker "${SOURCES}")
target_link_libraries(randstalker landstalker_lib)
