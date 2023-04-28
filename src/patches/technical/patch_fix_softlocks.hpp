#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include <landstalker-lib/constants/map_codes.hpp>
#include <landstalker-lib/constants/entity_type_codes.hpp>
#include <landstalker-lib/constants/flags.hpp>
#include <landstalker-lib/model/map.hpp>
#include <landstalker-lib/model/entity.hpp>

class PatchFixSoftlocks : public GamePatch
{
public:
    void alter_rom(md::ROM& rom) override
    {
        alter_labyrinth_rafts(rom);
    }

    void alter_world(World& world) override
    {
        fix_mir_after_lake_shrine_softlock(world);
        fix_reverse_greenmaze_fountain_softlock(world);
        allow_going_backwards_in_knl_exterior(world);
        add_sailor_to_take_boat_again_on_docks(world);
    }

    void inject_code(md::ROM& rom, World& world) override
    {
        fix_fara_arena_chest_softlock(rom, world);
        fix_crypt_softlocks(rom, world);
    }

private:
    /**
     * Make it so Lifestock chest near Fara in Swamp Shrine appears again when going back into the room afterwards, preventing any softlock there.
     */
    static void fix_fara_arena_chest_softlock(md::ROM& rom, const World& world)
    {
        // Move the chest to the ground
        Entity* chest = world.map(MAP_SWAMP_SHRINE_BOSS_ROOM)->entity(14);
        chest->position(26, 19, 4);
        chest->behavior_id(0);

        // Function to remove all entities but the chest when coming back in the room
        md::Code func;
        func.movem_to_stack({ reg_D0_D7 }, { reg_A0_A6 });
        {
            func.lea(0xFF5480, reg_A0);
            func.moveq(0xD, reg_D0);
            func.label("loop_remove_entities");
            {
                func.movew(0x7F7F, addr_(reg_A0));
                func.adda(0x80, reg_A0);
            }
            func.dbra(reg_D0, "loop_remove_entities");
        }
        func.movem_from_stack({ reg_D0_D7 }, { reg_A0_A6 });
        func.rts();

        uint32_t addr = rom.inject_code(func);

        // Call the injected function
        rom.set_code(0x019BE0, md::Code().jsr(addr).nop());
    }

    /**
     * In the original game, coming back to Mir room after Lake Shrine would softlock you because Mir
     * would not be there. This check is removed to prevent any softlock and allow fighting Mir after having
     * done Lake Shrine.
     */
    static void fix_mir_after_lake_shrine_softlock(World& world)
    {
        world.map(MAP_MIR_TOWER_BOSS_ARENA)->global_entity_mask_flags().clear();
    }

    /**
     * Fix potential softlocks by taking Mercator fountain secret passage in reverse, and clipping inside the fountain
     * with no way out.
     * This adds platforms on the other side of the tunnel preventing the player from taking that path altogether as long
     * as fountain has not been opened.
     */
    static void fix_reverse_greenmaze_fountain_softlock(World& world)
    {
        Flag is_mercator_fountain_open(0xE, 1);

        Entity* platform_1 = new Entity({
                .type_id = ENTITY_SMALL_YELLOW_PLATFORM,
                .position = Position(21, 22, 0, false, false, false),
                .palette = 2
        });
        platform_1->remove_when_flag_is_set(is_mercator_fountain_open);

        Entity* platform_2 = new Entity({
                .type_id = ENTITY_SMALL_YELLOW_PLATFORM,
                .position = Position(21, 23, 0, false, false, false),
                .palette = 2
        });
        platform_2->remove_when_flag_is_set(is_mercator_fountain_open);

        world.map(MAP_460)->add_entity(platform_1);
        world.map(MAP_460)->add_entity(platform_2);
    }

    static void allow_going_backwards_in_knl_exterior(World& world)
    {
        world.map(MAP_KN_LABYRINTH_EXTERIOR)->remove_entity(1);
        world.map(MAP_KN_LABYRINTH_EXTERIOR)->remove_entity(0);
    }

    /**
     * By default, once boat is taken, there is no way to take it again to go back to Verla.
     * In randomizer, using Spell Book brings you back in your spawn area, potentially locking you out forever from Verla.
     * This function replaces the sailor blocking the pier by a new one which makes you take the boat again once boat
     * has already been taken once.
     */
    static void add_sailor_to_take_boat_again_on_docks(World& world)
    {
        world.map(MAP_MERCATOR_DOCKS_LIGHTHOUSE_FIXED_VARIANT)->add_entity(new Entity({
                .type_id = ENTITY_NPC_SAILOR,
                .position = Position(35, 44, 1, true),
                .orientation = ENTITY_ORIENTATION_NE,
                .palette = 3,
                .talkable = true,
                .dialogue = 4,
                .entity_to_use_tiles_from = world.map(MAP_MERCATOR_DOCKS_LIGHTHOUSE_FIXED_VARIANT)->entity(0),
                .mask_flags = { EntityMaskFlag(true, FLAG_TOOK_BOAT_TO_VERLA) }
        }));
    }

    static void fix_crypt_softlocks(md::ROM& rom, World& world)
    {
        // 1) Remove the check "if shadow mummy was beaten, raft mummy never appears again"
        // 0x019DF6:
        // Before:	0839 0006 00FF1014 (btst bit 6 in FF1014) ; 66 14 (bne $19E14)
        // After:	4EB9 00019E14 (jsr $19E14; 4E71 4E71 (nop nop)
        rom.set_code(0x19DF6, md::Code().nop(5));

        // 2) Change the shadow mummy appearance to go back from "if armlet is owned" to "chest was opened"
        // 0x0117E8:
        // Before:	103C 001F ; 4EB9 00022ED0 ; 4A41 ; 6B00 F75C (bmi $10F52)
        // After:	0839 0002 00FF1097 (btst 2 FF1097)	; 6700 F75C (bne $10F52)
        md::Code inject_change_crypt_exit_check;
        {
            inject_change_crypt_exit_check.btst(0x2, addr_(0xFF1097));
            inject_change_crypt_exit_check.nop(2);
        }
        rom.set_code(0x117E8, inject_change_crypt_exit_check);
        rom.set_byte(0x117F4, 0x67); // Turn the BMI into a BEQ
    }

    /**
     * Change the rafts logic so we can take them several times in a row, preventing from getting softlocked by missing chests
     */
    static void alter_labyrinth_rafts(md::ROM& rom)
    {
        // The trick here is to use flag 1001 (which resets on every map change) to correctly end the cutscene
        // while discarding the "raft already taken" state as early as the player moves to another map.
        rom.set_word(0x09E031, 0x0100);
        rom.set_word(0x09E034, 0x0100);
        rom.set_word(0x09E04E, 0x0100);
        rom.set_word(0x09E051, 0x0100);
    }
};
