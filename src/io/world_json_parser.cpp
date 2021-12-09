#include "world_json_parser.hpp"

#include "../logic_model/world_node.hpp"
#include "../logic_model/world_region.hpp"
#include "../logic_model/hint_source.hpp"
#include <landstalker_lib/tools/tools.hpp>
#include <landstalker_lib/exceptions.hpp>

static Item* parse_item_from_name_in_json(const std::string& item_name, World& world)
{
    Item* item = world.item(item_name);
    if(item)
        return item;

    // If item is formatted as "X golds", parse X value and create the matching gold stack item
    if(item_name.ends_with("golds"))
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

static void parse_item_sources_from_json(World& world, const WorldLogic& logic, const Json& json)
{
    if(!json.count("itemSources"))
        return;

    const Json& item_sources_json = json.at("itemSources");
    for(auto&[region_id, item_sources_in_region_json] : item_sources_json.items())
    {
        WorldRegion *region = logic.region(region_id);
        std::map<std::string, ItemSource *> region_item_sources = region->item_sources();

        for(auto&[source_name, item_name] : item_sources_in_region_json.items())
        {
            ItemSource *source = region_item_sources.at(source_name);
            Item *item = parse_item_from_name_in_json(item_name, world);
            source->item(item);
        }
    }
}

static void parse_hints_from_json(WorldLogic& logic, const Json& json)
{
    if(!json.count("hints"))
        return;

    const Json& hints_json = json.at("hints");
    for(auto& [hint_source_name, contents] : hints_json.items())
    {
        HintSource* source = logic.hint_source(hint_source_name);

        if(contents.is_array())
        {
            std::vector<std::string> hint_lines;
            for(std::string line : contents)
                hint_lines.emplace_back(line);
            std::string hint = tools::join(hint_lines, "\n");
            source->text(hint);
        }
        else source->text(contents);
    }
}

static void parse_spawn_location_from_json(World& world, WorldLogic& logic, const Json& json)
{
    if(json.contains("spawnLocation"))
    {
        const std::string& spawn_location_name = json.at("spawnLocation");
        world.spawn_location(*logic.spawn_locations().at(spawn_location_name));
    }
}

static void parse_dark_region_from_json(World& world, WorldLogic& logic, const Json& json)
{
    if(json.contains("darkRegion"))
    {
        const std::string& dark_region_name = json.at("darkRegion");
        logic.dark_region(logic.region(dark_region_name), world);
    }
}

static void parse_fahl_enemies(World& world, const Json& json)
{
    if(json.contains("fahlEnemies"))
    {
        for(std::string enemy_name : json.at("fahlEnemies"))
        {
            EntityType* enemy = world.entity_type(enemy_name);
            if(!enemy)
                throw std::out_of_range("Unknown enemy type '" + enemy_name + "'");

            world.add_fahl_enemy(enemy);
        }
    }
}

void WorldJsonParser::parse_world_json(World &world, WorldLogic &logic, const Json& json)
{
    parse_item_sources_from_json(world, logic, json);
    parse_hints_from_json(logic, json);
    parse_spawn_location_from_json(world, logic, json);
    parse_dark_region_from_json(world, logic, json);
    parse_fahl_enemies(world, json);
}



/*
void World::parse_json(const Json& json)
{


    ////////// Miscellaneous ///////////////////////////////////////////


    // Parse dark node


    // Parse Fahl enemies

}


*/
