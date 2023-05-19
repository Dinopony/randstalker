#include "io.hpp"

#include <landstalker-lib/tools/json.hpp>
#include "../logic_model/world_teleport_tree.hpp"
#include <landstalker-lib/model/entity_type.hpp>

#include "../logic_model/world_region.hpp"
#include "../logic_model/randomizer_world.hpp"
#include "../logic_model/hint_source.hpp"
#include "../randomizer_options.hpp"

Json WorldJsonWriter::build_world_json(const RandomizerWorld& world, const RandomizerOptions& options)
{
    Json json;

    json["spawnLocation"] = world.spawn_location()->id();
    json["darkRegion"] = world.dark_region()->name();

    // Export hints
    json["hints"] = Json::object();
    for(HintSource* hint_source : world.used_hint_sources())
        json["hints"][hint_source->description()] = hint_source->text();

    // Export item sources
    std::vector<ItemSource*> item_sources = world.item_sources();
    std::sort(item_sources.begin(), item_sources.end(), [](ItemSource* a, ItemSource* b) {
        return a->name() < b->name();
    });

    json["itemSources"] = Json::object();
    for(ItemSource* source : item_sources)
    {
        if(source->is_shop_item())
        {
            json["itemSources"][source->name()] = {
                    { "item", source->item()->name() },
                    { "price", reinterpret_cast<ItemSourceShop*>(source)->price() }
            };
        }
        else
        {
            json["itemSources"][source->name()] = source->item()->name();
        }
    }

    // Fahl enemies
    json["fahlEnemies"] = Json::array();
    for(EntityType* enemy : world.fahl_enemies())
        json["fahlEnemies"].emplace_back(enemy->name());

    // Export teleport tree pairs if shuffled
    if(options.shuffle_tibor_trees())
    {
        json["teleportTreePairs"] = Json::array();
        for(auto& pair : world.teleport_tree_pairs())
            json["teleportTreePairs"].emplace_back(Json::array({ pair.first->name(), pair.second->name() }));
    }

    return json;
}
