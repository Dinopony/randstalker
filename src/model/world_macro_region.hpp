#pragma once

#include <string>
#include <vector>
#include "world_region.hpp"

class WorldMacroRegion
{
public:
    WorldMacroRegion(const std::string& name, const std::vector<WorldRegion*> regions) :
        _name    (name),
        _regions (regions)
    {}

    const std::string& name() const { return _name; }
    const std::vector<WorldRegion*>& regions() const { return _regions; }

    Json to_json() const 
    {
        Json json;

        json["name"] = _name;
        json["regionIds"] = Json::array();
        for(WorldRegion* region : _regions)
            json["regionIds"].push_back(region->id());

        return json;
    }

    static WorldMacroRegion* from_json(const Json& json, const std::map<std::string, WorldRegion*>& all_regions)
    {
        std::string name = json.at("name");
        
        std::vector<WorldRegion*> regions;
        for(std::string region_id : json.at("regionIds"))
            regions.push_back(all_regions.at(region_id));

        return new WorldMacroRegion(name, regions);
    }

private:
    std::string _name;
    std::vector<WorldRegion*> _regions;
};