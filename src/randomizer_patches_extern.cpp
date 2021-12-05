#include "randomizer_options.hpp"
#include "world_model/world.hpp"
#include "world_model/item.hpp"
#include "world_model/entity_type.hpp"
#include "constants/item_codes.hpp"
#include "exceptions.hpp"

void patch_items(World& world, const RandomizerOptions& options)
{
    // Patch model if user specified a model patch
    const Json& model_patch = options.items_model_patch();
    for(auto& [id_string, patch_json] : model_patch.items())
    {
        uint8_t id = std::stoi(id_string);
        if(world.items().count(id))
            world.item(id)->apply_json(patch_json);
        else
            world.add_item(Item::from_json(id, patch_json));
    }

    // Process custom starting quantities for items
    const std::map<std::string, uint8_t>& starting_items = options.starting_items();
    for(auto& [item_name, quantity] : starting_items)
    {
        Item* item = world.item(item_name);
        if(!item)
        {
            std::stringstream msg;
            msg << "Cannot set starting quantity of unknown item '" << item_name << "'";
            throw RandomizerException(msg.str());
        }

        item->starting_quantity(std::min<uint8_t>(quantity, 9));
    }

    // Alter a few things depending on settings
    if (options.use_armor_upgrades())
    {
        world.item(ITEM_STEEL_BREAST)->gold_value(250);
        world.item(ITEM_CHROME_BREAST)->gold_value(250);
        world.item(ITEM_SHELL_BREAST)->gold_value(250);
        world.item(ITEM_HYPER_BREAST)->gold_value(250);
    }

    if (options.consumable_record_book())
    {
        world.item(ITEM_RECORD_BOOK)->max_quantity(9);
        uint16_t currentPrice = world.item(ITEM_RECORD_BOOK)->gold_value();
        world.item(ITEM_RECORD_BOOK)->gold_value(currentPrice / 5);
    }

    if(options.jewel_count() > MAX_INDIVIDUAL_JEWELS)
    {
        Item* red_jewel = world.item(ITEM_RED_JEWEL);
        red_jewel->name("Kazalt Jewel");
        red_jewel->allowed_on_ground(false);
        red_jewel->max_quantity(options.jewel_count());
    }
}

void patch_entity_types(World& world, const RandomizerOptions& options)
{
    // Apply multiplicators from randomizer settings
    for (auto& [id, entity_type] : world.entity_types())
    {
        if(entity_type->type_name() != "enemy")
            continue;
        EntityEnemy* enemy_type = reinterpret_cast<EntityEnemy*>(entity_type);

        enemy_type->apply_damage_factor(options.enemies_damage_factor());
        enemy_type->apply_health_factor(options.enemies_health_factor());
        enemy_type->apply_armor_factor(options.enemies_armor_factor());
        enemy_type->apply_golds_factor(options.enemies_golds_factor());
        enemy_type->apply_drop_chance_factor(options.enemies_drop_chance_factor());
    }
}

void patch_game_strings(World& world, const RandomizerOptions& options)
{
    // Kazalt rejection message
    world.game_strings()[0x022] = std::string("Only the bearers of the ") 
        + std::to_string(options.jewel_count()) + " jewels\n are worthy of entering\n King Nole's domain...\x1E";
}