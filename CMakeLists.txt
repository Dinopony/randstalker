cmake_minimum_required(VERSION 3.21)

project(randstalker VERSION 1.8.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(RELEASE="${PROJECT_VERSION}")
add_compile_definitions(MAJOR_RELEASE=${PROJECT_VERSION_MAJOR}${PROJECT_VERSION_MINOR})

if(DEBUG)
    add_compile_definitions(DEBUG)
endif()

add_subdirectory(extlibs/landstalker_lib/landstalker-lib landstalker_lib)
include_directories("extlibs/landstalker_lib")

macro(WRAPPED_DEPENDENCY)
    message(STATUS "Defining wrapped dependency for ${CMAKE_CURRENT_SOURCE_DIR}/${ARGV1}")
    add_custom_command(
            OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/${ARGV1}.hxx"
            COMMAND "${CMAKE_COMMAND}"
            ARGS -DINPUT_FILE="${ARGV1}" -DVARIABLE_NAME="${ARGV2}" -DPROCESS_TYPE="${ARGV0}" -P "extlibs/landstalker_lib/landstalker-lib/wrap_file.cmake"
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
wrapped_dependency(TEXT     src/logic_model/data/item_source.json           ITEM_SOURCES_JSON)
wrapped_dependency(TEXT     src/logic_model/data/world_teleport_tree.json   WORLD_TELEPORT_TREES_JSON)
wrapped_dependency(TEXT     src/assets/game_strings.json                    GAME_STRINGS_JSON)
wrapped_dependency(RAW      src/assets/christmas_moneybag.bin               CHRISTMAS_MONEYBAG)
wrapped_dependency(RAW      src/assets/blue_jewel.bin                       BLUE_JEWEL_SPRITE)
wrapped_dependency(RAW      src/assets/green_jewel.bin                      GREEN_JEWEL_SPRITE)
wrapped_dependency(RAW      src/assets/yellow_jewel.bin                     YELLOW_JEWEL_SPRITE)
wrapped_dependency(RAW      src/assets/archipelago_item.bin                 ARCHIPELAGO_ITEM_SPRITE)
wrapped_dependency(RAW      src/assets/secret_arg_music_bank.bin            ARG_MUSIC_BANK)

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
        "src/assets/blue_jewel.bin.hxx"
        "src/assets/green_jewel.bin.hxx"
        "src/assets/yellow_jewel.bin.hxx"
        "src/assets/archipelago_item.bin.hxx"
        "src/assets/secret_arg_music_bank.bin.hxx"

        "src/constants/rando_constants.hpp"

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
        "src/logic_model/data/item_source.json.hxx"
        "src/logic_model/data/world_teleport_tree.json.hxx"

        # --- I/O ----------------------------------------
        "src/io/io.hpp"
        "src/io/graphviz_writer.cpp"
        "src/io/model_writer.cpp"
        "src/io/world_json_writer.cpp"
        "src/io/world_json_parser.cpp"

        # --- Logic model ----------------------------------------
        "src/logic_model/hint_source.hpp"
        "src/logic_model/hint_source.cpp"
        "src/logic_model/item_distribution.hpp"
        "src/logic_model/item_source.hpp"
        "src/logic_model/item_source.cpp"
        "src/logic_model/world_node.hpp"
        "src/logic_model/world_path.hpp"
        "src/logic_model/world_path.cpp"
        "src/logic_model/world_region.hpp"
        "src/logic_model/world_region.cpp"
        "src/logic_model/world_teleport_tree.hpp"
        "src/logic_model/randomizer_world.hpp"
        "src/logic_model/randomizer_world.cpp"

        # --- Patches ----------------------------------------
        "src/patches/edit_content/patch_credits.hpp"
        "src/patches/edit_content/patch_title_screen.hpp"
        "src/patches/edit_content/patch_make_ryuma_mayor_saveable.hpp"
        "src/patches/edit_content/patch_fahl_challenge.hpp"
        "src/patches/edit_content/patch_edit_dialogues.hpp"
        "src/patches/edit_content/patch_rando_world_edits.hpp"
        "src/patches/edit_content/patch_shorten_cutscenes.hpp"
        "src/patches/edit_content/patch_add_foxies.hpp"
        "src/patches/edit_content/patch_add_golas_heart.hpp"
        "src/patches/edit_content/patch_add_golas_heart.cpp"

        "src/patches/item_effects/patch_lithograph_hint_on_use.hpp"
        "src/patches/item_effects/patch_oracle_stone_hint_on_use.hpp"
        "src/patches/item_effects/patch_spell_book_teleport_on_use.hpp"
        "src/patches/item_effects/patch_record_book_save_on_use.hpp"
        "src/patches/item_effects/patch_consumable_pawn_ticket.hpp"

        "src/patches/gameplay_tweaks/patch_sword_of_gaia_in_volcano.hpp"
        "src/patches/gameplay_tweaks/patch_remove_tree_cutting_glitch_drops.hpp"
        "src/patches/gameplay_tweaks/patch_faster_pawn_ticket.hpp"
        "src/patches/gameplay_tweaks/patch_faster_gaia_effect.hpp"
        "src/patches/gameplay_tweaks/patch_armor_upgrades.hpp"
        "src/patches/gameplay_tweaks/patch_set_lifestocks_health.hpp"
        "src/patches/gameplay_tweaks/patch_remove_tibor_requirement.hpp"
        "src/patches/gameplay_tweaks/patch_allow_whistle_usage_behind_trees.hpp"
        "src/patches/gameplay_tweaks/patch_remove_ekeeke_auto_revive.hpp"
        "src/patches/gameplay_tweaks/patch_show_all_books_in_churches.hpp"
        "src/patches/gameplay_tweaks/patch_remove_continue_prompt_after_saving.hpp"
        "src/patches/gameplay_tweaks/patch_alternative_goals.hpp"
        "src/patches/gameplay_tweaks/patch_apply_difficulty_factors.hpp"

        "src/patches/technical/patch_apply_item_sources.hpp"
        "src/patches/technical/patch_handle_jewels.hpp"
        "src/patches/technical/patch_remove_story_dependencies.hpp"
        "src/patches/technical/patch_fix_item_checks.hpp"
        "src/patches/technical/patch_improve_lantern_handling.hpp"
        "src/patches/technical/patch_improve_gold_rewards_handling.hpp"
        "src/patches/technical/patch_remove_music.hpp"
        "src/patches/technical/patch_swap_overworld_music.hpp"
        "src/patches/technical/patch_fix_softlocks.hpp"
        "src/patches/technical/patch_fix_armlet_skip.hpp"
        "src/patches/technical/patch_randomizer_adaptations.hpp"
        "src/patches/technical/patch_save_select_show_hash_option.hpp"
        "src/patches/technical/patch_flags_for_ground_items.hpp"
        "src/patches/technical/patch_update_teleport_tree_connections.hpp"
        "src/patches/technical/patch_on_walk_effects.hpp"
        "src/patches/technical/patch_individual_price_for_shop_sources.hpp"
        "src/patches/technical/patch_handle_custom_dialogues.hpp"

        "src/patches/optimization/patch_optimize_maps.hpp"
        "src/patches/optimization/patch_optimize_destel_well_map.hpp"
        "src/patches/optimization/patch_optimize_swamp_shrine_map.hpp"
        "src/patches/optimization/patch_optimize_tibor_maps.hpp"
        "src/patches/optimization/patch_optimize_lake_shrine_platforms_map.hpp"

        "src/patches/events/patch_season_theme.hpp"
        "src/patches/events/patch_secret_arg.hpp"
        "src/patches/events/patch_secret_arg.cpp"

        "src/patches/patches.hpp"
        "src/patches/patches.cpp"

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
        "src/standalone_logic_solver.cpp"
        "src/bingo.hpp"
        "src/main.cpp")

add_executable(randstalker "${SOURCES}")
target_link_libraries(randstalker landstalker_lib)
