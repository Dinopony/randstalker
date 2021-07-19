#pragma once

namespace md { class ROM; }
class RandomizerOptions;
class World;

void applyPatches(md::ROM& rom, const RandomizerOptions& options, const World& world);