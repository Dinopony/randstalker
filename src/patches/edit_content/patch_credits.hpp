#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include <landstalker-lib/constants/offsets.hpp>
#include "../../constants/rando_constants.hpp"

/**
 * This patch makes the key not consumed on use, making it a unique item that needs to be used to open several doors.
 */
class PatchCredits : public GamePatch
{
private:
    uint16_t _waiting_time_after_end = 180;
    uint32_t _blue_credits_palette_addr = 0xFFFFFFFF;
    uint32_t _red_credits_palette_addr = 0xFFFFFFFF;
    uint32_t _gold_credits_palette_addr = 0xFFFFFFFF;
    uint32_t _all_checks_bytes_addr = 0xFFFFFFFF;
    std::vector<uint8_t> _original_credits;

public:
    /**
     * @param waiting_time_after_end the time to wait after "The End" (in seconds) to be brought back to title screen
     *                               (= 3 minutes in base game)
     */
    explicit PatchCredits(uint16_t waiting_time_after_end) : _waiting_time_after_end(waiting_time_after_end) {}

    void load_from_rom(const md::ROM& rom) override
    {
        _original_credits = rom.get_bytes(offsets::CREDITS_TEXT, offsets::CREDITS_TEXT_END);
    }

    void alter_rom(md::ROM& rom) override
    {
        // Change credits contents
        rom.set_bytes(offsets::CREDITS_TEXT, build_credits_bytes());

        // Change the waiting time after "The End" to come back to title screen
        uint32_t waiting_time_in_frames = _waiting_time_after_end * 60;
        if(waiting_time_in_frames > 0xFFFF)
            waiting_time_in_frames = 0xFFFF;
        rom.set_word(0x9ECC4, waiting_time_in_frames);

        // Credits debug code, uncomment to launch credits when opening inventory menu
        // rom.set_code(0x7654, md::Code().jsr(0xEAEA).rts());
    }

    void inject_data(md::ROM& rom, World& world) override
    {
        ByteArray blue_credits_palette;
        blue_credits_palette.add_word(0x000);
        blue_credits_palette.add_word(0xE66);
        blue_credits_palette.add_word(0xC44);
        blue_credits_palette.add_word(0xA22);
        _blue_credits_palette_addr = rom.inject_bytes(blue_credits_palette);

        ByteArray red_credits_palette;
        red_credits_palette.add_word(0x000);
        red_credits_palette.add_word(0x66E);
        red_credits_palette.add_word(0x44C);
        red_credits_palette.add_word(0x22A);
        _red_credits_palette_addr = rom.inject_bytes(red_credits_palette);

        ByteArray gold_credits_palette;
        gold_credits_palette.add_word(0x000);
        gold_credits_palette.add_word(0x6CE);
        gold_credits_palette.add_word(0x48C);
        gold_credits_palette.add_word(0x26A);
        _gold_credits_palette_addr = rom.inject_bytes(gold_credits_palette);

        std::vector<uint16_t> all_checks_signature = {
            0xFEFF, 0xFF3F, 0xFEFF, 0xFFFF, 0xFFFF, 0x0300, 0x0000, 0x0000,
            0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
            0xFFFF, 0x7FB3, 0x5EFE, 0xF71F, 0xFCFF, 0xFFFF, 0xFFFF, 0xFFFF,
            0xFFFF, 0xFFFF, 0xFFFF, 0xFF87, 0xF7FF, 0xFF3F
        };
        ByteArray all_checks_bytes;
        for(uint16_t word : all_checks_signature)
            all_checks_bytes.add_word(word);
        _all_checks_bytes_addr = rom.inject_bytes(all_checks_bytes);
    }

    void inject_code(md::ROM& rom, World& world) override
    {
        uint32_t func_choose_ending_palette_addr = inject_func_choose_ending_palette(rom);
        rom.set_code(0x9EC5A, md::Code().jsr(func_choose_ending_palette_addr));

        // Set a flag when Blue Ribbon is used to trigger the blue ending
        md::Code func;
        func.bset(FLAG_USED_BLUE_RIBBON.bit, addr_(0xFF1000 + FLAG_USED_BLUE_RIBBON.byte));
        func.jmp(world.item(ITEM_BLUE_RIBBON)->pre_use_address());
        world.item(ITEM_BLUE_RIBBON)->pre_use_address(rom.inject_code(func));
    }

private:
    [[nodiscard]] std::vector<uint8_t> build_credits_bytes() const
    {
        std::vector<uint8_t> new_credits_text;

        new_credits_text.insert(new_credits_text.begin(), _original_credits.begin(), _original_credits.begin() + 0x75);

        // Change "LANDSTALKER" to "RANDSTALKER"
        new_credits_text[0x2] = 0x13;

        std::vector<uint8_t> landstalker_randomizer = {
                0x27, 0x1C, 0x29, 0x1F, 0x2E, 0x2F, 0x1C, 0x27, 0x26, 0x20, 0x2D, 0x80, // "landstalker "
                0x2D, 0x1C, 0x29, 0x1F, 0x2A, 0x28, 0x24, 0x35, 0x20, 0x2D              // "randomizer"
        };
        std::copy(landstalker_randomizer.begin(), landstalker_randomizer.end(), new_credits_text.begin() + 0x14);

        // Widen the space between the end of the cast and the beginning of the rando staff
        new_credits_text[0x5C] = 0x0F;

        // Replace the "game cast" (useless list of game characters) by the randomizer credits
        std::vector<uint8_t> rando_credits = {
                // " R A N D O M I Z E R "
                0x08, 0xFF, 0x82,
                0x80, 0x13, 0x80, 0x02, 0x80, 0x0F, 0x80, 0x05, 0x80, 0x10, 0x80, 0x0E, 0x80, 0x0A, 0x80, 0x1B, 0x80, 0x06, 0x80, 0x13, 0x80, 0x00,
                // "developed by"
                0x04, 0xFF, 0x81,
                0x1F, 0x20, 0x31, 0x20, 0x27, 0x2A, 0x2B, 0x20, 0x1F, 0x80, 0x1D, 0x34, 0x00,
                // "DINOPONY"
                0x03, 0xFF,
                0x05, 0x0A, 0x0F, 0x10, 0x11, 0x10, 0x0F, 0x1A, 0x00,
                // "WIZ"
                0x08, 0xFF,
                0x18, 0x0A, 0x1B, 0x00,
                // "testing"
                0x04, 0xFF, 0x81,
                0x2F, 0x20, 0x2E, 0x2F, 0x24, 0x29, 0x22, 0x00,
                // "SAGAZ"
                0x03, 0xFF,
                0x14, 0x02, 0x08, 0x02, 0x1B, 0x00,
                // "STIK"
                0x03, 0xFF,
                0x14, 0x15, 0x0A, 0x0C, 0x00,
                // "HAWKREX"
                0x03, 0xFF,
                0x09, 0x02, 0x18, 0x0C, 0x13, 0x06, 0x19, 0x00,
                // "LANDRYLE"
                0x03, 0xFF,
                0x0D, 0x02, 0x0F, 0x05, 0x13, 0x1A, 0x0D, 0x06, 0x00,
                // "SORCIER"
                0x08, 0xFF,
                0x14, 0x10, 0x13, 0x04, 0x0A, 0x06, 0x13, 0x00,
                // "disassembly"
                0x04, 0xFF, 0x81,
                0x1F, 0x24, 0x2E, 0x1C, 0x2E, 0x2E, 0x20, 0x28, 0x1D, 0x27, 0x34, 0x00,
                // "LORDMIR"
                0x08, 0xFF,
                0x0D, 0x10, 0x13, 0x05, 0x0E, 0x0A, 0x13, 0x00,
        };
        new_credits_text.insert(new_credits_text.end(), rando_credits.begin(), rando_credits.end());

        // Add the rest of the original game credits
        new_credits_text.insert(new_credits_text.end(), _original_credits.begin() + 0x1D3, _original_credits.begin() + 0x929);

        while(new_credits_text.size() < _original_credits.size())
            new_credits_text.emplace_back(0x00);
        if(new_credits_text.size() > _original_credits.size())
            throw LandstalkerException("New credits cannot be bigger than original credits");

        return new_credits_text;
    }

    uint32_t inject_func_choose_ending_palette(md::ROM& rom)
    {
        md::Code func;
        {
            func.lea(0xFF0080, reg_A1); // instruction erased by the jsr
            func.cmpib(2, addr_(0xFF001C));
            func.bne("not_arg");
            {
                func.lea(_red_credits_palette_addr, reg_A0); // Use red credits palette if Dark Gola has been beaten
                func.bra("ret");
            }
            func.label("not_arg");
            func.movem_to_stack({ reg_D0, reg_D1 }, { reg_A0, reg_A1 });
            func.lea(0xFF1060, reg_A0);
            func.lea(_all_checks_bytes_addr, reg_A1);
            func.label("checks_loop");
            {
                func.movew(addr_postinc_(reg_A0), reg_D0);  // A0/D0 = cursor on check flags
                func.movew(addr_postinc_(reg_A1), reg_D1);  // A1/D1 = cursor on target values
                func.andw(reg_D1, reg_D0);                  // Check if D0 contains at least all bits set in D1
                func.cmpw(reg_D1, reg_D0);
                func.bne("not_all_checks");                 // If at least one is not equal, don't give golden ending
                func.cmpa(0xFF109C, reg_A0);
                func.blt("checks_loop");
                func.movem_from_stack({ reg_D0, reg_D1 }, { reg_A0, reg_A1 });
                func.lea(_gold_credits_palette_addr, reg_A0); // Everything was collected, give the golden credits
                func.bra("ret");
            }

            func.label("not_all_checks");
            func.movem_from_stack({ reg_D0, reg_D1 }, { reg_A0, reg_A1 });
            func.btst(FLAG_USED_BLUE_RIBBON.bit, addr_(0xFF1000 + FLAG_USED_BLUE_RIBBON.byte));
            func.beq("ret");
            {
                func.lea(_blue_credits_palette_addr, reg_A0); // Use blue credits palette if blue ribbon is held
            }
            func.label("ret");
        }
        func.rts();
        return rom.inject_code(func);
    }
};
