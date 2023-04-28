#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include <landstalker-lib/constants/offsets.hpp>

/**
 * This patch makes the key not consumed on use, making it a unique item that needs to be used to open several doors.
 */
class PatchCredits : public GamePatch
{
private:
    uint16_t _waiting_time_after_end = 180;
    uint32_t _blue_credits_palette_addr = 0xFFFFFFFF;
    uint32_t _red_credits_palette_addr = 0xFFFFFFFF;
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
    }

    void inject_code(md::ROM& rom, World& world) override
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
            func.btst(1, addr_(0xFF104C));
            func.beq("ret");
            {
                func.lea(_blue_credits_palette_addr, reg_A0); // Use blue credits palette if blue ribbon is held
            }
            func.label("ret");
        }
        func.rts();
        uint32_t func_addr = rom.inject_code(func);

        rom.set_code(0x9EC5A, md::Code().jsr(func_addr));
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
                // "frontend by"
                0x04, 0xFF, 0x81,
                0x21, 0x2D, 0x2A, 0x29, 0x2F, 0x20, 0x29, 0x1F, 0x80, 0x1D, 0x34, 0x00,
                // "HAWKREX"
                0x03, 0xFF,
                0x09, 0x02, 0x18, 0x0C, 0x13, 0x06, 0x19, 0x00,
                // "DILANDAU"
                0x08, 0xFF,
                0x05, 0x0A, 0x0D, 0x02, 0x0F, 0x05, 0x02, 0x16, 0x00,
                // "testing"
                0x04, 0xFF, 0x81,
                0x2F, 0x20, 0x2E, 0x2F, 0x24, 0x29, 0x22, 0x00,
                // "SAGAZ"
                0x03, 0xFF,
                0x14, 0x02, 0x08, 0x02, 0x1B, 0x00,
                // "STIK"
                0x03, 0xFF,
                0x14, 0x15, 0x0A, 0x0C, 0x00,
                // "LANDRYLE"
                0x03, 0xFF,
                0x0D, 0x02, 0x0F, 0x05, 0x13, 0x1A, 0x0D, 0x06, 0x00,
                // "SORCIER"
                0x08, 0xFF,
                0x14, 0x10, 0x13, 0x04, 0x0A, 0x06, 0x13, 0x00
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
};
