cmake_minimum_required(VERSION 3.1.0)

include("CMakeFunctions.txt")

project(randstalker VERSION 1.6.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif ()

# Wrap JSON model files into automated .hxx headers
wrap_file(SOURCE_FILE "src/logic_model/data/world_node.json" VARIABLE_NAME "WORLD_NODES_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/logic_model/data/world_path.json" VARIABLE_NAME "WORLD_PATHS_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/logic_model/data/world_region.json" VARIABLE_NAME "WORLD_REGIONS_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/logic_model/data/hint_source.json" VARIABLE_NAME "HINT_SOURCES_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/logic_model/data/spawn_location.json" VARIABLE_NAME "SPAWN_LOCATIONS_JSON" TEXT_FILE)
wrap_file(SOURCE_FILE "src/logic_model/data/item_distribution.json" VARIABLE_NAME "ITEM_DISTRIBUTIONS_JSON" TEXT_FILE)
# Wrap assets files into automated .hxx headers
wrap_file(SOURCE_FILE "src/assets/game_strings.json" VARIABLE_NAME "GAME_STRINGS_JSON" TEXT_FILE)

add_compile_definitions(RELEASE="${PROJECT_VERSION}")
add_compile_definitions(MAJOR_RELEASE="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

add_subdirectory(extlibs/landstalker_lib landstalker_lib)

include_directories("extlibs/")

set(SOURCES
        # --- Extlibs ----------------------------------------
        "src/tools/base64.hpp"
        "src/tools/base64.cpp"

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
