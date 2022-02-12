#include <landstalker_lib/md_tools.hpp>
#include <landstalker_lib/constants/offsets.hpp>
#include <landstalker_lib/constants/item_codes.hpp>
#include <landstalker_lib/model/item.hpp>
#include "../logic_model/randomizer_world.hpp"
#include "../logic_model/hint_source.hpp"

void handle_lithograph_hint(md::ROM& rom, RandomizerWorld& world)
{
    const std::vector<uint16_t>& text_ids = world.hint_source("Lithograph")->text_ids();

    md::Code func_handle_lithograph;
    for(uint16_t text_id : text_ids)
    {
        func_handle_lithograph.movew(text_id, reg_D0);
        func_handle_lithograph.jsr(0x22E90); // Display string in lower textbox
    }
    func_handle_lithograph.jmp(offsets::PROC_ITEM_USE_RETURN_SUCCESS);

    uint32_t lithograph_use_addr = rom.inject_code(func_handle_lithograph);
    world.item(ITEM_LITHOGRAPH)->pre_use_address(lithograph_use_addr);
}

void handle_oracle_stone_hint(md::ROM& rom, RandomizerWorld& world)
{
    const std::vector<uint16_t>& text_ids = world.hint_source("Oracle Stone")->text_ids();

    md::Code func_handle_oracle_stone;
    for(uint16_t text_id : text_ids)
    {
        func_handle_oracle_stone.movew(text_id, reg_D0);
        func_handle_oracle_stone.jsr(0x22E90); // Display string in lower textbox
    }
    func_handle_oracle_stone.jmp(offsets::PROC_ITEM_USE_RETURN_SUCCESS);

    uint32_t oracle_stone_use_addr = rom.inject_code(func_handle_oracle_stone);
    world.item(ITEM_ORACLE_STONE)->pre_use_address(oracle_stone_use_addr);

    // Neutralize regular use of Oracle Stone
    rom.set_code(0xDB92, md::Code().nop(2));
    rom.mark_empty_chunk(0xDBC4, 0xDC10);
}
