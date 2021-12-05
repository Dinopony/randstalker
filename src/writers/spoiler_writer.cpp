#include "writers.hpp"

#include <json.hpp>

#include "../world_model/world.hpp"
#include "../world_randomizer.hpp"
#include "../logic_model/world_logic.hpp"
#include "../randomizer_options.hpp"

#include "../world_model/spawn_location.hpp"
#include "../world_model/world_teleport_tree.hpp"
#include "../world_model/entity_type.hpp"

#include "../logic_model/world_region.hpp"
#include "../logic_model/hint_source.hpp"

Json SpoilerWriter::build_spoiler_json(const World& world, const WorldRandomizer& randomizer, const WorldLogic& logic, const RandomizerOptions& options)
{
    Json json;

    // Export dark node
    json["spawnLocation"] = world.spawn_location().id();
    json["darkRegion"] = randomizer.dark_region()->name();

    // Export hints
    for(auto& [description, source] : randomizer.hint_sources())
        json["hints"][description] = source->text();

    if(options.shuffle_tibor_trees())
    {
        json["tiborTrees"] = Json::array();
        for(auto& pair : world.teleport_tree_pairs())
            json["tiborTrees"].push_back(pair.first->name() + " <--> " + pair.second->name());
    }

    // Export item sources
    for(WorldRegion* region : logic.regions())
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
        json["fahlEnemies"].push_back(enemy->name());

    return json;
}
