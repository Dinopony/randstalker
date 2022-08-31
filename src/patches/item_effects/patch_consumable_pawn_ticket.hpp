#pragma once

#include "landstalker_lib/patches/game_patch.hpp"
#include "landstalker_lib/model/item.hpp"
#include "landstalker_lib/constants/item_codes.hpp"

/**
 * The effect of Pawn Ticket fits very well one of a consumable item, but didn't work this way in the original game.
 * Instead, a story flag was set when using it, preventing from using it ever again if we were to find another one
 * (which was not possible in the vanilla game).
 *
 * This function kills two birds with one stone, both making it consumable and removing that flag check.
 */
class PatchConsumablePawnTicket : public GamePatch
{
public:
    void alter_world(World& world) override
    {
        world.item(ITEM_PAWN_TICKET)->max_quantity(9);
    }

    void inject_code(md::ROM& rom, World& world) override
    {
        md::Code proc_consume_pawn_ticket;
        {
            proc_consume_pawn_ticket.jsr(0x8B98); // ConsumeItem
            proc_consume_pawn_ticket.nop(3);
        }
        rom.set_code(0x88D2, proc_consume_pawn_ticket);
    }
};
