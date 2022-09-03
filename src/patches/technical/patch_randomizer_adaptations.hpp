#pragma once

#include <landstalker_lib/patches/game_patch.hpp>
#include <landstalker_lib/constants/map_codes.hpp>
#include <landstalker_lib/constants/flags.hpp>
#include <landstalker_lib/model/map.hpp>

class PatchRandomizerAdaptations : public GamePatch
{
public:
    void alter_rom(md::ROM& rom) override
    {
        set_story_as_advanced(rom);
        alter_lifestock_handling_in_shops(rom);
        fix_mir_tower_priest_room_items(rom);
        fix_knl_priest_room_infinite_chest(rom);
    }

    void alter_world(World& world) override
    {
        alter_king_nole_cave_teleporter_to_mercator_condition(world);
    }

    void inject_code(md::ROM& rom, World& world) override
    {
        prevent_hint_item_save_scumming(rom);
        fix_castle_falling_cutscene_with_uncompressed_sprites(rom, world);
    }

private:
    /**
     * Set the story progress at least as "Duke fled to Kazalt" state, whatever the current progress is
     */
    static void set_story_as_advanced(md::ROM& rom)
    {
        rom.set_word(0x25324, 0x0000);
    }

    static void alter_lifestock_handling_in_shops(md::ROM& rom)
    {
        // Make Lifestock prices the same over all shops
        for (uint32_t addr = 0x024D34; addr <= 0x024EAE; addr += 0xE)
            rom.set_byte(addr + 0x03, 0x10);

        // Remove the usage of "bought lifestock in shop X" flags
        for (uint32_t addr = 0x009D18; addr <= 0x009D33; addr += 0xE)
            rom.set_byte(addr, 0xFF);
    }

    /**
     * Change the flag checked for teleporter appearance from "saw the duke Kazalt cutscene" to "has visited four white golems room in King Nole's Cave"
     */
    static void alter_king_nole_cave_teleporter_to_mercator_condition(World& world)
    {
        world.map(MAP_KN_CAVE_ROOM_WITH_WHITE_GOLEMS_VARIANT)->visited_flag(FLAG_OPENED_MERCATOR_TELEPORTER_TO_KNC);
    }

    /**
     * Remove the "shop/church" flag on the priest room of Mir Tower to make its items on ground work everytime
     */
    static void fix_mir_tower_priest_room_items(md::ROM& rom)
    {
        // 0x024E5A:
        // Before:	0307
        // After:	7F7F
        rom.set_word(0x024E5A, 0x7F7F);
    }

    /**
     * In the priest room of King Nole's Labyrinth, there is a chest in the save room that has a unique behavior.
     * It is "infinite" in vanilla game and can be open as many times as we want, to get as many EkeEke as we need
     * to complete the game. This function removes this behavior by turning it into a regular chest.
     */
    static void fix_knl_priest_room_infinite_chest(md::ROM& rom)
    {
        rom.set_code(0x9EB9C, md::Code().nop(5));
    }

    /**
     * When hint items (Lithograph / Oracle Stone) were first introduced in shops, some clever lads had the idea of:
     * - saving the game
     * - buying the hint item to read the hint
     * - load back to get their money back while still remembering the hint
     *
     * This patch makes the game automatically save whenever an item is bought, preventing any form of save scumming.
     */
    static void prevent_hint_item_save_scumming(md::ROM& rom)
    {
        // TODO: Only save if Lithograph / Oracle Stone was bought
        md::Code func_save_on_buy;
        {
            // Redo instructions that were removed by injection
            func_save_on_buy.movew(reg_D2, reg_D0);
            func_save_on_buy.jsr(0x291D6); // GetItem
            // Save game
            func_save_on_buy.jsr(0x1592);  // SaveGame
        }
        func_save_on_buy.rts();
        uint32_t func_save_on_buy_addr = rom.inject_code(func_save_on_buy);

        rom.set_code(0x24F3E, md::Code().jsr(func_save_on_buy_addr));
    }

    /**
     * There is a problem in Mercator castle court with the falling item.
     * When the falling item is an item using an uncompressed sprite, sprite is reloaded and turns into an EkeEke for some
     * reason mid-animation. This patch adds a mapsetup for this map setting a byte to force the "compressed sprite behavior".
     */
    static void fix_castle_falling_cutscene_with_uncompressed_sprites(md::ROM& rom, World& world)
    {
        // Replace the casino mapsetup easter egg where you can turn chickens into enemies
        md::Code func;
        {
            func.bset(0, addr_(0xFF550C));
        }
        func.rts();

        uint32_t func_addr = rom.inject_code(func);
        world.map(MAP_MERCATOR_CASTLE_EXTERIOR_LEFT_COURT)->map_setup_addr(func_addr);
    }
};
