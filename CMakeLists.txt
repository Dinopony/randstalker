cmake_minimum_required(VERSION 3.21.0)

project(randstalker VERSION 1.6.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif ()

add_compile_definitions(RELEASE="${PROJECT_VERSION}")
add_compile_definitions(MAJOR_RELEASE="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

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
wrapped_dependency(TEXT     src/logic_model/data/item_distribution.json     ITEM_DISTRIBUTIONS_JSON)
wrapped_dependency(TEXT     src/assets/game_strings.json                    GAME_STRINGS_JSON)

set(SOURCES
        # --- Auto-generated dependencies ----------------------------------------
        "src/assets/game_strings.json.hxx"
        "src/logic_model/data/world_node.json.hxx"
        "src/logic_model/data/world_path.json.hxx"
        "src/logic_model/data/world_region.json.hxx"
        "src/logic_model/data/hint_source.json.hxx"
        "src/logic_model/data/spawn_location.json.hxx"
        "src/logic_model/data/item_distribution.json.hxx"

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
        "src/logic_model/world_logic.hpp"
        "src/logic_model/world_logic.cpp"

        # --- Patches ----------------------------------------
        "src/patches/alter_hint_provider_dialogues.cpp"
        "src/patches/alter_randomizer_credits.cpp"
        "src/apply_randomizer_options.cpp"
        "src/patches/rando_world_edits.cpp"
        "src/patches/replace_copy_by_hash.cpp"
        "src/patches/shorten_cutscenes.cpp"

        "src/patches/patches.hpp"
        "src/patches/rando_adaptations.cpp"

        # --- Tools ----------------------------------------
        "src/tools/base64.hpp"
        "src/tools/base64.cpp"

        # --- Project root ----------------------------------------
        "src/apply_randomizer_options.hpp"
        "src/apply_randomizer_options.cpp"
        "src/randomizer_options.hpp"
        "src/randomizer_options.cpp"
        "src/world_randomizer.hpp"
        "src/world_randomizer.cpp"
        "src/world_solver.hpp"
        "src/world_solver.cpp"
        "src/main.cpp"
        )

add_executable(randstalker "${SOURCES}")
target_link_libraries(randstalker landstalker_lib)
