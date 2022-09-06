#pragma once

#include "landstalker_lib/patches/game_patch.hpp"
#include "landstalker_lib/model/item.hpp"
#include "landstalker_lib/constants/item_codes.hpp"

/**
 * This patch makes the key not consumed on use, making it a unique item that needs to be used to open several doors.
 */
class PatchPermanentKey : public GamePatch
{
public:
    void alter_world(World& world) override
    {
        world.item(ITEM_KEY)->max_quantity(1);
    }

    void alter_rom(md::ROM& rom) override
    {
        rom.set_code(0x8B34, md::Code().nop());
    }
};
