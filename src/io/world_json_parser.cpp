#include "io.hpp"

#include "../logic_model/world_node.hpp"
#include "../logic_model/world_region.hpp"
#include "../logic_model/randomizer_world.hpp"
#include "../logic_model/hint_source.hpp"
#include "../logic_model/world_teleport_tree.hpp"
#include <landstalker-lib/tools/stringtools.hpp>
#include <landstalker-lib/exceptions.hpp>

static Item* parse_item_from_name_in_json(const std::string& item_name, RandomizerWorld& world)
{
    Item* item = world.item(item_name);
    if(item)
        return item;

    // If item is formatted as "X golds", parse X value and create the matching gold stack item
    if(item_name.ends_with(" golds") || item_name.ends_with(" Golds"))
    {
        size_t space_index = item_name.find_first_of(' ');
        if(space_index != std::string::npos)
        {
            std::string number_part = item_name.substr(0, space_index);
            try {
                uint8_t gold_value = static_cast<uint8_t>(std::strtol(number_part.c_str(), nullptr, 10));
                return world.add_gold_item(gold_value);
            }
            catch(std::exception&) {}
        }
    }

    throw LandstalkerException("Item name '" + item_name + "' is invalid in plando JSON.");
}

static void parse_item_sources_from_json(RandomizerWorld& world, const Json& json)
{
    if(!json.count("itemSources"))
        return;

    std::map<std::string, ItemSource*> item_sources_table;
    for(ItemSource* source : world.item_sources())
        item_sources_table[source->name()] = source;

    const Json& item_sources_json = json.at("itemSources");
    for(auto&[source_name, item_data] : item_sources_json.items())
    {
        ItemSource* source;
        try {
            source = item_sources_table.at(source_name);
        } catch(std::out_of_range&) {
            throw LandstalkerException("Item source '" + source_name + "' could not be found.");
        }

        if(item_data.is_string())
        {
            // If we find a string, that's the shorthand syntax for an item name from our own world
            Item* item = parse_item_from_name_in_json(item_data, world);
            source->item(item);
            continue;
        }

        // Otherwise, it means it's either an item for another player in the case of an Archipelago world, or an item
        // with additionnal data (e.g. price)
        Item* item;
        if(item_data.contains("player"))
            item = world.add_archipelago_item(item_data["item"], item_data["player"], source->is_shop_item());
        else
            item = parse_item_from_name_in_json(item_data["item"], world);
        source->item(item);

        if(item_data.contains("price"))
        {
            if(!source->is_shop_item())
                throw LandstalkerException("Trying to put an item with a price inside an item source which is not a shop");

            uint16_t price = item_data.at("price");
            reinterpret_cast<ItemSourceShop*>(source)->price(price);
        }
    }
}

static void parse_hints_from_json(RandomizerWorld& world, const Json& json)
{
    if(!json.count("hints"))
        return;

    const Json& hints_json = json.at("hints");
    for(auto& [hint_source_name, contents] : hints_json.items())
    {
        try {
            HintSource* source = world.hint_source(hint_source_name);

            if(contents.is_array())
            {
                std::vector<std::string> hint_lines;
                for(std::string line : contents)
                    hint_lines.emplace_back(line);
                std::string hint = stringtools::join(hint_lines, "\n");
                source->text(hint);
            }
            else source->text(contents);

            world.add_used_hint_source(source);
        }
        catch(std::out_of_range&) {
            throw LandstalkerException("Hint source '" + hint_source_name + "' could not be found.");
        }
    }
}

static void parse_spawn_location_from_json(RandomizerWorld& world, const Json& json)
{
    if(json.contains("spawnLocation"))
    {
        const std::string& spawn_location_name = json.at("spawnLocation");
        try {
            SpawnLocation* spawn = world.available_spawn_locations().at(spawn_location_name);
            world.spawn_location(spawn);
        }
        catch(std::out_of_range&) {
            throw LandstalkerException("Spawn location '" + spawn_location_name + "' could not be found");
        }
    }
}

static void parse_dark_region_from_json(RandomizerWorld& world, const Json& json)
{
    if(json.contains("darkRegion"))
    {
        const std::string& dark_region_name = json.at("darkRegion");

        WorldRegion* region = world.region(dark_region_name);
        if(!region)
            throw LandstalkerException("Dark region '" + dark_region_name + "' could not be found");
        world.dark_region(region);
    }
}

static void parse_fahl_enemies(RandomizerWorld& world, const Json& json)
{
    if(!json.contains("fahlEnemies"))
        return;

    for(std::string enemy_name : json.at("fahlEnemies"))
    {
        EntityType* enemy = world.entity_type(enemy_name);
        if(!enemy)
            throw LandstalkerException("Enemy type '" + enemy_name + "' could not be found");

        world.add_fahl_enemy(enemy);
    }
}

static void parse_teleport_trees(RandomizerWorld& world, const Json& json)
{
    if(!json.contains("teleportTreePairs"))
        return;

    std::vector<std::pair<WorldTeleportTree*, WorldTeleportTree*>> pairs = world.teleport_tree_pairs();

    std::map<std::string, WorldTeleportTree*> tree_dictionary;
    for(const auto& pair : pairs)
    {
        tree_dictionary[pair.first->name()] = pair.first;
        tree_dictionary[pair.second->name()] = pair.second;
    }
    pairs.clear();

    for(Json pair : json.at("teleportTreePairs"))
    {
        WorldTeleportTree* tree_1 = tree_dictionary.at(pair[0]);
        WorldTeleportTree* tree_2 = tree_dictionary.at(pair[1]);
        if(tree_1 == tree_2)
            return;

        tree_dictionary.erase(pair[0]);
        tree_dictionary.erase(pair[1]);
        
        pairs.emplace_back(std::make_pair(tree_1, tree_2));
    }

    // Add the remaining unpaired trees as "self-connected pairs"
    for(auto& [_, tree] : tree_dictionary)
        pairs.emplace_back(std::make_pair(tree, tree));

    world.teleport_tree_pairs(pairs);
}

void WorldJsonParser::parse_world_json(RandomizerWorld& world, const Json& json)
{
    parse_item_sources_from_json(world, json);
    parse_hints_from_json(world, json);
    parse_spawn_location_from_json(world, json);
    parse_dark_region_from_json(world, json);
    parse_fahl_enemies(world, json);
    parse_teleport_trees(world, json);
}
