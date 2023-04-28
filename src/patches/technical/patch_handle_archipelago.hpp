#pragma once

#include <landstalker-lib/patches/game_patch.hpp>

#include <landstalker-lib/model/world.hpp>
#include <landstalker-lib/model/item.hpp>
#include <landstalker-lib/constants/item_codes.hpp>
#include <landstalker-lib/tools/game_text.hpp>
#include "../../assets/archipelago_item.bin.hxx"

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
    static constexpr uint32_t ADDR_CURRENT_LOCATION_UUID = 0xFF0026;
    static constexpr uint32_t ADDR_CURRENT_RECEIVED_ITEM_INDEX = 0xFF107E;

    uint32_t _seed;
    uint32_t _remote_names_table_addr = 0xFFFFFFFF;

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

        // Change the message when obtaining an Archipelago item on the ground
        add_proc_handle_archipelago_items_on_ground(rom);
        // Change the message when obtaining an Archipelago item from an NPC
        handle_npc_set_uuid_on_reward(rom, reinterpret_cast<RandomizerWorld&>(world));
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

        // Inject remote item names to make them appear in textboxes
        _remote_names_table_addr = inject_remote_item_names(rom, reinterpret_cast<RandomizerWorld&>(world));
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

                // Update current chest UUID
                proc.clrw(reg_D7);
                proc.moveb(addr_(reg_A4, 0x37), reg_D7);  // Chest ID
                proc.addiw(0x100, reg_D7);                // + 0x100
                proc.movew(reg_D7, addr_(ADDR_CURRENT_LOCATION_UUID));

                proc.movew(0x001E, reg_D0);         // "Sent {ITEM}"
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

    static uint32_t inject_remote_item_names(md::ROM& rom, RandomizerWorld& world)
    {
        std::map<std::string, uint32_t> remote_item_names_dict;
        std::vector<uint32_t> source_id_to_name_table;
        constexpr uint16_t STARTING_ID = 0x100;

        for(ItemSource* source : world.item_sources())
        {
            Item* item = source->item();
            if(item->id() != ITEM_ARCHIPELAGO)
                continue;
            if(source->uuid() < STARTING_ID)
                continue;

            // If that's the first time we encounter that item name, inject it and add it to the dictionary
            if(!remote_item_names_dict.count(item->name()))
            {
                ByteArray str_bytes(Symbols::bytes_for_symbols(item->name()));
                str_bytes.insert(str_bytes.begin(), (uint8_t)str_bytes.size()-1);
                remote_item_names_dict[item->name()] = rom.inject_bytes(str_bytes);
            }

            // Match the item name with the item source we are processing
            uint16_t table_index = source->uuid() - STARTING_ID;
            if(source_id_to_name_table.size() <= table_index)
                source_id_to_name_table.resize(table_index+1, 0xFFFFFFFF);
            source_id_to_name_table[table_index] = remote_item_names_dict[item->name()];
        }

        // Now that we have built a full matching table for all item names in sources containing remote items, inject it
        ByteArray addresses_table;
        for(uint32_t addr : source_id_to_name_table)
            addresses_table.add_long(addr);
        return rom.inject_bytes(addresses_table);
    }

    void extend_load_item_name_function(md::ROM& rom) const
    {
        const std::string ARCHIPELAGO_ITEM_NAME = "Archipelago Item";
        uint32_t string_addr = rom.inject_bytes(Symbols::bytes_for_symbols(ARCHIPELAGO_ITEM_NAME));

        md::Code proc;
        {
            proc.cmpiw(ITEM_ARCHIPELAGO, reg_D1);
            proc.bne("not_archipelago");
            {
               proc.clrl(reg_D7);
               proc.movew(addr_(ADDR_CURRENT_LOCATION_UUID), reg_D7);
               proc.cmpiw(0x100, reg_D7);
               proc.blt("generic_ap_item");
               {
                   // If UUID is greater or equal to 0x100, it's valid and we can use it
                   proc.subiw(0x100, reg_D7);
                   proc.lsll(2, reg_D7);
                   proc.lea(_remote_names_table_addr, reg_A2);
                   proc.movel(addr_(reg_A2, reg_D7), reg_A2);
                   proc.clrl(reg_D7);
                   proc.moveb(addr_(reg_A2), reg_D7);  // Put string size inside D7 (first byte)
                   proc.adda(1, reg_A2);               // Make A2 point on the first char of the string (second byte)
                   proc.bra("return_ap");
               }
               proc.label("generic_ap_item");
               {
                    // If UUID is invalid, put a generic "Archipelago Item" as a fallback
                    proc.movew(ARCHIPELAGO_ITEM_NAME.size() - 1, reg_D7);
                    proc.movel(string_addr, reg_A2);
                }
                proc.label("return_ap");
                proc.movew(0x0000, addr_(ADDR_CURRENT_LOCATION_UUID));
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

    static void add_proc_handle_archipelago_items_on_ground(md::ROM& rom)
    {
        md::Code proc;
        {
            proc.cmpib(ITEM_ARCHIPELAGO, reg_D0);
            proc.bne("not_archipelago");
            {
                proc.trap(0x00, { 0x00, 0x05 });  // Play "get item" jingle

                // Update current ground item UUID
                proc.clrw(reg_D7);
                proc.moveb(addr_(reg_A5, 0x3A), reg_D7);  // Ground ID stored as dialogue
                proc.lsrb(2, reg_D7);                     //   / 4

                // Set the ground check flag manually since we skip the "GetItem" function
                proc.movem_to_stack({ reg_D0, reg_D7 }, { reg_A0 });
                proc.movel(reg_D7, reg_D0);
                proc.andib(0x07, reg_D7);      // D7 contains flag bit
                proc.lsrb(3, reg_D0);          // D0 contains flag byte
                proc.lea(0xFF1060, reg_A0);
                proc.bset(reg_D7, addr_(reg_A0, reg_D0));
                proc.movem_from_stack({ reg_D0, reg_D7 }, { reg_A0 });

                proc.addiw(0x200, reg_D7);                // add 0x200 to ground_item_id to get UUID
                proc.movew(reg_D7, addr_(ADDR_CURRENT_LOCATION_UUID));

                proc.movew(0x1E, reg_D0); // "Sent {ITEM}"
                proc.jsr(0x28FD8);        // DisplayText
                proc.jmp(0x24B30);        // Return to normal flow not giving any item
            }
            proc.label("not_archipelago");
            // Item is regular, perform the checks as usual
            proc.jsr(0x29232); // GetRemainingItemAllowedCount
            proc.bne("jmp_to_24AF4");
            proc.jmp(0x24AEA);
            proc.label("jmp_to_24AF4");
            proc.jmp(0x24AF4);
        }

        uint32_t addr = rom.inject_code(proc);
        rom.set_code(0x24AE4, md::Code().jmp(addr).nop());
    }

    static uint32_t inject_npc_addr_to_uuid_table(md::ROM& rom, RandomizerWorld& world)
    {
        std::vector<uint32_t> npc_reward_addresses;
        for(ItemSource* source : world.item_sources())
        {
            if(!source->is_npc_reward())
                continue;
            ItemSourceReward* reward_source = reinterpret_cast<ItemSourceReward*>(source);

            uint8_t reward_id = reward_source->reward_id();
            if(npc_reward_addresses.size() <= reward_id)
                npc_reward_addresses.resize(reward_id+1, 0xFFFFFFFF);
            npc_reward_addresses[reward_id] = reward_source->address_in_rom() + 1;
        }

        ByteArray bytes;
        for(uint32_t addr : npc_reward_addresses)
            bytes.add_long(addr);
        bytes.add_long(0xFFFFFFFF);
        return rom.inject_bytes(bytes);
    }

    void handle_npc_set_uuid_on_reward(md::ROM& rom, RandomizerWorld& world)
    {
        // Inject a lookup table to determine the UUID of a NPC reward based on its address
        // This is injected at "inject_code" time since it needs for RemoveStoryDependency patch to execute its
        // own inject_code routine to have the correct address for the vanilla Red Jewel item source.
        uint32_t npc_addr_to_uuid_table_addr = inject_npc_addr_to_uuid_table(rom, world);

        md::Code proc;
        {
            proc.clrl(reg_D7);
            proc.lea(npc_addr_to_uuid_table_addr, reg_A1);
            proc.label("loop");
            {
                proc.cmpil(0xFFFFFFFF, addr_(reg_A1, reg_D7));
                proc.beq("return"); // We reached the end of the table, this reward is not handled, just skip it
                proc.cmpa(addr_(reg_A1, reg_D7), reg_A0);
                proc.bne("not_this_reward");
                {
                    proc.lsrw(2, reg_D7);
                    proc.addiw(ItemSourceReward::base_reward_uuid(), reg_D7);
                    proc.movew(reg_D7, addr_(ADDR_CURRENT_LOCATION_UUID));
                    proc.bra("return");
                }
                proc.label("not_this_reward");
                proc.addqb(4, reg_D7);
                proc.bra("loop");
            }
        }
        proc.label("return");
        proc.movem_from_stack({ reg_D0 }, { reg_A1 });
        proc.rts();

        uint32_t proc_addr = rom.inject_code(proc);
        rom.set_code(0x28DDE, md::Code().jmp(proc_addr));
    }
};
