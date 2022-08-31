#pragma once

#include <landstalker_lib/patches/game_patch.hpp>
#include <landstalker_lib/constants/offsets.hpp>
#include <landstalker_lib/constants/item_codes.hpp>

#include "../../logic_model/hint_source.hpp"
#include "../../logic_model/randomizer_world.hpp"

class PatchLithographHintOnUse : public GamePatch
{
public:
    void inject_code(md::ROM& rom, World& world) override
    {
        const RandomizerWorld& randomizer_world = reinterpret_cast<const RandomizerWorld&>(world);
        const std::vector<uint16_t>& text_ids = randomizer_world.hint_source("Lithograph")->text_ids();

        md::Code func_handle_lithograph;
        {
            for(uint16_t text_id : text_ids)
            {
                func_handle_lithograph.movew(text_id, reg_D0);
                func_handle_lithograph.jsr(0x22E90); // Display string in lower textbox
            }
            func_handle_lithograph.jmp(offsets::PROC_ITEM_USE_RETURN_SUCCESS);
        }
        uint32_t lithograph_use_addr = rom.inject_code(func_handle_lithograph);

        world.item(ITEM_LITHOGRAPH)->pre_use_address(lithograph_use_addr);
    }
};
