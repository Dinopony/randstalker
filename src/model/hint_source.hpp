#pragma once

#include <vector>
#include <string>
#include "../extlibs/json.hpp"
#include "world_region.hpp"

class HintSource 
{
public:
    HintSource(const std::vector<uint16_t>& text_ids, const std::string& description, WorldRegion* region, bool special) :
        _text_ids    (text_ids),
        _description (description),
        _region      (region),
        _special     (special),
        _text        ()
    {}

    const std::vector<uint16_t>& text_ids() const { return _text_ids; }
    const std::string& description() const { return _description; }
    WorldRegion* region() const { return _region; }
    bool special() const { return _special; }
    
    const std::string& text() const { return _text; }
    void text(const std::string& text) { _text = text; }

    Json to_json() const
    {
        Json json;
        json["description"] = _description;
        json["textIds"] = _text_ids;

        if(_special)
            json["special"] = true;
        else
            json["regionId"] = _region->id();

        return json;
    }

    static HintSource* from_json(const Json& json, const std::map<std::string, WorldRegion*>& regions)
    {
        std::string description = json.at("description");
        std::vector<uint16_t> text_ids = json.at("textIds");
        bool special = json.value("special", false);

        WorldRegion* region = nullptr;
        std::string region_id = json.value("regionId", "");
        if(!region_id.empty())
            region = regions.at(region_id);

        return new HintSource(text_ids, description, region, special);
    }

private:
    std::vector<uint16_t> _text_ids;
    std::string _description;
    WorldRegion* _region;
    bool _special;
    std::string _text;
};