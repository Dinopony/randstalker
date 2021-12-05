#pragma once

class World;
class RandomizerOptions;

void patch_items(World& world, const RandomizerOptions& options);
void patch_entity_types(World& world, const RandomizerOptions& options);
void patch_game_strings(World& world, const RandomizerOptions& options);