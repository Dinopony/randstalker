#pragma once

#include "../tools/megadrive/rom.hpp"
#include "../tools/megadrive/code.hpp"
#include "../randomizer_options.hpp"
#include "../world.hpp"

void patch_game_init(md::ROM& rom, const RandomizerOptions& options, const World& world);
void patch_item_behavior(md::ROM& rom, const RandomizerOptions& options, const World& world);
void patch_quality_of_life(md::ROM& rom, const RandomizerOptions& options, const World& world);
void patch_rando_adaptations(md::ROM& rom, const RandomizerOptions& options, const World& world);
void patch_softlocks(md::ROM& rom, const RandomizerOptions& options, const World& world);
void patch_story_dependencies(md::ROM& rom, const RandomizerOptions& options, const World& world);

void apply_world_edits(World& world, const RandomizerOptions& options, md::ROM& rom);

inline void apply_game_patches(md::ROM& rom, const RandomizerOptions& options, const World& world)
{
    patch_game_init(rom, options, world);
    patch_item_behavior(rom, options, world);
    patch_quality_of_life(rom, options, world);
    patch_rando_adaptations(rom, options, world);
    patch_softlocks(rom, options, world);
    patch_story_dependencies(rom, options, world);
}