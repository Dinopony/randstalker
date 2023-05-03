#pragma once

#include <landstalker-lib/md_tools.hpp>

class RandomizerWorld;
class RandomizerOptions;
class PersonalSettings;

void apply_randomizer_patches(md::ROM& rom, RandomizerWorld& world, const RandomizerOptions& options, const PersonalSettings& personal_settings);