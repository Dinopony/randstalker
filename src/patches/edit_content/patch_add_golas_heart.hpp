#pragma once

#include <landstalker-lib/patches/game_patch.hpp>

/**
 * This patch adds Gola's Heart, the additional dungeon featured in "The Secret" ARG released on 2022.
 */
class PatchAddGolasHeart : public GamePatch
{
public:
    void inject_code(md::ROM& rom, World& world);
};
