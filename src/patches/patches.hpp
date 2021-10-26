#include "../tools/megadrive/rom.hpp"
#include "../tools/megadrive/code.hpp"
#include "../randomizer_options.hpp"
#include "../world.hpp"

void patch_game_init(md::ROM& rom, const RandomizerOptions& options, const World& world);
void patch_story_flag_reading(md::ROM& rom, const RandomizerOptions& options, const World& world);
void patch_item_behavior(md::ROM& rom, const RandomizerOptions& options, const World& world);

void apply_other_patches(md::ROM& rom, const RandomizerOptions& options, const World& world);