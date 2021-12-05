#pragma once

#include <md_tools.hpp>

#include "../randomizer_options.hpp"
#include "../world_model/world.hpp"

// Global patches
void add_functions_to_items_on_use(md::ROM& rom, bool consumable_record_book);
void add_statue_of_jypta_effect(md::ROM& rom);
void alter_gold_rewards_handling(md::ROM& rom, World& world);
void alter_lantern_handling(md::ROM& rom);
void alter_ui_color(md::ROM& rom, uint16_t ui_color);
void fix_hud_tilemap(md::ROM& rom);
void fix_item_checks(md::ROM& rom);
void patch_game_init(md::ROM& rom, const World& world, bool add_ingame_tracker);
void handle_additional_jewels(md::ROM& rom, World& world, WorldLogic& logic, uint8_t jewel_count);

// alter_items_consumability.cpp patches
void make_pawn_ticket_consumable(md::ROM& rom);
void make_key_not_consumed_on_use(md::ROM& rom);

// quality_of_life.cpp patches
void alter_item_order_in_menu(md::ROM& rom);
void quicken_gaia_effect(md::ROM& rom);

// story_dependencies.cpp patches
void make_massan_elder_reward_not_story_dependant(md::ROM& rom);
void make_lumberjack_reward_not_story_dependant(md::ROM& rom);
void change_falling_ribbon_position(md::ROM& rom);
void make_tibor_always_open(md::ROM& rom);
void make_gumi_boulder_push_not_story_dependant(World& world);
void make_falling_ribbon_not_story_dependant(World& world);

// Randomizer patches
void alter_hint_provider_dialogues(md::ROM& rom);
void alter_randomizer_credits(md::ROM& rom);
void apply_rando_world_edits(md::ROM& rom, World& world, bool fix_armlet_skip);
void replace_copy_save_by_show_hash(md::ROM& rom, const std::string& seed_hash_sentence);
void shorten_cutscenes(md::ROM& rom);

// Not yet processed patches
void patch_rando_adaptations(md::ROM& rom, const RandomizerOptions& options, const World& world);
void apply_rando_options_to_world(const RandomizerOptions& options, World& world);

void apply_kaizo_edits(World& world, md::ROM& rom);

inline void apply_randomizer_patches(md::ROM& rom, World& world, WorldLogic& logic, const RandomizerOptions& options)
{
    add_functions_to_items_on_use(rom, options.consumable_record_book());
    add_statue_of_jypta_effect(rom);
    alter_gold_rewards_handling(rom, world);
    alter_lantern_handling(rom);
    alter_ui_color(rom, options.hud_color());
    fix_hud_tilemap(rom);
    fix_item_checks(rom);
    patch_game_init(rom, world, options.add_ingame_item_tracker());
    handle_additional_jewels(rom, world, logic, options.jewel_count());
    
    make_pawn_ticket_consumable(rom);
    make_key_not_consumed_on_use(rom);

    alter_item_order_in_menu(rom);
    quicken_gaia_effect(rom);

    make_massan_elder_reward_not_story_dependant(rom);
    make_lumberjack_reward_not_story_dependant(rom);
    change_falling_ribbon_position(rom);
    make_tibor_always_open(rom);
    make_gumi_boulder_push_not_story_dependant(world);
    make_falling_ribbon_not_story_dependant(world);

    alter_hint_provider_dialogues(rom);
    alter_randomizer_credits(rom);
    apply_rando_world_edits(rom, world, options.fix_armlet_skip());
    replace_copy_save_by_show_hash(rom, options.hash_sentence());
    shorten_cutscenes(rom);
    
    apply_rando_options_to_world(options, world);
    patch_rando_adaptations(rom, options, world);
}