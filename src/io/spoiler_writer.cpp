#include "io.hpp"

#include <landstalker_lib/tools/json.hpp>
#include "../logic_model/world_teleport_tree.hpp"
#include <landstalker_lib/model/entity_type.hpp>

#include "../logic_model/world_region.hpp"
#include "../logic_model/randomizer_world.hpp"
#include "../logic_model/hint_source.hpp"
#include "../randomizer_options.hpp"

Json SpoilerWriter::build_spoiler_json(const RandomizerWorld& world, const RandomizerOptions& options)
{
    Json json;

    // Export dark node
    json["spawnLocation"] = world.spawn_location()->id();
    json["darkRegion"] = world.dark_region()->name();

    // Export hints
    for(HintSource* hint_source : world.used_hint_sources())
        json["hints"][hint_source->description()] = hint_source->text();

    if(options.shuffle_tibor_trees())
    {
        json["tiborTrees"] = Json::array();
        for(auto& pair : world.teleport_tree_pairs())
        {
            const std::string& node_1_name = pair.first->name();
            const std::string& node_2_name = pair.second->name();
            json["tiborTrees"].emplace_back(node_1_name + " <--> " + node_2_name);
        }
    }

    // Export item sources
    for(WorldRegion* region : world.regions())
    {
        for(WorldNode* node : region->nodes())
        {
            for(ItemSource* source : node->item_sources())
            {
                Item* item = world.item(source->item_id());
                json["itemSources"][region->name()][source->name()] = item->name();
            }
        }
    }

    // Fahl enemies
    json["fahlEnemies"] = Json::array();
    for(EntityType* enemy : world.fahl_enemies())
        json["fahlEnemies"].emplace_back(enemy->name());

    return json;
}
