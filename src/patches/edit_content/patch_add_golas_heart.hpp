#pragma once

#include <landstalker-lib/patches/game_patch.hpp>

/**
 * This patch adds Gola's Heart, the additional dungeon featured in "The Secret" ARG released on 2022.
 */
class PatchAddGolasHeart : public GamePatch
{
private:
    Map* _room_1 = nullptr;
    Map* _room_2 = nullptr;
    Map* _room_3 = nullptr;
    Map* _room_4 = nullptr;
    Map* _room_5 = nullptr;
    Map* _room_6 = nullptr;
    Map* _room_7 = nullptr;
    Map* _room_8 = nullptr;
    Map* _room_9 = nullptr;
    Map* _room_10 = nullptr;
    Map* _room_11 = nullptr;
    Map* _room_12 = nullptr;
    Map* _room_13 = nullptr;
    Map* _room_14 = nullptr;

    uint32_t _new_room_music_lut_addr = 0xFFFFFFFF;

public:
    void alter_world(World& world) override;
    void inject_data(md::ROM& rom, World& world) override;
    void inject_code(md::ROM& rom, World& world) override;
};
