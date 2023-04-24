#pragma once

#include "landstalker_lib/patches/game_patch.hpp"

#include "landstalker_lib/model/world.hpp"
#include "landstalker_lib/model/item.hpp"
#include "landstalker_lib/constants/item_codes.hpp"

#include "../../assets/archipelago_item.bin.hxx"
#include "landstalker_lib/tools/game_text.hpp"

// TODO: To handle shops, we need:
//      - Explicit item naming, for the player to be able to choose if they buy the item or not
//      - Per-source pricing, because AP items cannot have a unique price

/**
 * This patch handles most things specifically related to Archipelago multiworld.
 */
class PatchHandleArchipelago : public GamePatch
{
private:
    static constexpr uint32_t ADDR_RECEIVED_ITEM = 0xFF0020;
    static constexpr uint32_t ADDR_SEED = 0xFF0022;
    static constexpr uint32_t ADDR_CURRENT_RECEIVED_ITEM_INDEX = 0xFF107E;

    uint32_t _seed;

public:
    explicit PatchHandleArchipelago(const RandomizerOptions& options) :
        _seed(options.seed())
    {}

    void inject_code(md::ROM& rom, World& world) override
    {
        // Right before SEGA logo display, call the specific initialization function
        uint32_t func_boot_init_addr = this->inject_func_boot_init(rom);
        rom.set_long(0x38616, func_boot_init_addr);

        // Change the message when obtaining an Archipelago item in chest
        add_proc_handle_archipelago_items_in_chests(rom);

        // Add a function which initializes Archipelago ground item entities in a specific way to make them use
        // the correct sprite and have the right behavior
        add_proc_init_archipelago_ground_items(rom);

        // Extend the function that read item names to add a special case for ITEM_ARCHIPELAGO
        extend_load_item_name_function(rom);

        // Remove the check that prevents from getting the item and displays a "bag is full" message because
        // the game evaluates we are always full of AP items (since it's not a valid item)
        remove_bag_full_check_for_ground_ap_items(rom);
    }

    void inject_data(md::ROM& rom, World& world) override
    {
        // Add a sprite for the Archipelago item entity type
        SubSpriteMetadata subsprite(0x77FB);
        Sprite ap_item_sprite(ARCHIPELAGO_ITEM_SPRITE, ARCHIPELAGO_ITEM_SPRITE_SIZE, { subsprite });
        uint32_t ap_item_sprite_addr = rom.inject_bytes(ap_item_sprite.encode());

        // Attach this new sprite as the last frame of the last "animation" (= sprite bank) for items
        ByteArray frame_pointers(rom.get_bytes(0x121658, 0x121674));
        frame_pointers.add_long(ap_item_sprite_addr);
        uint32_t new_gfx_035_anim_07 = rom.inject_bytes(frame_pointers);
        rom.set_long(0x12059C, new_gfx_035_anim_07);
    }

    void alter_world(World& world) override
    {
        // Make the formerly unused AP item entity type use the same palette as other item entities
        EntityType* entity_type_ekeeke = world.entity_type(0xC0);
        EntityType* entity_type_ap_item = world.entity_type(ENTITY_ARCHIPELAGO_ITEM);
        entity_type_ap_item->low_palette(entity_type_ekeeke->low_palette());
        entity_type_ap_item->high_palette(entity_type_ekeeke->high_palette());
    }

private:
    uint32_t inject_func_boot_init(md::ROM& rom) const
    {
        md::Code func;
        {
            func.moveb(0xFF, addr_(ADDR_RECEIVED_ITEM));
            func.movel(_seed, addr_(ADDR_SEED));
            func.movew(0x0000, addr_(ADDR_CURRENT_RECEIVED_ITEM_INDEX));
        }
        func.jsr(0x230); // call j_DisableDisplayAndInts whose call was replaced by this function
        func.rts();
        return rom.inject_code(func);
    }

    static void add_proc_handle_archipelago_items_in_chests(md::ROM& rom)
    {
        md::Code proc;
        {
            proc.cmpib(ITEM_LIFESTOCK, reg_D0);
            proc.bne("not_lifestock");
            {
                proc.jmp(0x7132);
            }
            proc.label("not_lifestock");
            proc.cmpib(ITEM_ARCHIPELAGO, reg_D0);
            proc.bne("not_archipelago");
            {
                proc.movem_to_stack({ reg_D0, reg_D1 }, {});
                proc.jsr(0x9B00C);                  // Open chest
                proc.trap(0x00, { 0x00, 0x05 });    // Play chest jingle
                proc.movew(0x001E, reg_D0);         // "Got an Archipelago Item"
                proc.jsr(0x22E90);                  // j_PrintString
                proc.jsr(0x852);                    // RestoreBGM
                proc.movem_from_stack({ reg_D0, reg_D1 }, {});
                proc.jmp(0x7190);
            }
            proc.label("not_archipelago");
        }
        proc.jmp(0x70DC);

        uint32_t addr = rom.inject_code(proc);
        rom.set_code(0x70D6, md::Code().jmp(addr));
    }

    static void add_proc_init_archipelago_ground_items(md::ROM& rom)
    {
        md::Code func;
        {
            func.cmpib(0xC0, reg_D0);
            func.bcc("regular_item_entity");
            {
                func.cmpib(ENTITY_ARCHIPELAGO_ITEM, reg_D0);
                func.bne("not_an_item");
                {
                    func.moveb(0x23, addr_(reg_A1, 0x0B));
                    func.moveb(ITEM_ARCHIPELAGO, addr_(reg_A1, 0x36));  // Contents value

                    // Apply custom behavior, or 0x0002 if none was specified
                    func.clrw(reg_D2);
                    func.moveb(addr_(reg_A2, 0xFFFE), reg_D2);
                    func.andib(0x03, reg_D2);
                    func.lslw(0x08, reg_D2);
                    func.moveb(addr_(reg_A2, 0x01), reg_D2);
                    func.bne("not_zero");
                    {
                        func.movew(0x0002, reg_D2);
                    }
                    func.label("not_zero");
                    func.movew(reg_D2, addr_(reg_A1, 0x34));

                    func.moveb(0x1C, addr_(reg_A1, 0x25));              // Sprite (animation)
                    func.moveb(0x1C, addr_(reg_A1, 0x27));              // Sprite (frame)
                    func.jmp(0x196FA);
                }
                func.label("not_an_item");
                func.jmp(0x1972A);
            }
            func.label("regular_item_entity");
            func.jmp(0x19638);
        }

        uint32_t func_addr = rom.inject_code(func);
        rom.set_code(0x19630, md::Code().jmp(func_addr));
    }

    static void extend_load_item_name_function(md::ROM& rom)
    {
        const std::string ARCHIPELAGO_ITEM_NAME = "Archipelago Item";
        uint32_t string_addr = rom.inject_bytes(Symbols::bytes_for_symbols(ARCHIPELAGO_ITEM_NAME));

        md::Code proc;
        {
            proc.cmpiw(ITEM_ARCHIPELAGO, reg_D1);
            proc.bne("not_archipelago");
            {
                proc.movew(ARCHIPELAGO_ITEM_NAME.size()-1, reg_D7);
                proc.movel(string_addr, reg_A2);
                proc.jmp(0x294B2);
            }
            proc.label("not_archipelago");
            proc.cmpiw(0x0040, reg_D1);
            proc.bcc("not_item_name");
            {
                proc.jmp(0x294A2);
            }
            proc.label("not_item_name");
            proc.jmp(0x294A8);
        }

        uint32_t proc_addr = rom.inject_code(proc);
        rom.set_code(0x2949C, md::Code().jmp(proc_addr));
    }

    static void remove_bag_full_check_for_ground_ap_items(md::ROM& rom)
    {
        md::Code proc;
        {
            proc.cmpib(ITEM_ARCHIPELAGO, reg_D0);
            proc.bne("not_archipelago");
            {
                // Item is Archipelago-typed, jump after the "bag full" checks to ensure item is taken
                proc.movew(ITEM_NO_SWORD, reg_D2);
                proc.jmp(0x24B16);
            }
            proc.label("not_archipelago");
            // Item is regular, perform the checks as usual
            proc.jsr(0x29232);
            proc.bne("jmp_to_24AF4");
            proc.jmp(0x24AEA);
            proc.label("jmp_to_24AF4");
            proc.jmp(0x24AF4);
        }

        uint32_t addr = rom.inject_code(proc);
        rom.set_code(0x24AE4, md::Code().jmp(addr).nop());
    }
};