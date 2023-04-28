#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include <landstalker-lib/constants/offsets.hpp>

#include "../../logic_model/randomizer_world.hpp"
#include "../../logic_model/item_source.hpp"

/**
 * This patch applies the content of the item sources inside their matching World element (or directly inside the
 * ROM if there is none)
 */
class PatchApplyItemSources : public GamePatch
{
public:
    void inject_data(md::ROM& rom, World& world) override
    {
        RandomizerWorld& randomizer_world = reinterpret_cast<RandomizerWorld&>(world);
        for(ItemSource* source : randomizer_world.item_sources())
        {
            if(source->type_name() == "chest")
            {
                uint8_t chest_id = reinterpret_cast<ItemSourceChest*>(source)->chest_id();
                world.chest_contents(chest_id, source->item());
            }
            else if(source->type_name() == "reward")
            {
                uint32_t address_in_rom = reinterpret_cast<ItemSourceReward*>(source)->address_in_rom();
                rom.set_byte(address_in_rom, source->item_id());
            }
            else
            {
                // Ground & shop item sources are tied to map entities that are updated as their contents change.
                // Therefore those types of item sources will effectively be written when map entities are written.
                ItemSourceOnGround* ground_source = reinterpret_cast<ItemSourceOnGround*>(source);
                for (Entity* entity : ground_source->entities())
                {
                    uint8_t item_id = ground_source->item_id();
                    uint8_t entity_type_id = (item_id == ITEM_ARCHIPELAGO) ? ENTITY_ARCHIPELAGO_ITEM : item_id+0xC0;
                    entity->entity_type_id(entity_type_id);
                }
            }
        }
    }
};
