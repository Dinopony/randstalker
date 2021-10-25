#include "item_source.hpp"
#include "world_region.hpp"
#include "../Exceptions.hpp"

ItemSource::ItemSource(const std::string& name, WorldRegion* region, const std::vector<std::string>& hints) :
    _name   (name),
    _item   (nullptr),
    _region (region),
    _hints  (hints)
{
    _region->add_item_source(this);
}

Json ItemSource::to_json() const
{
    Json json;
    json["name"] = _name;
    json["type"] = type_name();
    json["regionId"] = _region->id();
    if(!_hints.empty())
        json["hints"] = _hints;
    return json;
}

ItemSource* ItemSource::from_json(const Json& json, const std::map<std::string, WorldRegion*>& regions)
{
    const std::string& name = json.at("name");
    const std::string& type = json.at("type");

    const std::string& regionId = json.at("regionId");
    WorldRegion* region = regions.at(regionId);

    std::vector<std::string> hints;
    if(json.contains("hints")) 
        json.at("hints").get_to(hints);

    if(type == "chest")
    {
        uint8_t chest_id = json.at("chestId");
        return new ItemSourceChest(chest_id, name, region, hints);
    }
    else if(type == "ground")
    {
        std::vector<uint32_t> addresses_in_rom;
        if(json.at("address").is_array())
            json.at("address").get_to(addresses_in_rom);
        else
            addresses_in_rom = { json.at("address") };

        bool cannot_be_taken_repeatedly = json.value("cannotBeTakenRepeatedly", false);

        return new ItemSourceOnGround(addresses_in_rom, name, region, hints, cannot_be_taken_repeatedly);
    }
    else if(type == "shop")
    {
        std::vector<uint32_t> addresses_in_rom;
        if(json.at("address").is_array())
            json.at("address").get_to(addresses_in_rom);
        else
            addresses_in_rom = { json.at("address") };

        return new ItemSourceShop(addresses_in_rom, name, region, hints);
    }
    else if(type == "reward")
    {
        uint32_t address_in_rom = json.at("address");
        return new ItemSourceReward(address_in_rom, name, region, hints);        
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
    
    if(_addresses_in_rom.size() == 1)
        json["address"] = _addresses_in_rom[0];
    else 
        json["address"] = _addresses_in_rom;

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

    // If another shop source in the same region contains the same item, deny item placement
    const std::vector<ItemSource*> sources_in_region = region()->item_sources();
    for(ItemSource* source : sources_in_region)
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