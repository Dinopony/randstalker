#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include <landstalker-lib/model/map.hpp>

/**
 * This patch removes all music inside the game, while leaving all of the SFX / jingles intact.
 * This was requested by players with *lots* of hours on the game who wanted to play with their own music in the
 * background. I'm so sorry, Takenouchi-san.
 */
class PatchRemoveMusic : public GamePatch
{
private:
    static constexpr uint8_t MUSIC_SILENT = 0x20;
    uint32_t _maps_using_tibor_music_addr = 0xFFFFFFFF;

public:
    void alter_rom(md::ROM& rom) override
    {
        // Empty the RoomMusicLUT
        for(uint32_t addr=0x2A32 ; addr < 0x2A44 ; ++addr)
            rom.set_byte(addr, MUSIC_SILENT);

        // Remove cutscene-triggered music
        rom.set_byte(0x9E59A, MUSIC_SILENT); // Duke Fanfare before last boss
        rom.set_byte(0x155EB, MUSIC_SILENT); // Last boss cutscene
        rom.set_byte(0x27721, MUSIC_SILENT); // Last boss cutscene
        rom.set_byte(0x15523, MUSIC_SILENT); // Last boss music
        rom.set_byte(0x9EBE3, MUSIC_SILENT); // Credits music

        // Remove boss musics
        rom.set_byte(0x9D6A1, MUSIC_SILENT);
        rom.set_byte(0x9D747, MUSIC_SILENT);
        rom.set_byte(0x9E195, MUSIC_SILENT);
        rom.set_byte(0x9E2C1, MUSIC_SILENT);
        rom.set_byte(0x9E57C, MUSIC_SILENT);
    }

    void inject_data(md::ROM& rom, World& world) override
    {
        // Since some conditions rely on current room music being Tibor's, we need to build a list of maps using this music
        // to be able to check it in another way
        ByteArray maps_using_tibor_music;
        for(auto& [map_id, map] : world.maps())
        {
            if(map->background_music() == 7)
                maps_using_tibor_music.add_word(map_id);
        }
        maps_using_tibor_music.add_word(0xFFFF);
        _maps_using_tibor_music_addr = rom.inject_bytes(maps_using_tibor_music);
    }

    void inject_code(md::ROM& rom, World& world) override
    {
        // Inject a function checking on the tree maps table to check if we should trigger
        // a "wave-y" teleportation effect or the regular one
        uint32_t func_check_tree_map_addr = inject_func_check_tree_map(rom);
        rom.set_code(0x6310, md::Code().jsr(func_check_tree_map_addr).nop());

        // This code was used to fix the check removing enemies once Tibor had been saved (which was also relying
        // on music). This is not useful anymore with the flexible mapsetup system which attaches the procedure
        // to the correct Tibor maps directly, not relying on music anymore.
        // rom.set_code(0x1A034, md::Code().jsr(func_check_tree_map_addr).nop());
    }

private:
    uint32_t inject_func_check_tree_map(md::ROM& rom) const
    {
        md::Code func_check_tree_map;
        func_check_tree_map.movem_to_stack({ reg_D0 }, { reg_A0 });
        {
            func_check_tree_map.movew(addr_(0xFF1206), reg_D0);
            func_check_tree_map.lea(_maps_using_tibor_music_addr, reg_A0);
            func_check_tree_map.label("loop_maps");
            {
                func_check_tree_map.cmpiw(0xFFFF, addr_(reg_A0));
                func_check_tree_map.bne("not_the_end");
                {
                    // Reached the end without finding the map, return false
                    func_check_tree_map.tstb(addr_(0xFF0000));
                    func_check_tree_map.bra("return");
                }
                func_check_tree_map.label("not_the_end");

                func_check_tree_map.cmpw(addr_(reg_A0), reg_D0);
                func_check_tree_map.bne("next_map");
                {
                    // Found the map, return true
                    func_check_tree_map.bra("return");
                }
                func_check_tree_map.label("next_map");
                func_check_tree_map.adda(2, reg_A0);
            }
            func_check_tree_map.bra("loop_maps");
            func_check_tree_map.label("return");
        }
        func_check_tree_map.movem_from_stack({ reg_D0 }, { reg_A0 });
        func_check_tree_map.rts();

        return rom.inject_code(func_check_tree_map);
    }
};

