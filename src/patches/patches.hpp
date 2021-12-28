#pragma once

#include <landstalker_lib/md_tools.hpp>
#include <landstalker_lib/model/world.hpp>
#include <landstalker_lib/patches/patches.hpp>
#include "../randomizer_options.hpp"
#include "../world_shuffler.hpp"
#include "../personal_settings.hpp"

class RandomizerWorld;

// Randomizer patches
void alter_hint_provider_dialogues(md::ROM& rom);
void alter_randomizer_credits(md::ROM& rom);
void alter_randomizer_title(md::ROM& rom);
void apply_rando_world_edits(md::ROM& rom, World& world, bool fix_armlet_skip);
void handle_fox_hints(md::ROM& rom, RandomizerWorld& world);
void optimize_maps(World& world);
void replace_copy_save_by_show_hash(md::ROM& rom, const std::string& seed_hash_sentence);
void shorten_cutscenes(md::ROM& rom);

// Not yet processed patches
void patch_rando_adaptations(md::ROM& rom, const RandomizerOptions& options, World& world);

// Events
void christmas_event(md::ROM& rom, World& world);

inline void apply_randomizer_patches(md::ROM& rom, RandomizerWorld& world, const RandomizerOptions& options, const PersonalSettings& personal_settings)
{
    optimize_maps(world);

    add_functions_to_items_on_use(rom, world, options.consumable_record_book());
    add_statue_of_jypta_effect(rom);
    alter_fahl_challenge(rom, world);
    alter_gold_rewards_handling(rom, world);
    alter_lantern_handling(rom, world);
    alter_nigel_colors(rom, personal_settings.nigel_colors());
    alter_ui_color(rom, personal_settings.hud_color());
    fix_hud_tilemap(rom);
    fix_item_checks(rom);
    improve_engine(rom);
    handle_additional_jewels(rom, world, options.jewel_count());
    make_sword_of_gaia_work_in_volcano(rom);
    normalize_special_enemies_hp(rom);
    make_lifestocks_give_specific_health(rom, options.health_gained_per_lifestock());

    if(options.remove_tree_cutting_glitch_drops())
        remove_tree_cutting_glitch_drops(rom);

    if (options.use_armor_upgrades())
        handle_armor_upgrades(rom);

    make_pawn_ticket_consumable(rom);
    make_key_not_consumed_on_use(rom);

    alter_item_order_in_menu(rom);
    quicken_gaia_effect(rom);
    quicken_pawn_ticket_effect(rom);

    make_massan_elder_reward_not_story_dependant(rom, world);
    make_lumberjack_reward_not_story_dependant(rom);
    change_falling_ribbon_position(rom);
    make_tibor_always_open(rom);
    make_gumi_boulder_push_not_story_dependant(world);
    make_falling_ribbon_not_story_dependant(world);

    alter_hint_provider_dialogues(rom);
    alter_randomizer_credits(rom);
    alter_randomizer_title(rom);
    apply_rando_world_edits(rom, world, options.fix_armlet_skip());
    replace_copy_save_by_show_hash(rom, options.hash_sentence());
    shorten_cutscenes(rom);

    handle_fox_hints(rom, world);
    patch_rando_adaptations(rom, options, world);
    patch_game_init(rom, world, personal_settings.add_ingame_item_tracker());

    if(options.christmas_event())
        christmas_event(rom, world);
}
