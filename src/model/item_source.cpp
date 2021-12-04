#include "item_source.hpp"
#include "world_node.hpp"
#include "entity.hpp"
#include "entity_type.hpp"
#include "map.hpp"
#include "../exceptions.hpp"
#include "../world.hpp"


ItemSource::ItemSource(const std::string& name, WorldNode* node, const std::vector<std::string>& hints) :
    _name   (name),
    _item   (nullptr),
    _node   (node),
    _hints  (hints)
{
    if(_node)
        _node->add_item_source(this);
}

Json ItemSource::to_json() const
{
    Json json;
    json["name"] = _name;
    json["type"] = type_name();
    json["nodeId"] = _node->id();
    if(!_hints.empty())
        json["hints"] = _hints;
    return json;
}

ItemSource* ItemSource::from_json(const Json& json, const World& world)
{
    const std::string& name = json.at("name");
    const std::string& type = json.at("type");

    const std::string& nodeId = json.at("nodeId");
    WorldNode* node = world.nodes().at(nodeId);

    std::vector<std::string> hints;
    if(json.contains("hints")) 
        json.at("hints").get_to(hints);

    if(type == "chest")
    {
        uint8_t chest_id = json.at("chestId");
        return new ItemSourceChest(chest_id, name, node, hints);
    }
    else if(type == "ground" || type == "shop")
    {
        std::vector<Entity*> entities;

        std::vector<Json> entity_jsons;
        if(json.contains("entities"))
            json.at("entities").get_to(entity_jsons);
        else if(json.contains("entity"))
            entity_jsons = { json.at("entity") };
        else
            throw RandomizerException("No entity information was provided for ground item source '" + name + "'"); 

        for(const Json& entity_json : entity_jsons)
        {
            uint16_t map_id = entity_json.at("mapId");
            uint8_t entity_id = entity_json.at("entityId");
            Entity* entity = world.map(map_id)->entity(entity_id);
            if(entity->entity_type_id() < 0xC0) // 0xC0 is the first ground item ID
                throw RandomizerException("EntityType " + std::to_string(entity_id) + " of map " + std::to_string(map_id) + " is not a ground item.");
            
            entities.push_back(entity);
        }

        if(type == "shop")
            return new ItemSourceShop(name, entities, node, hints);

        bool cannot_be_taken_repeatedly = json.value("cannotBeTakenRepeatedly", false);
        return new ItemSourceOnGround(name, entities, node, hints, cannot_be_taken_repeatedly); 
    }
    else if(type == "reward")
    {
        uint32_t address_in_rom = json.at("address");
        return new ItemSourceReward(address_in_rom, name, node, hints);        
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
            json["entities"].push_back(entity_json);
    }

    if(_cannot_be_taken_repeatedly)
        json["cannotBeTakenRepeatedly"] = _cannot_be_taken_repeatedly;

    return json;
}

////////////////////////////////////////////////////////////////

bool ItemSourceShop::is_item_compatible(Item* item) const
{
    if(!ItemSourceOnGround::is_item_compatible(item))
        return false;

    if (item->id() == ITEM_NONE)
        return false;

    // If another shop source in the same node contains the same item, deny item placement
    const std::vector<ItemSource*> sources_in_node = node()->item_sources();
    for(ItemSource* source : sources_in_node)
        if(source->type_name() == "shop" && source->item() == item)
            return false;
    
    return true;
}

////////////////////////////////////////////////////////////////

Json ItemSourceReward::to_json() const
{
    Json json = ItemSource::to_json();
    json["address"] = _address_in_rom;
    return json;
}