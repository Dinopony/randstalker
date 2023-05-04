#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include "../../randomizer_options.hpp"

class PatchSecretARG : public GamePatch
{
private:
    Map* _room_1 = nullptr;
    Map* _room_2 = nullptr;
    Map* _room_3 = nullptr;
    Map* _room_4 = nullptr;
    Map* _room_5 = nullptr;
    Map* _room_6 = nullptr;

public:
    PatchSecretARG() = default;

    void alter_rom(md::ROM& rom) override
    {
        // Replace Dexter voice pitch info (now unused) by Nole voice pitch info
        rom.set_word(0x29124, 0xA572); // A5 = SPR_NOLE, 72 = SND_Fireball2
    }

    void alter_world(World& world) override;
    void inject_code(md::ROM& rom, World& world) override;
};
