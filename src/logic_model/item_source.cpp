#include "item_source.hpp"
#include <landstalker-lib/model/entity.hpp>
#include <landstalker-lib/model/entity_type.hpp>
#include <landstalker-lib/model/map.hpp>
#include <landstalker-lib/model/world.hpp>
#include <landstalker-lib/exceptions.hpp>

ItemSource::ItemSource(const std::string& name, const std::string& node_id, const std::vector<std::string>& hints) :
    _name       (name),
    _item       (nullptr),
    _node_id    (node_id),
    _hints      (hints)
{}

Json ItemSource::to_json() const
{
    Json json;
    json["name"] = _name;
    json["type"] = type_name();
    json["nodeId"] = _node_id;
    if(!_hints.empty())
        json["hints"] = _hints;
    return json;
}

ItemSource* ItemSource::from_json(const Json& json, const World& world, bool resolve_map_entities)
{
    const std::string& name = json.at("name");
    const std::string& type = json.at("type");
    const std::string& node_id = json.at("nodeId");

    std::vector<std::string> hints;
    if(json.contains("hints")) 
        json.at("hints").get_to(hints);

    if(type == "chest")
    {
        uint8_t chest_id = json.at("chestId");
        return new ItemSourceChest(chest_id, name, node_id, hints);
    }
    else if(type == "ground" || type == "shop")
    {
        std::vector<Entity*> entities;

        if(resolve_map_entities)
        {
            std::vector<Json> entity_jsons;
            if(json.contains("entities"))
                json.at("entities").get_to(entity_jsons);
            else if(json.contains("entity"))
                entity_jsons = { json.at("entity") };
            else
                throw LandstalkerException("No entity information was provided for ground item source '" + name + "'");

            for(const Json& entity_json : entity_jsons)
            {
                uint16_t map_id = entity_json.at("mapId");
                uint8_t entity_id = entity_json.at("entityId");
                Entity* entity = world.map(map_id)->entity(entity_id);
                if(entity->entity_type_id() < 0xC0) // 0xC0 is the first ground item ID
                    throw LandstalkerException("EntityType " + std::to_string(entity_id) + " of map " + std::to_string(map_id) + " is not a ground item.");

                entities.emplace_back(entity);
            }
        }

        if(type == "shop")
        {
            uint8_t shop_item_id = json.at("shopItemId");
            return new ItemSourceShop(name, entities, shop_item_id, node_id, hints);
        }
        else
        {
            uint8_t ground_item_id = json.at("groundItemId");
            return new ItemSourceOnGround(name, entities, ground_item_id, node_id, hints);
        }
    }
    else if(type == "reward")
    {
        uint32_t address_in_rom = json.at("address");
        uint8_t reward_id = json.at("rewardId");
        return new ItemSourceReward(address_in_rom, name, reward_id, node_id, hints);
    }

    std::stringstream msg;
    msg << "Invalid item source type name '" << type << "' found in JSON.";
    throw JsonParsingException(msg.str());
}

////////////////////////////////////////////////////////////////

Json ItemSourceChest::to_json() const
{
    Json json = ItemSource::to_json();
    json["chestId"] = _chest_id;
    return json;
}

////////////////////////////////////////////////////////////////

Json ItemSourceOnGround::to_json() const
{
    Json json = ItemSource::to_json();
    
    if(_entities.size() > 1)
        json["entities"] = Json::array();

    for(Entity* entity : _entities)
    {
        Json entity_json = {
            { "mapId", entity->map()->id() },
            { "entityId", entity->entity_id() }
        };

        if(_entities.size() == 1)
            json["entity"] = entity_json;
        else
            json["entities"].emplace_back(entity_json);
    }

    return json;
}

////////////////////////////////////////////////////////////////

Json ItemSourceReward::to_json() const
{
    Json json = ItemSource::to_json();
    json["address"] = _address_in_rom;
    return json;
}
